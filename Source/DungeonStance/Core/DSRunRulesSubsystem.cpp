#include "Core/DSRunRulesSubsystem.h"

#include "Core/DSRunRules.h"
#include "DungeonStance.h"

void UDSRunRulesSubsystem::InitializeSeed(int32 InSeed)
{
	RunSeed = InSeed;
	RunRandom.Initialize(InSeed);
	UE_LOG(LogDungeonStance, Log, TEXT("RunRules: semente da run = %d"), InSeed);
}

FRandomStream UDSRunRulesSubsystem::MakeWaveRandom(int32 Wave) const
{
	return FRandomStream(DSRunRules::GetWaveSeed(RunSeed, Wave));
}

int32 UDSRunRulesSubsystem::RandomIntInRange(int32 Min, int32 Max)
{
	return RunRandom.RandRange(Min, Max);
}

float UDSRunRulesSubsystem::RandomFloatInRange(float Min, float Max)
{
	return RunRandom.FRandRange(Min, Max);
}

bool UDSRunRulesSubsystem::IsBossWave(int32 Wave, int32 MaxWaves)
{
	return DSRunRules::IsBossWave(Wave, MaxWaves);
}

int32 UDSRunRulesSubsystem::GetRegularEnemyCount(int32 Wave)
{
	return DSRunRules::GetRegularEnemyCount(Wave);
}

int32 UDSRunRulesSubsystem::GetScaledWaveEnemyCount(int32 Wave, int32 MaxWaves, int32 NumPlayers)
{
	return DSRunRules::GetScaledWaveEnemyCount(Wave, MaxWaves, NumPlayers);
}

float UDSRunRulesSubsystem::GetWaveHealthMultiplier(int32 Wave, float AvgRank)
{
	return DSRunRules::GetWaveHealthMultiplier(Wave, AvgRank);
}

float UDSRunRulesSubsystem::GetWaveDamageMultiplier(int32 Wave, float AvgRank)
{
	return DSRunRules::GetWaveDamageMultiplier(Wave, AvgRank);
}

float UDSRunRulesSubsystem::GetPartyEnemyCountMultiplier(int32 NumPlayers)
{
	return DSRunRules::GetPartyEnemyCountMultiplier(NumPlayers);
}

float UDSRunRulesSubsystem::GetPartyBossHealthMultiplier(int32 NumPlayers)
{
	return DSRunRules::GetPartyBossHealthMultiplier(NumPlayers);
}

int32 UDSRunRulesSubsystem::GetTeamXPToNext(int32 Level)
{
	return DSRunRules::GetTeamXPToNext(Level);
}

int32 UDSRunRulesSubsystem::GetRankXPToNext(int32 Rank)
{
	return DSRunRules::GetRankXPToNext(Rank);
}

int32 UDSRunRulesSubsystem::GetEndOfRunRankXP(int32 WavesCleared, bool bVictory, int32 Kills)
{
	return DSRunRules::GetEndOfRunRankXP(WavesCleared, bVictory, Kills);
}

FVector UDSRunRulesSubsystem::GetSpawnOffset(int32 Index, int32 NumPlayers, float RadiusCm)
{
	return DSRunRules::GetSpawnOffset(Index, NumPlayers, RadiusCm);
}

int32 UDSRunRulesSubsystem::GetArenaEffectiveLevel(int32 ArenaIndex, int32 Torment)
{
	return DSRunRules::GetArenaEffectiveLevel(ArenaIndex, Torment);
}

int32 UDSRunRulesSubsystem::GetWaveGold(int32 Wave, int32 Kills)
{
	return DSRunRules::GetWaveGold(Wave, Kills);
}

float UDSRunRulesSubsystem::GetRankHealthMultiplier(int32 Rank)
{
	return DSRunRules::GetRankHealthMultiplier(Rank);
}

float UDSRunRulesSubsystem::GetRankDamageMultiplier(int32 Rank)
{
	return DSRunRules::GetRankDamageMultiplier(Rank);
}
