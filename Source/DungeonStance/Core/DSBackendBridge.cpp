#include "Core/DSBackendBridge.h"

#include "DungeonStance.h"
#include "Engine/GameInstance.h"

// O caminho do header do backend ainda não está fixado no CONTRATOS.md;
// procura nos dois lugares prováveis da área Network.
#if __has_include("Network/Backend/DSBackendSubsystem.h")
	#include "Network/Backend/DSBackendSubsystem.h"
	#define DS_HAS_BACKEND_SUBSYSTEM 1
#elif __has_include("Network/DSBackendSubsystem.h")
	#include "Network/DSBackendSubsystem.h"
	#define DS_HAS_BACKEND_SUBSYSTEM 1
#else
	#define DS_HAS_BACKEND_SUBSYSTEM 0
#endif

namespace DSBackendBridge
{
	void SubmitRunResults(UGameInstance* GameInstance, const TArray<FDSRunResult>& Results)
	{
		if (Results.Num() == 0)
		{
			return;
		}
#if DS_HAS_BACKEND_SUBSYSTEM
		UDSBackendSubsystem* Backend = GameInstance ? GameInstance->GetSubsystem<UDSBackendSubsystem>() : nullptr;
		if (!Backend)
		{
			UE_LOG(LogDungeonStance, Error, TEXT("Backend indisponível: %d resultado(s) de run não enviados."), Results.Num());
			return;
		}
		Backend->SubmitRunResults(Results);
#else
		UE_LOG(LogDungeonStance, Warning, TEXT("UDSBackendSubsystem não compilado: %d resultado(s) de run não enviados."), Results.Num());
#endif
	}
}

#undef DS_HAS_BACKEND_SUBSYSTEM
