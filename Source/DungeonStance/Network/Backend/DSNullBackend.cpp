#include "Network/Backend/DSNullBackend.h"

#include "DungeonStance.h"
#include "Core/DSRunRules.h"
#include "Network/Backend/DSBackendProfileSave.h"

void UDSNullBackend::Configure(const FString& InSlotName)
{
	SlotName = InSlotName;
}

void UDSNullBackend::Authenticate(const FString& DeviceId, FDSOnBackendAuthComplete OnComplete)
{
	if (!DSBackendIds::IsSafeIdentifier(DeviceId, 10, 128))
	{
		OnComplete.ExecuteIfBound(false, TEXT("Device id inválido."));
		return;
	}

	SaveData = UDSBackendProfileSave::LoadOrCreate(SlotName);
	if (!DSBackendIds::IsSafeIdentifier(SaveData->PlayerId))
	{
		SaveData->PlayerId = FString(TEXT("local-")) + DeviceId.Left(64);
		SaveData->DisplayName = TEXT("Jogador");
		SaveData->SaveToSlot(SlotName);
	}

	PlayerId = SaveData->PlayerId;
	bAuthenticated = true;
	UE_LOG(LogDungeonStance, Log, TEXT("[Backend:Null] Autenticado offline como %s."), *PlayerId);
	OnComplete.ExecuteIfBound(true, FString());
}

void UDSNullBackend::FetchProfile(FDSOnBackendProfileComplete OnComplete)
{
	if (!bAuthenticated || !SaveData)
	{
		OnComplete.ExecuteIfBound(false, FDSPlayerProfile(), TEXT("Não autenticado."));
		return;
	}
	OnComplete.ExecuteIfBound(true, BuildProfile(), FString());
}

void UDSNullBackend::SubmitRunResults(const TArray<FDSRunResult>& Results, FDSOnBackendSubmitComplete OnComplete)
{
	if (!bAuthenticated || !SaveData)
	{
		OnComplete.ExecuteIfBound(false, TEXT("Não autenticado."));
		return;
	}

	int32 Applied = 0;
	for (const FDSRunResult& Result : Results)
	{
		FString Reason;
		if (!DSBackendIds::ValidateRunResult(Result, Reason))
		{
			UE_LOG(LogDungeonStance, Warning, TEXT("[Backend:Null] Resultado descartado: %s."), *Reason);
			continue;
		}
		if (Result.PlayerId != PlayerId)
		{
			continue; // Offline: só o jogador local tem perfil nesta máquina.
		}

		const int32 Index = SaveData->Classes.IndexOfByPredicate([&Result](const FDSClassProgress& Entry)
		{
			return Entry.HeroClass == Result.HeroClass;
		});
		const FDSClassProgress Current = Index != INDEX_NONE ? SaveData->Classes[Index] : MakeDefaultProgress(Result.HeroClass);
		const FDSClassProgress Updated = ApplyRunResult(Current, Result);
		if (Index != INDEX_NONE)
		{
			SaveData->Classes[Index] = Updated;
		}
		else
		{
			SaveData->Classes.Add(Updated);
		}
		++Applied;
	}

	const bool bSaved = Applied == 0 || SaveData->SaveToSlot(SlotName);
	UE_LOG(LogDungeonStance, Log, TEXT("[Backend:Null] %d resultado(s) aplicados ao perfil local."), Applied);
	OnComplete.ExecuteIfBound(bSaved, bSaved ? FString() : TEXT("Falha ao gravar o perfil local."));
}

void UDSNullBackend::VerifyPlayerToken(const FString& SessionToken, FDSOnBackendTokenVerified OnComplete)
{
	// Offline não há autoridade de conta: aceita o id informado se for sintaticamente seguro.
	const bool bValid = DSBackendIds::IsSafeIdentifier(SessionToken);
	OnComplete.ExecuteIfBound(bValid, bValid ? SessionToken : FString(), bValid ? FString() : TEXT("Id inválido."));
}

FDSPlayerProfile UDSNullBackend::BuildProfile() const
{
	FDSPlayerProfile Profile;
	Profile.PlayerId = PlayerId;
	Profile.DisplayName = SaveData->DisplayName;
	Profile.Classes = SaveData->Classes;
	return Profile;
}

FDSClassProgress UDSNullBackend::MakeDefaultProgress(EDSHeroClass HeroClass)
{
	FDSClassProgress Progress;
	Progress.HeroClass = HeroClass;
	Progress.Rank = 1;
	Progress.XPToNext = DSRunRules::GetRankXPToNext(1);
	return Progress;
}

FDSClassProgress UDSNullBackend::ApplyRunResult(const FDSClassProgress& Current, const FDSRunResult& Result)
{
	// Mesma regra de progression.ts::applyXp, com a fórmula de XP vinda de Core/DSRunRules.
	int32 Rank = FMath::Clamp(Current.Rank, 1, DSConstants::MaxClassRank);
	int32 XP = FMath::Max(0, Current.XP) + FMath::Max(0, Result.RankXPEarned);
	while (Rank < DSConstants::MaxClassRank)
	{
		const int32 Needed = DSRunRules::GetRankXPToNext(Rank);
		if (Needed <= 0 || XP < Needed)
		{
			break;
		}
		XP -= Needed;
		++Rank;
	}

	FDSClassProgress Updated = Current;
	Updated.Rank = Rank;
	Updated.XP = Rank >= DSConstants::MaxClassRank ? 0 : XP;
	Updated.XPToNext = Rank >= DSConstants::MaxClassRank ? 0 : DSRunRules::GetRankXPToNext(Rank);
	Updated.MatchesPlayed = Current.MatchesPlayed + 1;
	Updated.Victories = Current.Victories + (Result.bVictory ? 1 : 0);
	return Updated;
}
