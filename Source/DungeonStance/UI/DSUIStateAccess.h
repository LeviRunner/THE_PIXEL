#pragma once

#include "CoreMinimal.h"
#include "Core/DSTypes.h"

class AActor;
class ADSGameState;
class ADSPlayerController;
class ADSPlayerState;
class UDSSessionSubsystem;
class APlayerController;
class UAbilitySystemComponent;
class UObject;

/**
 * Ponto ÚNICO onde a UI lê a API das outras áreas (Core/Gameplay/Network).
 * Se um getter mudar de nome no GameState/PlayerState/GameInstance, ajuste só o .cpp deste arquivo.
 * Delegates (AddDynamic) ficam nos widgets porque dependem do nome do membro (ver CONTRATOS.md).
 */
namespace DSUIState
{
	// --- ADSGameState ---
	int32 GetCurrentWave(const ADSGameState& GameState);
	int32 GetMaxWaves(const ADSGameState& GameState);
	bool IsBossWave(const ADSGameState& GameState);
	EDSMatchPhase GetMatchPhase(const ADSGameState& GameState);
	int32 GetTeamLevel(const ADSGameState& GameState);
	int32 GetTeamXP(const ADSGameState& GameState);
	int32 GetTeamXPToNext(const ADSGameState& GameState);
	int32 GetEnemiesRemaining(const ADSGameState& GameState);
	float GetPhaseTimeRemaining(const ADSGameState& GameState);

	// --- ADSPlayerState ---
	EDSHeroClass GetHeroClass(const ADSPlayerState& PlayerState);
	EDSStance GetStance(const ADSPlayerState& PlayerState);
	int32 GetClassRank(const ADSPlayerState& PlayerState);
	int32 GetKills(const ADSPlayerState& PlayerState);
	int32 GetPendingCardPicks(const ADSPlayerState& PlayerState);
	TArray<FName> GetCurrentCardChoices(const ADSPlayerState& PlayerState);
	int32 GetOwnedCardStacks(const ADSPlayerState& PlayerState, FName CardId);
	bool IsReady(const ADSPlayerState& PlayerState);

	/** ASC de qualquer ator (PlayerState, herói, inimigo) via IAbilitySystemInterface. */
	UAbilitySystemComponent* GetAbilitySystem(const AActor* Actor);

	/** Controller do jogo (RPCs Server_*). Nulo fora de uma partida DS. */
	ADSPlayerController* GetDSPlayerController(APlayerController* PlayerController);

	/**
	 * Resumo de fim de run montado no cliente a partir do estado replicado.
	 * TODO(Core): substituir por um FDSRunResult replicado pelo servidor (o XP real é decidido lá).
	 */
	FDSRunResult BuildLocalRunResult(const ADSGameState& GameState, const ADSPlayerState& PlayerState, bool bVictory);

	// --- Network ---
	UDSSessionSubsystem* GetSessionSubsystem(const UObject* WorldContext);

	// --- UDSGameInstance (preferências locais do lobby) ---
	void LoadHeroPreference(const UObject* WorldContext, EDSHeroClass& OutClass, EDSStance& OutStance, EDSPartySize& OutPartySize);
	void StoreHeroPreference(const UObject* WorldContext, EDSHeroClass HeroClass, EDSStance Stance, EDSPartySize PartySize);
}
