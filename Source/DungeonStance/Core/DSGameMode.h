#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "Core/DSArenaTypes.h"
#include "Core/DSRunRules.h"
#include "Core/DSTypes.h"
#include "DSGameMode.generated.h"

class ADSEnemyCharacter;
class ADSGameState;
class ADSHeroCharacter;
class ADSPlayerController;
class ADSPlayerState;
class UDSRunRulesSubsystem;

/** Inimigo vivo da onda + XP de time que ele dá ao morrer (não refletido). */
struct FDSLiveEnemyRecord
{
	TWeakObjectPtr<ADSEnemyCharacter> Enemy;
	int32 XP = 0;
};

DECLARE_MULTICAST_DELEGATE_TwoParams(FDSOnRunFinishedNative, bool /*bVictory*/, const TArray<FDSRunResult>& /*Results*/);

/**
 * Ciclo da partida do Dungeon Stance (só servidor) — porta de squad_match.ts.
 *
 *  WaitingForPlayers ─(todos entraram e prontos | 30 s com alguém)→ BetweenWaves (4 s)
 *  → WaveInProgress ─(onda limpa | 150 s)→ CardPick (20+2 s, se há escolha) ou BetweenWaves (4 s)
 *  → ... onda 10 limpa → Victory | todos caídos → Defeat → (10 s) → Ended → fecha a partida.
 *  Partida sem ninguém conectado por 15 s fecha sem recompensa.
 *
 * Tudo dirigido por FTimerHandle; o GameMode não tem Tick.
 * Cada arena é um Blueprint filho com a lista de inimigos/chefes e o índice.
 */
UCLASS()
class DUNGEONSTANCE_API ADSGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	ADSGameMode(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	// ----- AGameModeBase -----
	virtual void InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage) override;
	virtual void StartPlay() override;
	virtual FString InitNewPlayer(APlayerController* NewPlayerController, const FUniqueNetIdRepl& UniqueId, const FString& Options, const FString& Portal = TEXT("")) override;
	virtual void PostLogin(APlayerController* NewPlayer) override;
	virtual void Logout(AController* Exiting) override;
	virtual void HandleStartingNewPlayer_Implementation(APlayerController* NewPlayer) override;
	virtual UClass* GetDefaultPawnClassForController_Implementation(AController* InController) override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	// ----- API do contrato (chamada pela Gameplay) -----

	/** Inimigo morreu. Idempotente: pode vir do delegate OnEnemyDied e de chamada direta. */
	void NotifyEnemyKilled(ADSEnemyCharacter* Enemy, AController* Killer);

	/** Herói caiu (vida 0). Levanta de novo no início da próxima onda. */
	void NotifyHeroDowned(ADSHeroCharacter* Hero);

	// ----- Chamadas pelo ADSPlayerController (servidor) -----
	void NotifyPlayerReadyChanged(ADSPlayerController* Player);
	void NotifyCardPicked(ADSPlayerController* Player);

	/** Disparado no servidor quando a run termina (antes da tela final). */
	FDSOnRunFinishedNative OnRunFinished;

	UFUNCTION(BlueprintPure, Category = "DungeonStance|Match")
	int32 GetWavesCleared() const { return WavesCleared; }

	UFUNCTION(BlueprintPure, Category = "DungeonStance|Match")
	int32 GetRunSeed() const { return RunSeed; }

