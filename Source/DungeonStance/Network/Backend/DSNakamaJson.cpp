#include "Network/Backend/DSNakamaJson.h"

#include "DungeonStance.h"
#include "Core/DSTypes.h"
#include "Dom/JsonValue.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"
#include "Serialization/JsonWriter.h"
#include "Policies/CondensedJsonPrintPolicy.h"

namespace DSNakamaJsonPrivate
{
	constexpr int32 MaxTokenLength = 8192;
	constexpr int32 MaxDisplayNameLength = 64;
	constexpr int32 MaxErrorLength = 256;
	constexpr int32 MaxCounter = 1000000000;

	/** Lê um número inteiro e exige que esteja em [Min, Max]. */
	bool TryGetBoundedInt(const TSharedPtr<FJsonObject>& Object, const TCHAR* Field, int32 Min, int32 Max, int32& OutValue)
	{
		double Number = 0.0;
		if (!Object.IsValid() || !Object->TryGetNumberField(Field, Number) || !FMath::IsFinite(Number))
		{
			return false;
		}
		if (Number < static_cast<double>(Min) || Number > static_cast<double>(Max))
		{
			return false;
		}
		OutValue = static_cast<int32>(Number);
		return true;
	}

	/** Escapa uma string para literal JSON. A entrada é JSON condensado gerado por nós. */
	FString QuoteJsonString(const FString& Raw)
	{
		FString Escaped;
		Escaped.Reserve(Raw.Len() + 16);
		Escaped.AppendChar(TEXT('"'));
		for (const TCHAR Char : Raw)
		{
			switch (Char)
			{
			case TEXT('"'):  Escaped += TEXT("\\\""); break;
			case TEXT('\\'): Escaped += TEXT("\\\\"); break;
			case TEXT('\n'): Escaped += TEXT("\\n"); break;
			case TEXT('\r'): Escaped += TEXT("\\r"); break;
			case TEXT('\t'): Escaped += TEXT("\\t"); break;
			default:         Escaped.AppendChar(Char); break;
			}
		}
		Escaped.AppendChar(TEXT('"'));
		return Escaped;
	}

	bool ParseClassEntry(const FString& ClassKey, const TSharedPtr<FJsonObject>& Entry, FDSClassProgress& OutProgress)
	{
		const EDSHeroClass HeroClass = DSBackendIds::HeroClassFromBackendId(ClassKey);
		const TSharedPtr<FJsonObject>* ProgressionObject = nullptr;
		if (HeroClass == EDSHeroClass::None || !Entry.IsValid() || !Entry->TryGetObjectField(TEXT("progression"), ProgressionObject))
		{
			return false;
		}

		FDSClassProgress Progress;
		Progress.HeroClass = HeroClass;
		const TSharedPtr<FJsonObject>& Progression = *ProgressionObject;
		const bool bValid = TryGetBoundedInt(Progression, TEXT("rank"), 1, DSConstants::MaxClassRank, Progress.Rank)
			&& TryGetBoundedInt(Progression, TEXT("xp"), 0, MaxCounter, Progress.XP)
			&& TryGetBoundedInt(Progression, TEXT("matches_played"), 0, MaxCounter, Progress.MatchesPlayed)
			&& TryGetBoundedInt(Progression, TEXT("victories"), 0, MaxCounter, Progress.Victories);
		if (!bValid)
		{
			return false;
		}
		// Campo derivado: opcional, 0 se ausente/inválido.
		TryGetBoundedInt(Entry, TEXT("xp_to_next_rank"), 0, MaxCounter, Progress.XPToNext);
		OutProgress = Progress;
		return true;
	}
}

bool DSNakamaJson::ParseObject(const FString& Json, TSharedPtr<FJsonObject>& OutObject)
{
	if (Json.IsEmpty())
	{
		return false;
	}
	const TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Json);
	return FJsonSerializer::Deserialize(Reader, OutObject) && OutObject.IsValid();
}

FString DSNakamaJson::Serialize(const TSharedRef<FJsonObject>& Object)
{
	FString Output;
	const TSharedRef<TJsonWriter<TCHAR, TCondensedJsonPrintPolicy<TCHAR>>> Writer =
		TJsonWriterFactory<TCHAR, TCondensedJsonPrintPolicy<TCHAR>>::Create(&Output);
	FJsonSerializer::Serialize(Object, Writer);
	return Output;
}

FString DSNakamaJson::MakeRpcBody(const TSharedRef<FJsonObject>& Payload)
{
	return DSNakamaJsonPrivate::QuoteJsonString(Serialize(Payload));
}

bool DSNakamaJson::ParseRpcPayload(const FString& Body, TSharedPtr<FJsonObject>& OutPayload)
{
	TSharedPtr<FJsonObject> Envelope;
	FString PayloadString;
	if (!ParseObject(Body, Envelope) || !Envelope->TryGetStringField(TEXT("payload"), PayloadString))
	{
		return false;
	}
	return ParseObject(PayloadString, OutPayload);
}

