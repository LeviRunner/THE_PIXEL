#include "Network/Backend/DSNakamaBackend.h"

#include "DungeonStance.h"
#include "Network/DSNetworkSettings.h"
#include "Network/Backend/DSNakamaJson.h"
#include "HttpModule.h"
#include "Interfaces/IHttpRequest.h"
#include "Interfaces/IHttpResponse.h"
#include "GenericPlatform/GenericPlatformHttp.h"
#include "HAL/PlatformMisc.h"
#include "Misc/Base64.h"

namespace DSNakamaPrivate
{
	constexpr int32 HttpUnauthorized = 401;
	constexpr int32 MinDeviceIdLength = 10;
	constexpr int32 MaxDeviceIdLength = 128;
	constexpr int32 MaxTokenLength = 8192;

	bool IsSuccessStatus(int32 StatusCode)
	{
		return StatusCode >= 200 && StatusCode < 300;
	}

	/** Host só com caracteres de hostname/IP (evita injeção na URL). */
	bool IsValidHost(const FString& Host)
	{
		if (Host.IsEmpty() || Host.Len() > 253)
		{
			return false;
		}
		for (const TCHAR Char : Host)
		{
			if (!FChar::IsAlnum(Char) && Char != TEXT('.') && Char != TEXT('-'))
			{
				return false;
			}
		}
		return true;
	}
}

bool UDSNakamaBackend::Configure(const UDSNetworkSettings& Settings)
{
	const bool bPortOk = Settings.NakamaPort > 0 && Settings.NakamaPort <= 65535;
	const bool bRpcOk = DSBackendIds::IsSafeIdentifier(Settings.NakamaProfileRpcId) && DSBackendIds::IsSafeIdentifier(Settings.NakamaSubmitRunRpcId);
	if (!DSNakamaPrivate::IsValidHost(Settings.NakamaHost) || !bPortOk || !bRpcOk)
	{
		UE_LOG(LogDungeonStance, Error, TEXT("[Backend:Nakama] Configuração inválida (host/porta/ids de RPC)."));
		return false;
	}

	BaseUrl = FString::Printf(TEXT("%s://%s:%d"), Settings.bNakamaUseSSL ? TEXT("https") : TEXT("http"), *Settings.NakamaHost, Settings.NakamaPort);
	TimeoutSeconds = Settings.HttpTimeoutSeconds;
	ServerKeyEnvVar = Settings.NakamaServerKeyEnvVar;
	HttpKeyEnvVar = Settings.NakamaHttpKeyEnvVar;
	ProfileRpcId = Settings.NakamaProfileRpcId;
	SubmitRunRpcId = Settings.NakamaSubmitRunRpcId;

	if (!Settings.bNakamaUseSSL)
	{
		UE_LOG(LogDungeonStance, Warning, TEXT("[Backend:Nakama] SSL desligado: use só em desenvolvimento local."));
	}
	return true;
}

// --- HTTP ---------------------------------------------------------------

void UDSNakamaBackend::SendRequest(const FString& Verb, const FString& PathAndQuery, const FString& Body, const FString& AuthHeader, FHttpDone OnDone)
{
	const TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Request = FHttpModule::Get().CreateRequest();
	Request->SetURL(BaseUrl + PathAndQuery);
	Request->SetVerb(Verb);
	Request->SetHeader(TEXT("Content-Type"), TEXT("application/json"));
	Request->SetHeader(TEXT("Accept"), TEXT("application/json"));
	if (!AuthHeader.IsEmpty())
	{
		Request->SetHeader(TEXT("Authorization"), AuthHeader);
	}
	if (!Body.IsEmpty())
	{
		Request->SetContentAsString(Body);
	}
	Request->SetTimeout(TimeoutSeconds);

	// Garante uma única chamada de OnDone mesmo se ProcessRequest falhar e o delegate também disparar.
	const TSharedRef<bool> bCompleted = MakeShared<bool>(false);
	Request->OnProcessRequestComplete().BindWeakLambda(this,
		[OnDone, bCompleted](FHttpRequestPtr /*Req*/, FHttpResponsePtr Response, bool bConnectedSuccessfully)
		{
			if (*bCompleted)
			{
				return;
			}
			*bCompleted = true;
			const int32 StatusCode = Response.IsValid() ? Response->GetResponseCode() : 0;
			const FString ResponseBody = Response.IsValid() ? Response->GetContentAsString() : FString();
			OnDone(bConnectedSuccessfully && DSNakamaPrivate::IsSuccessStatus(StatusCode), StatusCode, ResponseBody);
		});

	if (!Request->ProcessRequest() && !*bCompleted)
	{
		*bCompleted = true;
		UE_LOG(LogDungeonStance, Error, TEXT("[Backend:Nakama] Falha ao iniciar requisição %s %s."), *Verb, *PathAndQuery.Left(64));
		OnDone(false, 0, FString());
	}
}