protected:
	// ----- Configuração da arena (Blueprint filho por arena) -----

	/** Índice da arena (0 Clareira, 1 Galerias, 2 Cemitério) → nível efetivo. */
	UPROPERTY(EditDefaultsOnly, Category = "DungeonStance|Arena", meta = (ClampMin = "0"))
	int32 ArenaIndex = 0;

	/** Tormento padrão; a opção de URL "?Torment=N" sobrescreve. */
	UPROPERTY(EditDefaultsOnly, Category = "DungeonStance|Arena", meta = (ClampMin = "0"))
	int32 Torment = 0;

	UPROPERTY(EditDefaultsOnly, Category = "DungeonStance|Arena", meta = (ClampMin = "1"))
	int32 MaxWaves = DSConstants::MaxWaves;

	UPROPERTY(EditDefaultsOnly, Category = "DungeonStance|Arena")
	TArray<FDSEnemySpawnEntry> RegularEnemies;

	UPROPERTY(EditDefaultsOnly, Category = "DungeonStance|Arena")
	TArray<FDSBossSpawnEntry> Bosses;

	/** Classe do ator inimigo; o UDSEnemyData define o resto via InitializeFromData. */
	UPROPERTY(EditDefaultsOnly, Category = "DungeonStance|Arena")
	TSubclassOf<ADSEnemyCharacter> EnemyCharacterClass;

	UPROPERTY(EditDefaultsOnly, Category = "DungeonStance|Arena")
	FDSEnemySpawnRing EnemySpawnRing;

	// ----- Heróis -----

	UPROPERTY(EditDefaultsOnly, Category = "DungeonStance|Heroes")
	TMap<EDSHeroClass, TSubclassOf<ADSHeroCharacter>> HeroPawnClasses;

	/** Usado por quem não escolheu herói até o início da run. */
	UPROPERTY(EditDefaultsOnly, Category = "DungeonStance|Heroes")
	EDSHeroClass DefaultHeroClass = EDSHeroClass::Barbarian;

	UPROPERTY(EditDefaultsOnly, Category = "DungeonStance|Heroes")
	EDSStance DefaultStance = EDSStance::Critical;

	UPROPERTY(EditDefaultsOnly, Category = "DungeonStance|Heroes", meta = (Units = "cm"))
	float HeroSpawnRadiusCm = DSRunRules::HeroSpawnRadiusCm;

	// ----- Fluxo -----

	/** Se verdadeiro, a run só começa antes do timeout quando todos marcaram "pronto". */
	UPROPERTY(EditDefaultsOnly, Category = "DungeonStance|Flow")
	bool bRequireReadyToStart = true;

	UPROPERTY(EditDefaultsOnly, Category = "DungeonStance|Flow", meta = (ClampMin = "1"))
	int32 NumCardChoices = 3;

	UPROPERTY(EditDefaultsOnly, Category = "DungeonStance|Flow", meta = (Units = "s"))
	float JoinTimeoutSec = DSRunRules::JoinTimeoutSec;

	UPROPERTY(EditDefaultsOnly, Category = "DungeonStance|Flow", meta = (Units = "s"))
	float WaveBreakSec = DSRunRules::WaveBreakSec;

	UPROPERTY(EditDefaultsOnly, Category = "DungeonStance|Flow", meta = (Units = "s"))
	float PickBreakSec = DSRunRules::PickBreakSec;

	UPROPERTY(EditDefaultsOnly, Category = "DungeonStance|Flow", meta = (Units = "s"))
	float AllPickedGraceSec = DSRunRules::AllPickedGraceSec;

	UPROPERTY(EditDefaultsOnly, Category = "DungeonStance|Flow", meta = (Units = "s"))
	float WaveTimeoutSec = DSRunRules::WaveTimeoutSec;

	UPROPERTY(EditDefaultsOnly, Category = "DungeonStance|Flow", meta = (Units = "s"))
	float EndScreenSec = DSRunRules::EndScreenSec;

	UPROPERTY(EditDefaultsOnly, Category = "DungeonStance|Flow", meta = (Units = "s"))
	float EmptyMatchTimeoutSec = DSRunRules::EmptyMatchTimeoutSec;

	/** Para onde o servidor viaja ao fechar a partida (vazio = sai/volta ao menu). */
	UPROPERTY(EditDefaultsOnly, Category = "DungeonStance|Flow")
	FString PostMatchTravelURL;

	// ----- Ganchos -----

	/** Levanta um herói caído no início da onda. Padrão: tira as tags Downed/Dead e enche a vida. */
	UFUNCTION(BlueprintNativeEvent, Category = "DungeonStance|Heroes")
	void ReviveHero(ADSHeroCharacter* Hero);
	virtual void ReviveHero_Implementation(ADSHeroCharacter* Hero);

	/** Fecha a partida (viagem, menu ou encerrar o servidor dedicado). */
	UFUNCTION(BlueprintNativeEvent, Category = "DungeonStance|Flow")
	void CloseMatch();
	virtual void CloseMatch_Implementation();

	/** XP de time dado por um inimigo recém-criado. Padrão: round(4 + 0,05 × vida base). */
	virtual int32 ComputeEnemyXP(ADSEnemyCharacter* Enemy, float HealthMultiplier, int32 EnemyLevel) const;

