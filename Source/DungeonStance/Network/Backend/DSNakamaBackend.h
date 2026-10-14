#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "Network/DSBackendService.h"
#include "DSNakamaBackend.generated.h"

class UDSNetworkSettings;

/**
 * Backend Nakama via REST (FHttpModule). Endpoints usados:
 *  - POST /v2/account/authenticate/device?create=true  (Basic server_key:)  → token de sessão
 *  - GET  /v2/account                                  (Bearer token)       → user id / nome
 *  - POST /v2/rpc/load_all_progressions                (Bearer token)       → rank por classe
 *  - POST /v2/rpc/submit_run_results?http_key=...      (servidor→servidor)  → A CRIAR no Nakama
 *
 * Chaves: server key em env NakamaServerKeyEnvVar (cliente); runtime http_key em env
 * NakamaHttpKeyEnvVar (SÓ na máquina do servidor dedicado). Nenhuma chave/token é logada.
 */
UCLASS()
class DUNGEONSTANCE_API UDSNakamaBackend : public UObject, public IDSBackendService
{
	GENERATED_BODY()

public:
	/** Valida e copia a configuração. Retorna false se host/porta/RPCs forem inválidos. */
	bool Configure(const UDSNetworkSettings& Settings);

	// IDSBackendService
	virtual FName GetBackendName() const override { return FName(TEXT("Nakama")); }
	virtual void Authenticate(const FString& DeviceId, FDSOnBackendAuthComplete OnComplete) override;
	virtual void FetchProfile(FDSOnBackendProfileComplete OnComplete) override;
	virtual void SubmitRunResults(const TArray<FDSRunResult>& Results, FDSOnBackendSubmitComplete OnComplete) override;
	virtual void VerifyPlayerToken(const FString& InSessionToken, FDSOnBackendTokenVerified OnComplete) override;
	virtual bool IsAuthenticated() const override { return !SessionToken.IsEmpty() && !PlayerId.IsEmpty(); }
	virtual FString GetPlayerId() const override { return PlayerId; }
	virtual FString GetSessionToken() const override { return SessionToken; }

private:
	/** bOk = conexão estabelecida e HTTP 2xx. Sempre chamado uma única vez, no game thread. */
	using FHttpDone = TFunction<void(bool bOk, int32 StatusCode, const FString& Body)>;

	void SendRequest(const FString& Verb, const FString& PathAndQuery, const FString& Body, const FString& AuthHeader, FHttpDone OnDone);
	void RequestAccount(const FString& Token, TFunction<void(bool bOk, const FString& UserId, const FString& DisplayName, const FString& Error)> OnDone);
	void HandleAuthResponse(bool bOk, int32 StatusCode, const FString& Body, FDSOnBackendAuthComplete OnComplete);

	void ClearSession();
	FString MakeBearerHeader(const FString& Token) const;
	static FString ReadEnvironmentVariable(const FString& Name);
	static FString DescribeHttpError(int32 StatusCode, const FString& Body);

	FString BaseUrl;
	float TimeoutSeconds = 10.f;
	FString ServerKeyEnvVar;
	FString HttpKeyEnvVar;
	FString ProfileRpcId;
	FString SubmitRunRpcId;

	FString SessionToken;
	FString RefreshToken;
	FString PlayerId;
	FString DisplayName;
};
