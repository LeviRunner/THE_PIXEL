#include "UI/DSUIStateAccess.h"

#include "AbilitySystemGlobals.h"
#include "Core/DSGameInstance.h"
#include "Core/DSGameState.h"
#include "Core/DSPlayerController.h"
#include "Core/DSPlayerState.h"
#include "Core/DSRunRules.h"
#include "Engine/GameInstance.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "Network/DSSessionSubsystem.h"

// Nomes de getters assumidos a partir de CONTRATOS.md ("Getters BlueprintPure").
// Se a área Core usar outros nomes, este é o único arquivo da UI a ajustar.

namespace DSUIState
{
	int32 GetCurrentWave(const ADSGameState& GameState) { return GameState.GetCurrentWave(); }
	int32 GetMaxWaves(const ADSGameState& GameState) { return GameState.GetMaxWaves(); }
	bool IsBossWave(const ADSGameState& GameState) { return GameState.IsBossWave(); }
	EDSMatchPhase GetMatchPhase(const ADSGameState& GameState) { return GameState.GetMatchPhase(); }
	int32 GetTeamLevel(const ADSGameState& GameState) { return GameState.GetTeamLevel(); }
	int32 GetTeamXP(const ADSGameState& GameState) { return GameState.GetTeamXP(); }
	int32 GetTeamXPToNext(const ADSGameState& GameState) { return GameState.GetTeamXPToNext(); }
	int32 GetEnemiesRemaining(const ADSGameState& GameState) { return GameState.GetEnemiesRemaining(); }
	float GetPhaseTimeRemaining(const ADSGameState& GameState) { return GameState.GetPhaseTimeRemaining(); }

	EDSHeroClass GetHeroClass(const ADSPlayerState& PlayerState) { return PlayerState.GetHeroClass(); }
	EDSStance GetStance(const ADSPlayerState& PlayerState) { return PlayerState.GetStance(); }
	int32 GetClassRank(const ADSPlayerState& PlayerState) { return PlayerState.GetClassRank(); }
	int32 GetKills(const ADSPlayerState& PlayerState) { return PlayerState.GetKills(); }
	int32 GetPendingCardPicks(const ADSPlayerState& PlayerState) { return PlayerState.GetPendingCardPicks(); }
	TArray<FName> GetCurrentCardChoices(const ADSPlayerState& PlayerState) { return PlayerState.GetCurrentCardChoices(); }
	bool IsReady(const ADSPlayerState& PlayerState) { return PlayerState.IsReady(); }

	int32 GetOwnedCardStacks(const ADSPlayerState& PlayerState, FName CardId)
	{
		for (const FDSOwnedCard& Card : PlayerState.GetOwnedCards())
		{
			if (Card.CardId == CardId)
			{
				return Card.Stacks;
			}
		}
		return 0;
	}

	UAbilitySystemComponent* GetAbilitySystem(const AActor* Actor)
	{
		return UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(Actor);
	}

	ADSPlayerController* GetDSPlayerController(APlayerController* PlayerController)
	{
		return Cast<ADSPlayerController>(PlayerController);
	}

	FDSRunResult BuildLocalRunResult(const ADSGameState& GameState, const ADSPlayerState& PlayerState, bool bVictory)
	{
		FDSRunResult Result;
		Result.HeroClass = GetHeroClass(PlayerState);
		Result.Kills = GetKills(PlayerState);
		Result.bVictory = bVictory;
		// Na vitória todas as ondas foram limpas; na derrota, a onda atual não conta.
		Result.WavesCleared = bVictory ? GetMaxWaves(GameState) : FMath::Max(0, GetCurrentWave(GameState) - 1);
		Result.RankXPEarned = DSRunRules::GetEndOfRunRankXP(Result.WavesCleared, bVictory, Result.Kills);
		// TODO(Core): ouro/espólio reais só existem no servidor; ficam 0 até virem replicados.
		Result.GoldEarned = 0;
		return Result;
	}

	UDSSessionSubsystem* GetSessionSubsystem(const UObject* WorldContext)
	{
		const UWorld* World = WorldContext ? WorldContext->GetWorld() : nullptr;
		const UGameInstance* GameInstance = World ? World->GetGameInstance() : nullptr;
		return GameInstance ? GameInstance->GetSubsystem<UDSSessionSubsystem>() : nullptr;
	}

	void LoadHeroPreference(const UObject* WorldContext, EDSHeroClass& OutClass, EDSStance& OutStance, EDSPartySize& OutPartySize)
	{
		const UWorld* World = WorldContext ? WorldContext->GetWorld() : nullptr;
		const UDSGameInstance* GameInstance = World ? World->GetGameInstance<UDSGameInstance>() : nullptr;
		if (!GameInstance)
		{
			return;
		}
		OutClass = GameInstance->LastClass;
		OutStance = GameInstance->LastStance;
		OutPartySize = GameInstance->DesiredPartySize;
	}

	void StoreHeroPreference(const UObject* WorldContext, EDSHeroClass HeroClass, EDSStance Stance, EDSPartySize PartySize)
	{
		const UWorld* World = WorldContext ? WorldContext->GetWorld() : nullptr;
		UDSGameInstance* GameInstance = World ? World->GetGameInstance<UDSGameInstance>() : nullptr;
		if (!GameInstance)
		{
			return;
		}
		GameInstance->LastClass = HeroClass;
		GameInstance->LastStance = Stance;
		GameInstance->DesiredPartySize = PartySize;
	}
}