bool DSNakamaJson::ParseSessionResponse(const FString& Body, FString& OutToken, FString& OutRefreshToken)
{
	TSharedPtr<FJsonObject> Object;
	FString Token;
	if (!ParseObject(Body, Object) || !Object->TryGetStringField(TEXT("token"), Token))
	{
		return false;
	}

	// JWT: header.payload.assinatura, ASCII, tamanho limitado.
	int32 DotCount = 0;
	for (const TCHAR Char : Token)
	{
		DotCount += Char == TEXT('.') ? 1 : 0;
	}
	if (Token.Len() > DSNakamaJsonPrivate::MaxTokenLength || DotCount != 2)
	{
		return false;
	}

	FString RefreshToken;
	Object->TryGetStringField(TEXT("refresh_token"), RefreshToken);
	OutToken = Token;
	OutRefreshToken = RefreshToken.Len() <= DSNakamaJsonPrivate::MaxTokenLength ? RefreshToken : FString();
	return true;
}

bool DSNakamaJson::ParseAccountResponse(const FString& Body, FString& OutUserId, FString& OutDisplayName)
{
	TSharedPtr<FJsonObject> Object;
	const TSharedPtr<FJsonObject>* User = nullptr;
	FString UserId;
	if (!ParseObject(Body, Object) || !Object->TryGetObjectField(TEXT("user"), User) || !(*User)->TryGetStringField(TEXT("id"), UserId))
	{
		return false;
	}

	// Ids de usuário do Nakama são UUIDs.
	FGuid ParsedId;
	if (!FGuid::Parse(UserId, ParsedId) || !DSBackendIds::IsSafeIdentifier(UserId))
	{
		return false;
	}

	FString DisplayName;
	if (!(*User)->TryGetStringField(TEXT("display_name"), DisplayName) || DisplayName.IsEmpty())
	{
		(*User)->TryGetStringField(TEXT("username"), DisplayName);
	}
	OutUserId = UserId;
	OutDisplayName = DisplayName.Left(DSNakamaJsonPrivate::MaxDisplayNameLength);
	return true;
}

bool DSNakamaJson::ParseProgressions(const TSharedPtr<FJsonObject>& Payload, TArray<FDSClassProgress>& OutClasses)
{
	const TSharedPtr<FJsonObject>* Progressions = nullptr;
	if (!Payload.IsValid() || !Payload->TryGetObjectField(TEXT("progressions"), Progressions))
	{
		return false;
	}

	TArray<FDSClassProgress> Parsed;
	for (const auto& Pair : (*Progressions)->Values)
	{
		const TSharedPtr<FJsonObject>* Entry = nullptr;
		const bool bIsObject = Pair.Value.IsValid() && Pair.Value->TryGetObject(Entry) && Entry;
		FDSClassProgress Progress;
		if (bIsObject && DSNakamaJsonPrivate::ParseClassEntry(Pair.Key, *Entry, Progress))
		{
			Parsed.Add(Progress);
		}
		else
		{
			UE_LOG(LogDungeonStance, Warning, TEXT("[Backend:Nakama] Progressão inválida ignorada para a chave '%s'."), *Pair.Key.Left(32));
		}
	}
	OutClasses = MoveTemp(Parsed);
	return true;
}

TSharedRef<FJsonObject> DSNakamaJson::MakeRunResultsPayload(const TArray<FDSRunResult>& Results)
{
	TArray<TSharedPtr<FJsonValue>> Entries;
	for (const FDSRunResult& Result : Results)
	{
		const TSharedRef<FJsonObject> Entry = MakeShared<FJsonObject>();
		Entry->SetStringField(TEXT("user_id"), Result.PlayerId);
		Entry->SetStringField(TEXT("class_id"), DSBackendIds::HeroClassToBackendId(Result.HeroClass));
		Entry->SetNumberField(TEXT("waves_cleared"), Result.WavesCleared);
		Entry->SetNumberField(TEXT("kills"), Result.Kills);
		Entry->SetBoolField(TEXT("victory"), Result.bVictory);
		Entry->SetNumberField(TEXT("rank_xp"), Result.RankXPEarned);
		Entry->SetNumberField(TEXT("gold"), Result.GoldEarned);
		Entries.Add(MakeShared<FJsonValueObject>(Entry));
	}

	const TSharedRef<FJsonObject> Payload = MakeShared<FJsonObject>();
	Payload->SetArrayField(TEXT("results"), Entries);
	return Payload;
}

FString DSNakamaJson::ExtractErrorMessage(const FString& Body)
{
	TSharedPtr<FJsonObject> Object;
	FString Message;
	if (ParseObject(Body, Object) && (Object->TryGetStringField(TEXT("message"), Message) || Object->TryGetStringField(TEXT("error"), Message)))
	{
		return Message.Left(DSNakamaJsonPrivate::MaxErrorLength);
	}
	return FString(TEXT("resposta sem mensagem de erro"));
}