private:
	// Início
	void TryStartRun();
	bool AreAllPlayersReady() const;
	void HandleJoinTimeout();
	void StartRun();
	void PrepareParticipants();
	void SpawnHeroes();
	void SpawnHeroInRing(APlayerController* Player, int32 RingIndex, int32 RingSize);

	// Ondas
	void StartNextWave();
	void SpawnWaveEnemies(FRandomStream& WaveRandom);
	bool SpawnEnemy(const UDSEnemyData* Data, bool bElite, bool bBoss, const FVector& Center, const TArray<FVector>& HeroLocations, FRandomStream& WaveRandom);
	void HandleWaveTimeout();
	void HandleWaveCleared();
	void AwardWaveGold();
	void ReviveDownedHeroes();
	void RemoveAllEnemies();
	void PruneInvalidEnemies();

	// Cartas e XP
	void AddTeamXP(int32 Amount);
	void RollCardChoices(ADSPlayerState& Player);
	void RollCardChoicesForAll();
	bool AnyPendingPicks() const;
	void ClearAllCardChoices();

	// Fim
	void CheckDefeat();
	bool IsHeroUp(const ADSHeroCharacter* Hero) const;
	void FinishRun(bool bVictory);
	FDSRunResult BuildRunResult(const ADSPlayerState& Player, bool bVictory) const;
	void HandleEndScreenFinished();
	void HandleEmptyMatchTimeout();

	// Utilitários
	void SetPhase(EDSMatchPhase Phase, float DurationSec);
	void ScheduleNextWave(float DelaySec);
	float GetServerTime() const;
	int32 CountConnectedPlayers(const AController* Excluding = nullptr) const;
	TArray<ADSPlayerState*> GetDSPlayerStates() const;
	TArray<FVector> GetHeroLocations() const;
	ADSGameState* GetDSGameState() const;
	void HandleEnemyDied(ADSEnemyCharacter* Enemy, AController* Killer);

	UPROPERTY(Transient)
	TObjectPtr<UDSRunRulesSubsystem> RunRules;

	TArray<FDSLiveEnemyRecord> LiveEnemies;
	TSet<TWeakObjectPtr<const ADSHeroCharacter>> DownedHeroes;
	TMap<TWeakObjectPtr<ADSPlayerState>, int32> KillsAtWaveStart;

	FTimerHandle JoinTimeoutTimer;
	FTimerHandle PhaseTimer;
	FTimerHandle WaveTimeoutTimer;
	FTimerHandle EmptyMatchTimer;

	FTransform HeroSpawnOrigin = FTransform::Identity;
	int32 ExpectedPlayers = DSConstants::MinPartySize;
	int32 RunPartySize = DSConstants::MinPartySize;
	int32 RunSeed = 0;
	int32 CurrentWave = 0;
	int32 WavesCleared = 0;
	float AverageRank = 1.f;
	float BreakMinServerTime = 0.f;
	bool bMatchInitialized = false;
	bool bMatchClosing = false;
	bool bRunStarted = false;
	bool bRunFinished = false;
	bool bWaveActive = false;
};
