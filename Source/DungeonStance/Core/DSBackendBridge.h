#pragma once

#include "CoreMinimal.h"
#include "Core/DSTypes.h"

class UGameInstance;

/**
 * Ponto ÚNICO onde o Core fala com o UDSBackendSubsystem (área Network).
 * Se o header do backend ainda não existir, compila sem ele e só registra aviso.
 */
namespace DSBackendBridge
{
	/** Envia os resultados da run ao backend (só servidor). */
	void SubmitRunResults(UGameInstance* GameInstance, const TArray<FDSRunResult>& Results);
}
