#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "Math/RandomStream.h"
#include "Core/DSTypes.h"
#include "DSRunRulesSubsystem.generated.h"

/**
 * Wrapper Blueprint das fórmulas de DSRunRules + RNG determinístico da run.
 *
 * Uma instância por World. O ADSGameMode chama InitializeSeed() no começo da
 * run; toda aleatoriedade de regra (tipos de inimigo, posições, cartas) sai de
 * GetRunRandom() ou de MakeWaveRandom() — mesma seed, mesma run.
 * O RNG só tem significado no servidor (clientes não sorteiam regras).
 */
UCLASS()
class DUNGEONSTANCE_API UDSRunRulesSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	// ----- RNG da run -----

	/** Reinicia o RNG da run com a semente dada. */
	UFUNCTION(BlueprintCallable, Category = "DungeonStance|RunRules")
	void InitializeSeed(int32 InSeed);

	UFUNCTION(BlueprintPure, Category = "DungeonStance|RunRules")
	int32 GetRunSeed() const { return RunSeed; }

	/** RNG principal da run (servidor). */
	FRandomStream& GetRunRandom() { return RunRandom; }

	/** RNG independente de uma onda (seed + onda × 7919, como no protótipo). */
	FRandomStream MakeWaveRandom(int32 Wave) const;

	UFUNCTION(BlueprintCallable, Category = "DungeonStance|RunRules")
	int32 RandomIntInRange(int32 Min, int32 Max);

	UFUNCTION(BlueprintCallable, Category = "DungeonStance|RunRules")
	float RandomFloatInRange(float Min, float Max);

	// ----- Fórmulas (DSRunRules) -----

	UFUNCTION(BlueprintPure, Category = "DungeonStance|RunRules")
	static bool IsBossWave(int32 Wave, int32 MaxWaves);

	UFUNCTION(BlueprintPure, Category = "DungeonStance|RunRules")
	static int32 GetRegularEnemyCount(int32 Wave);

	UFUNCTION(BlueprintPure, Category = "DungeonStance|RunRules")
	static int32 GetScaledWaveEnemyCount(int32 Wave, int32 MaxWaves, int32 NumPlayers);

	UFUNCTION(BlueprintPure, Category = "DungeonStance|RunRules")
	static float GetWaveHealthMultiplier(int32 Wave, float AvgRank);

	UFUNCTION(BlueprintPure, Category = "DungeonStance|RunRules")
	static float GetWaveDamageMultiplier(int32 Wave, float AvgRank);

	UFUNCTION(BlueprintPure, Category = "DungeonStance|RunRules")
	static float GetPartyEnemyCountMultiplier(int32 NumPlayers);

	UFUNCTION(BlueprintPure, Category = "DungeonStance|RunRules")
	static float GetPartyBossHealthMultiplier(int32 NumPlayers);

	UFUNCTION(BlueprintPure, Category = "DungeonStance|RunRules")
	static int32 GetTeamXPToNext(int32 Level);

	UFUNCTION(BlueprintPure, Category = "DungeonStance|RunRules")
	static int32 GetRankXPToNext(int32 Rank);

	UFUNCTION(BlueprintPure, Category = "DungeonStance|RunRules")
	static int32 GetEndOfRunRankXP(int32 WavesCleared, bool bVictory, int32 Kills);

	UFUNCTION(BlueprintPure, Category = "DungeonStance|RunRules")
	static FVector GetSpawnOffset(int32 Index, int32 NumPlayers, float RadiusCm);

	UFUNCTION(BlueprintPure, Category = "DungeonStance|RunRules")
	static int32 GetArenaEffectiveLevel(int32 ArenaIndex, int32 Torment);

	UFUNCTION(BlueprintPure, Category = "DungeonStance|RunRules")
	static int32 GetWaveGold(int32 Wave, int32 Kills);

	UFUNCTION(BlueprintPure, Category = "DungeonStance|RunRules")
	static float GetRankHealthMultiplier(int32 Rank);

	UFUNCTION(BlueprintPure, Category = "DungeonStance|RunRules")
	static float GetRankDamageMultiplier(int32 Rank);

private:
	int32 RunSeed = 0;
	FRandomStream RunRandom;
};
