#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "Network/DSBackendTypes.h"
#include "DSBackendService.generated.h"

UINTERFACE(MinimalAPI, meta = (CannotImplementInterfaceInBlueprint))
class UDSBackendService : public UInterface
{
	GENERATED_BODY()
};

/**
 * Backend de conta/progressão. Implementações: UDSNullBackend (offline) e UDSNakamaBackend (REST).
 * Não use diretamente da UI: passe pelo UDSBackendSubsystem (delegates Blueprint + cache do perfil).
 * Todos os callbacks rodam no game thread e são chamados exatamente uma vez.
 */
class DUNGEONSTANCE_API IDSBackendService
{
	GENERATED_BODY()

public:
	virtual FName GetBackendName() const = 0;

	/** Autentica a conta deste dispositivo (cria se não existir). */
	virtual void Authenticate(const FString& DeviceId, FDSOnBackendAuthComplete OnComplete) = 0;

	/** Perfil + rank por classe da conta autenticada. */
	virtual void FetchProfile(FDSOnBackendProfileComplete OnComplete) = 0;

	/** SÓ SERVIDOR: persiste o fim de run de todos os jogadores da sessão. */
	virtual void SubmitRunResults(const TArray<FDSRunResult>& Results, FDSOnBackendSubmitComplete OnComplete) = 0;

	/**
	 * SÓ SERVIDOR: resolve o PlayerId a partir do token de sessão enviado pelo cliente no login,
	 * para que o servidor nunca confie em um PlayerId informado pelo próprio cliente.
	 */
	virtual void VerifyPlayerToken(const FString& SessionToken, FDSOnBackendTokenVerified OnComplete) = 0;

	virtual bool IsAuthenticated() const = 0;
	virtual FString GetPlayerId() const = 0;

	/** Token a ser enviado ao servidor UE no login (offline: o próprio PlayerId). Nunca logar. */
	virtual FString GetSessionToken() const = 0;
};
