#pragma once

#include "CoreMinimal.h"
#include "Core/DSTypes.h"
#include "DSSessionTypes.generated.h"

/** Resumo de uma sessão encontrada, pronto para a UI (lista de partidas). */
USTRUCT(BlueprintType)
struct DUNGEONSTANCE_API FDSSessionInfo
{
	GENERATED_BODY()

	/** Índice a passar para UDSSessionSubsystem::JoinSession. */
	UPROPERTY(BlueprintReadOnly, Category = "DS|Session")
	int32 ResultIndex = INDEX_NONE;

	UPROPERTY(BlueprintReadOnly, Category = "DS|Session")
	FString SessionId;

	UPROPERTY(BlueprintReadOnly, Category = "DS|Session")
	FString HostName;

	UPROPERTY(BlueprintReadOnly, Category = "DS|Session")
	EDSPartySize PartySize = EDSPartySize::Solo;

	UPROPERTY(BlueprintReadOnly, Category = "DS|Session")
	int32 MaxPlayers = 0;

	UPROPERTY(BlueprintReadOnly, Category = "DS|Session")
	int32 CurrentPlayers = 0;

	UPROPERTY(BlueprintReadOnly, Category = "DS|Session")
	int32 PingMs = 0;

	UPROPERTY(BlueprintReadOnly, Category = "DS|Session")
	bool bIsLAN = false;

	int32 GetOpenSlots() const { return FMath::Max(0, MaxPlayers - CurrentPlayers); }
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FDSOnSessionCreated, bool, bWasSuccessful);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FDSOnSessionsFound, const TArray<FDSSessionInfo>&, Sessions);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FDSOnSessionJoined, bool, bWasSuccessful);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FDSOnSessionDestroyed, bool, bWasSuccessful);

/** Chaves dos settings custom anunciados na sessão. */
namespace DSSessionKeys
{
	/** int32: número de jogadores da party (1..5). Usado no filtro da busca. */
	inline FName PartySize() { return FName(TEXT("DS_PARTY_SIZE")); }

	/** FString: modo de jogo (UDSNetworkSettings::GameModeTag). */
	inline FName GameMode() { return FName(TEXT("DS_GAME_MODE")); }

	/** Opção de URL lida por ADSGameSession para fixar MaxPlayers ("?PartySize=3"). */
	inline const TCHAR* PartySizeUrlOption() { return TEXT("PartySize"); }
}