FString UDSNakamaBackend::MakeBearerHeader(const FString& Token) const
{
	return FString(TEXT("Bearer ")) + Token;
}

FString UDSNakamaBackend::ReadEnvironmentVariable(const FString& Name)
{
	return Name.IsEmpty() ? FString() : FPlatformMisc::GetEnvironmentVariable(*Name);
}

FString UDSNakamaBackend::DescribeHttpError(int32 StatusCode, const FString& Body)
{
	if (StatusCode == 0)
	{
		return TEXT("Sem conexão com o Nakama (timeout ou servidor fora do ar).");
	}
	return FString::Printf(TEXT("HTTP %d: %s"), StatusCode, *DSNakamaJson::ExtractErrorMessage(Body));
}

void UDSNakamaBackend::ClearSession()
{
	SessionToken.Reset();
	RefreshToken.Reset();
	PlayerId.Reset();
	DisplayName.Reset();
}

// --- Autenticação -------------------------------------------------------

void UDSNakamaBackend::Authenticate(const FString& DeviceId, FDSOnBackendAuthComplete OnComplete)
{
	if (!DSBackendIds::IsSafeIdentifier(DeviceId, DSNakamaPrivate::MinDeviceIdLength, DSNakamaPrivate::MaxDeviceIdLength))
	{
		OnComplete.ExecuteIfBound(false, TEXT("Device id inválido (10-128 caracteres alfanuméricos)."));
		return;
	}

	const FString ServerKey = ReadEnvironmentVariable(ServerKeyEnvVar);
	if (ServerKey.IsEmpty())
	{
		UE_LOG(LogDungeonStance, Error, TEXT("[Backend:Nakama] Variável de ambiente '%s' (server key) não definida."), *ServerKeyEnvVar);
		OnComplete.ExecuteIfBound(false, TEXT("Server key do Nakama não configurada."));
		return;
	}

	ClearSession();
	const TSharedRef<FJsonObject> Body = MakeShared<FJsonObject>();
	Body->SetStringField(TEXT("id"), DeviceId);
	const FString BasicAuth = FString(TEXT("Basic ")) + FBase64::Encode(ServerKey + TEXT(":"));

	SendRequest(TEXT("POST"), TEXT("/v2/account/authenticate/device?create=true"), DSNakamaJson::Serialize(Body), BasicAuth,
		[this, OnComplete](bool bOk, int32 StatusCode, const FString& ResponseBody)
		{
			HandleAuthResponse(bOk, StatusCode, ResponseBody, OnComplete);
		});
}

void UDSNakamaBackend::HandleAuthResponse(bool bOk, int32 StatusCode, const FString& Body, FDSOnBackendAuthComplete OnComplete)
{
	FString Token;
	FString Refresh;
	if (!bOk || !DSNakamaJson::ParseSessionResponse(Body, Token, Refresh))
	{
		const FString Error = bOk ? FString(TEXT("Resposta de autenticação inválida.")) : DescribeHttpError(StatusCode, Body);
		UE_LOG(LogDungeonStance, Warning, TEXT("[Backend:Nakama] Autenticação falhou: %s"), *Error);
		OnComplete.ExecuteIfBound(false, Error);
		return;
	}

	RequestAccount(Token, [this, Token, Refresh, OnComplete](bool bAccountOk, const FString& UserId, const FString& Name, const FString& Error)
	{
		if (!bAccountOk)
		{
			OnComplete.ExecuteIfBound(false, Error);
			return;
		}
		SessionToken = Token;
		RefreshToken = Refresh;
		PlayerId = UserId;
		DisplayName = Name;
		UE_LOG(LogDungeonStance, Log, TEXT("[Backend:Nakama] Autenticado como %s."), *PlayerId);
		OnComplete.ExecuteIfBound(true, FString());
	});
}

void UDSNakamaBackend::RequestAccount(const FString& Token, TFunction<void(bool, const FString&, const FString&, const FString&)> OnDone)
{
	SendRequest(TEXT("GET"), TEXT("/v2/account"), FString(), MakeBearerHeader(Token),
		[OnDone](bool bOk, int32 StatusCode, const FString& Body)
		{
			FString UserId;
			FString Name;
			if (!bOk)
			{
				OnDone(false, FString(), FString(), DescribeHttpError(StatusCode, Body));
				return;
			}
			if (!DSNakamaJson::ParseAccountResponse(Body, UserId, Name))
			{
				OnDone(false, FString(), FString(), TEXT("Resposta de conta inválida."));
				return;
			}
			OnDone(true, UserId, Name, FString());
		});
}

