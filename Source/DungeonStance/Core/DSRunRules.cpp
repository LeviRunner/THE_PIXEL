#include "Core/DSRunRules.h"

namespace DSRunRules
{
	namespace
	{
		/** Arredonda para 3 casas como o round3 do progression.ts. */
		float Round3(float Value)
		{
			constexpr float Scale = 1000.f;
			return FMath::RoundToFloat(Value * Scale) / Scale;
		}

		/** Passos acima do mínimo (ex.: rank − 1), nunca negativo. */
		float StepsAbove(float Value, float Minimum)
		{
			return FMath::Max(0.f, Value - Minimum);
		}
	}

	int32 ClampPartySize(int32 NumPlayers)
	{
		return FMath::Clamp(NumPlayers, DSConstants::MinPartySize, DSConstants::MaxPartySize);
	}

	bool IsBossWave(int32 Wave, int32 MaxWaves)
	{
		if (Wave <= 0)
		{
			return false;
		}
		return (Wave % DSConstants::BossWaveInterval == 0) || Wave == MaxWaves;
	}

	int32 GetRegularEnemyCount(int32 Wave)
	{
		return BaseRegularEnemies + RegularEnemiesPerWave * FMath::Max(0, Wave);
	}

	float GetWaveHealthMultiplier(int32 Wave, float AvgRank)
	{
		const float WaveScale = 1.f + HealthPerWave * StepsAbove(Wave, 1.f);
		const float RankScale = 1.f + HealthPerRank * StepsAbove(AvgRank, 1.f);
		return WaveScale * RankScale;
	}

	float GetWaveDamageMultiplier(int32 Wave, float AvgRank)
	{
		const float WaveScale = 1.f + DamagePerWave * StepsAbove(Wave, 1.f);
		const float RankScale = 1.f + DamagePerRank * StepsAbove(AvgRank, 1.f);
		return WaveScale * RankScale;
	}

	float GetPartyEnemyCountMultiplier(int32 NumPlayers)
	{
		return PartyEnemyCountBase + PartyEnemyCountPerPlayer * ClampPartySize(NumPlayers);
	}

	float GetPartyBossHealthMultiplier(int32 NumPlayers)
	{
		return PartyBossHealthBase + PartyBossHealthPerPlayer * ClampPartySize(NumPlayers);
	}

	int32 GetTeamXPToNext(int32 Level)
	{
		return TeamXPBase + TeamXPPerLevel * FMath::Max(1, Level);
	}

	int32 GetRankXPToNext(int32 Rank)
	{
		if (Rank >= DSConstants::MaxClassRank)
		{
			return 0;
		}
		const float SafeRank = static_cast<float>(FMath::Max(1, Rank));
		return FMath::RoundToInt(RankXPBase * FMath::Pow(SafeRank, RankXPExponent));
	}

	int32 GetEndOfRunRankXP(int32 WavesCleared, bool bVictory, int32 Kills)
	{
		const int32 KillXP = FMath::Min(RunXPKillCap, FMath::Max(0, Kills) * RunXPPerKill);
		const int32 WaveXP = FMath::Max(0, WavesCleared) * RunXPPerWave;
		return RunXPParticipation + WaveXP + (bVictory ? RunXPVictoryBonus : 0) + KillXP;
	}

	FVector GetSpawnOffset(int32 Index, int32 NumPlayers, float RadiusCm)
	{
		const int32 Count = FMath::Max(1, NumPlayers);
		const float Angle = (static_cast<float>(Index) / static_cast<float>(Count)) * UE_TWO_PI;
		return FVector(FMath::Cos(Angle) * RadiusCm, FMath::Sin(Angle) * RadiusCm, 0.f);
	}

	int32 GetArenaEffectiveLevel(int32 ArenaIndex, int32 Torment)
	{
		return ArenaBaseLevel + ArenaLevelPerIndex * FMath::Max(0, ArenaIndex) + ArenaLevelPerTorment * FMath::Max(0, Torment);
	}

	int32 GetWaveGold(int32 Wave, int32 Kills)
	{
		return GoldPerWaveBase + GoldPerWaveStep * FMath::Max(0, Wave) + GoldPerKill * FMath::Max(0, Kills);
	}

	int32 GetScaledWaveEnemyCount(int32 Wave, int32 MaxWaves, int32 NumPlayers)
	{
		const float Scaled = GetRegularEnemyCount(Wave) * GetPartyEnemyCountMultiplier(NumPlayers);
		const int32 Regular = FMath::Max(1, FMath::RoundToInt(Scaled));
		return Regular + (IsBossWave(Wave, MaxWaves) ? 1 : 0);
	}

	FTeamXPResult AddTeamXP(int32 CurrentXP, int32 CurrentLevel, int32 Amount)
	{
		FTeamXPResult Result;
		Result.XP = CurrentXP + FMath::Max(0, Amount);
		Result.Level = FMath::Max(1, CurrentLevel);
		while (Result.XP >= GetTeamXPToNext(Result.Level))
		{
			Result.XP -= GetTeamXPToNext(Result.Level);
			++Result.Level;
		}
		Result.LevelsGained = Result.Level - FMath::Max(1, CurrentLevel);
		return Result;
	}

	int32 GetPendingPicks(int32 TeamLevel, int32 PicksUsed)
	{
		return FMath::Max(0, TeamLevel - 1 - PicksUsed);
	}

	float GetRankHealthMultiplier(int32 Rank)
	{
		return Round3(1.f + RankHealthBonusPerRank * StepsAbove(Rank, 1.f));
	}

	float GetRankDamageMultiplier(int32 Rank)
	{
		return Round3(1.f + RankDamageBonusPerRank * StepsAbove(Rank, 1.f));
	}

	int32 GetEnemyCombatXP(float BaseHealth)
	{
		constexpr float XPBase = 4.f;
		constexpr float XPPerHealth = 0.05f;
		return FMath::RoundToInt(XPBase + FMath::Max(0.f, BaseHealth) * XPPerHealth);
	}

	float GetEnemyBaseHealth(int32 Level)
	{
		constexpr float Base = 50.f;
		constexpr float PerLevel = 16.f;
		constexpr float PerLevelSquared = 0.5f;
		const float L = static_cast<float>(Level);
		return Base + PerLevel * L + PerLevelSquared * L * L;
	}

	float GetEnemyBaseDPS(int32 Level)
	{
		constexpr float Base = 12.f;
		constexpr float PerLevel = 4.5f;
		return Base + PerLevel * static_cast<float>(Level);
	}

	int32 GetEnemyAverageGold(int32 Level)
	{
		constexpr float Base = 2.f;
		constexpr float PerLevel = 1.6f;
		constexpr float PerLevelSquared = 0.05f;
		const float L = static_cast<float>(Level);
		return FMath::RoundToInt(Base + PerLevel * L + PerLevelSquared * L * L);
	}

	int32 GetWaveSeed(int32 RunSeed, int32 Wave)
	{
		// Soma em 64 bits para não estourar int32 (comportamento indefinido em C++).
		const int64 Seed = static_cast<int64>(RunSeed) + static_cast<int64>(Wave) * WaveSeedPrime;
		return static_cast<int32>(Seed & 0x7fffffff);
	}
}
