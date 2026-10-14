#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "Core/DSTypes.h"
#include "DSGameState.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FDSOnWaveChanged, int32, Wave, bool, bIsBoss);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FDSOnMatchPhaseChanged, EDSMatchPhase, NewPhase);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FDSOnTeamXPChanged, int32, Level, int32, XP, int32, XPToNext);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FDSOnEnemiesRemainingChanged, int32, Remaining);

/**
 * Estado replicado da partida: onda, fase, nível/XP do time, timer da fase e
 * inimigos restantes. Quem escreve é só o ADSGameMode (servidor) pelos Server*;
 * UI e Gameplay só leem e escutam os delegates.
 */
UCLASS()
class DUNGEONSTANCE_API ADSGameState : public AGameStateBase
{
	GENERATED_BODY()

public:
	ADSGameState(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	// ----- Getters -----
	UFUNCTION(BlueprintPure, Category = "DungeonStance|Match")
	int32 GetCurrentWave() const { return CurrentWave; }

	UFUNCTION(BlueprintPure, Category = "DungeonStance|Match")
	int32 GetMaxWaves() const { return MaxWaves; }

	UFUNCTION(BlueprintPure, Category = "DungeonStance|Match")
	bool IsBossWave() const { return bIsBossWave; }

	UFUNCTION(BlueprintPure, Category = "DungeonStance|Match")
	EDSMatchPhase GetMatchPhase() const { return MatchPhase; }

	UFUNCTION(BlueprintPure, Category = "DungeonStance|Match")
	int32 GetTeamLevel() const { return TeamLevel; }

	UFUNCTION(BlueprintPure, Category = "DungeonStance|Match")
	int32 GetTeamXP() const { return TeamXP; }

	UFUNCTION(BlueprintPure, Category = "DungeonStance|Match")
	int32 GetTeamXPToNext() const { return TeamXPToNext; }

	/** Fim da fase atual no relógio do servidor (GetServerWorldTimeSeconds); 0 = sem prazo. */
	UFUNCTION(BlueprintPure, Category = "DungeonStance|Match")
	float GetPhaseEndServerTime() const { return PhaseEndServerTime; }

	/** Segundos até o fim da fase atual (0 se não houver prazo). */
	UFUNCTION(BlueprintPure, Category = "DungeonStance|Match")
	float GetPhaseTimeRemaining() const;

	UFUNCTION(BlueprintPure, Category = "DungeonStance|Match")
	int32 GetEnemiesRemaining() const { return EnemiesRemaining; }

	// ----- Só servidor (ADSGameMode) -----
	void ServerSetWave(int32 InWave, int32 InMaxWaves, bool bInIsBossWave);
	void ServerSetMatchPhase(EDSMatchPhase InPhase, float InPhaseEndServerTime);
	/** Muda só o prazo da fase atual (ex.: todos escolheram as cartas). */
	void ServerSetPhaseEndTime(float InPhaseEndServerTime);
	void ServerSetTeamXP(int32 InLevel, int32 InXP, int32 InXPToNext);
	void ServerSetEnemiesRemaining(int32 InRemaining);

	// ----- Delegates -----
	UPROPERTY(BlueprintAssignable, Category = "DungeonStance|Match")
	FDSOnWaveChanged OnWaveChanged;

	UPROPERTY(BlueprintAssignable, Category = "DungeonStance|Match")
	FDSOnMatchPhaseChanged OnMatchPhaseChanged;

	UPROPERTY(BlueprintAssignable, Category = "DungeonStance|Match")
	FDSOnTeamXPChanged OnTeamXPChanged;

	UPROPERTY(BlueprintAssignable, Category = "DungeonStance|Match")
	FDSOnEnemiesRemainingChanged OnEnemiesRemainingChanged;

protected:
	UFUNCTION()
	void OnRep_Wave();

	UFUNCTION()
	void OnRep_MatchPhase();

	UFUNCTION()
	void OnRep_TeamXP();

	UFUNCTION()
	void OnRep_EnemiesRemaining();

	UPROPERTY(ReplicatedUsing = OnRep_Wave)
	int32 CurrentWave = 0;

	UPROPERTY(ReplicatedUsing = OnRep_Wave)
	int32 MaxWaves = DSConstants::MaxWaves;

	UPROPERTY(ReplicatedUsing = OnRep_Wave)
	bool bIsBossWave = false;

	UPROPERTY(ReplicatedUsing = OnRep_MatchPhase)
	EDSMatchPhase MatchPhase = EDSMatchPhase::WaitingForPlayers;

	UPROPERTY(ReplicatedUsing = OnRep_TeamXP)
	int32 TeamLevel = 1;

	UPROPERTY(ReplicatedUsing = OnRep_TeamXP)
	int32 TeamXP = 0;

	UPROPERTY(ReplicatedUsing = OnRep_TeamXP)
	int32 TeamXPToNext = 0;

	UPROPERTY(Replicated)
	float PhaseEndServerTime = 0.f;

	UPROPERTY(ReplicatedUsing = OnRep_EnemiesRemaining)
	int32 EnemiesRemaining = 0;
};