void UDSNakamaBackend::VerifyPlayerToken(const FString& InSessionToken, FDSOnBackendTokenVerified OnComplete)
{
	if (InSessionToken.IsEmpty() || InSessionToken.Len() > DSNakamaPrivate::MaxTokenLength)
	{
		OnComplete.ExecuteIfBound(false, FString(), TEXT("Token ausente ou grande demais."));
		return;
	}
	RequestAccount(InSessionToken, [OnComplete](bool bOk, const FString& UserId, const FString& /*Name*/, const FString& Error)
	{
		OnComplete.ExecuteIfBound(bOk, UserId, Error);
	});
}

// --- Perfil -------------------------------------------------------------

void UDSNakamaBackend::FetchProfile(FDSOnBackendProfileComplete OnComplete)
{
	if (!IsAuthenticated())
	{
		OnComplete.ExecuteIfBound(false, FDSPlayerProfile(), TEXT("Não autenticado."));
		return;
	}

	const FString Path = FString(TEXT("/v2/rpc/")) + ProfileRpcId;
	SendRequest(TEXT("POST"), Path, DSNakamaJson::MakeRpcBody(MakeShared<FJsonObject>()), MakeBearerHeader(SessionToken),
		[this, OnComplete](bool bOk, int32 StatusCode, const FString& Body)
		{
			if (StatusCode == DSNakamaPrivate::HttpUnauthorized)
			{
				ClearSession(); // TODO: renovar via /v2/account/session/refresh antes de exigir novo login.
			}

			TSharedPtr<FJsonObject> Payload;
			FDSPlayerProfile Profile;
			if (!bOk || !DSNakamaJson::ParseRpcPayload(Body, Payload) || !DSNakamaJson::ParseProgressions(Payload, Profile.Classes))
			{
				const FString Error = bOk ? FString(TEXT("Payload de progressão inválido.")) : DescribeHttpError(StatusCode, Body);
				UE_LOG(LogDungeonStance, Warning, TEXT("[Backend:Nakama] FetchProfile falhou: %s"), *Error);
				OnComplete.ExecuteIfBound(false, FDSPlayerProfile(), Error);
				return;
			}

			Profile.PlayerId = PlayerId;
			Profile.DisplayName = DisplayName;
			OnComplete.ExecuteIfBound(true, Profile, FString());
		});
}

// --- Fim de run (servidor→servidor) -------------------------------------

void UDSNakamaBackend::SubmitRunResults(const TArray<FDSRunResult>& Results, FDSOnBackendSubmitComplete OnComplete)
{
	const FString HttpKey = ReadEnvironmentVariable(HttpKeyEnvVar);
	if (HttpKey.IsEmpty())
	{
		UE_LOG(LogDungeonStance, Error, TEXT("[Backend:Nakama] SubmitRunResults exige a http_key em '%s' (só no servidor dedicado)."), *HttpKeyEnvVar);
		OnComplete.ExecuteIfBound(false, TEXT("http_key do Nakama ausente neste processo."));
		return;
	}

	TArray<FDSRunResult> Valid;
	for (const FDSRunResult& Result : Results)
	{
		FString Reason;
		if (DSBackendIds::ValidateRunResult(Result, Reason))
		{
			Valid.Add(Result);
		}
		else
		{
			UE_LOG(LogDungeonStance, Warning, TEXT("[Backend:Nakama] Resultado descartado: %s."), *Reason);
		}
	}
	if (Valid.IsEmpty())
	{
		OnComplete.ExecuteIfBound(false, TEXT("Nenhum resultado válido para enviar."));
		return;
	}

	const FString Path = FString::Printf(TEXT("/v2/rpc/%s?http_key=%s"), *SubmitRunRpcId, *FGenericPlatformHttp::UrlEncode(HttpKey));
	const FString Body = DSNakamaJson::MakeRpcBody(DSNakamaJson::MakeRunResultsPayload(Valid));
	const int32 NumResults = Valid.Num();

	SendRequest(TEXT("POST"), Path, Body, FString(),
		[OnComplete, NumResults](bool bOk, int32 StatusCode, const FString& ResponseBody)
		{
			const FString Error = bOk ? FString() : DescribeHttpError(StatusCode, ResponseBody);
			UE_LOG(LogDungeonStance, Log, TEXT("[Backend:Nakama] SubmitRunResults (%d jogadores): %s"),
				NumResults, bOk ? TEXT("ok") : *Error);
			OnComplete.ExecuteIfBound(bOk, Error);
		});
}
