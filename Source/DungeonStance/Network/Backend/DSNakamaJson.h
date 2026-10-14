#pragma once

#include "CoreMinimal.h"
#include "Dom/JsonObject.h"
#include "Network/DSBackendTypes.h"

/**
 * Parse/validação das respostas REST do Nakama. Nunca confia no conteúdo:
 * tipos, tamanhos e intervalos são conferidos e o que estiver fora é rejeitado.
 */
namespace DSNakamaJson
{
	bool ParseObject(const FString& Json, TSharedPtr<FJsonObject>& OutObject);
	FString Serialize(const TSharedRef<FJsonObject>& Object);

	/** Corpo de /v2/rpc/{id}: o payload vai como STRING JSON (`"{\"a\":1}"`). */
	FString MakeRpcBody(const TSharedRef<FJsonObject>& Payload);

	/** Resposta de /v2/rpc/{id}: `{"payload":"<json>"}` → objeto do payload. */
	bool ParseRpcPayload(const FString& Body, TSharedPtr<FJsonObject>& OutPayload);

	/** `{"token":"...","refresh_token":"..."}` de /v2/account/authenticate/device. */
	bool ParseSessionResponse(const FString& Body, FString& OutToken, FString& OutRefreshToken);

	/** `{"user":{"id":"<uuid>","username":"...","display_name":"..."}}` de /v2/account. */
	bool ParseAccountResponse(const FString& Body, FString& OutUserId, FString& OutDisplayName);

	/** Payload de load_all_progressions (progression.ts::RpcLoadAllProgressions). */
	bool ParseProgressions(const TSharedPtr<FJsonObject>& Payload, TArray<FDSClassProgress>& OutClasses);

	/** Payload de submit_run_results (contrato novo, ver README). */
	TSharedRef<FJsonObject> MakeRunResultsPayload(const TArray<FDSRunResult>& Results);

	/** Extrai a mensagem de erro do corpo `{"error":"...","message":"...","code":N}`, truncada. */
	FString ExtractErrorMessage(const FString& Body);
}
