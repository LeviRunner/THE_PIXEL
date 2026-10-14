#pragma once

#include "CoreMinimal.h"
#include "Gameplay/Data/DSEnemyData.h"

/**
 * Fórmulas puras do bestiário (bestiary.ts §8.1), portadas do protótipo.
 * Minion padrão de nível L: vida 50 + 16L + 0,5L², DPS 12 + 4,5L, armadura 2L,
 * XP round(4 + 0,05 × vida), ouro médio round(2 + 1,6L + 0,05L²).
 */
namespace DSEnemyRules
{
	inline float BaseHealth(int32 Level) { return 50.f + 16.f * Level + 0.5f * Level * Level; }
	inline float BaseDps(int32 Level) { return 12.f + 4.5f * Level; }
	inline float BaseArmor(int32 Level) { return 2.f * Level; }
	inline int32 CombatXP(float Health) { return FMath::RoundToInt(4.f + Health * 0.05f); }
	inline int32 AverageGold(int32 Level) { return FMath::RoundToInt(2.f + 1.6f * Level + 0.05f * Level * Level); }

	/** Defaults do protótipo por arquétipo (px × 2,5 = cm). */
	inline FDSEnemyArchetypeStats GetArchetypeDefaults(EDSEnemyArchetype Archetype)
	{
		auto Make = [](float Hp, float Dps, float Speed, float Radius, bool bRanged = false, float Cooldown = 0.f, float Range = 0.f, float ProjSpeed = 0.f)
		{
			FDSEnemyArchetypeStats S;
			S.HealthMultiplier = Hp;
			S.DpsMultiplier = Dps;
			S.MoveSpeed = Speed;
			S.Radius = Radius;
			S.bRanged = bRanged;
			S.AttackCooldown = Cooldown;
			S.AttackRange = Range;
			S.ProjectileSpeed = ProjSpeed;
			return S;
		};
		switch (Archetype)
		{
		case EDSEnemyArchetype::Runner:  return Make(0.6f, 0.7f, 325.f, 27.5f);
		case EDSEnemyArchetype::Brute:   return Make(3.7f, 2.3f, 112.5f, 55.f);
		case EDSEnemyArchetype::Shooter: return Make(0.75f, 0.4f, 150.f, 32.5f, true, 2.6f, 850.f, 700.f);
		case EDSEnemyArchetype::Caster:  return Make(0.8f, 0.5f, 137.5f, 32.5f, true, 4.0f, 750.f, 475.f);
		case EDSEnemyArchetype::Swarm:   return Make(0.3f, 0.35f, 275.f, 22.5f);
		case EDSEnemyArchetype::Passive: return Make(0.5f, 0.3f, 150.f, 30.f);
		case EDSEnemyArchetype::Boss:    return Make(30.f, 3.5f, 130.f, 85.f);
		case EDSEnemyArchetype::Minion:
		default:                         return Make(1.f, 1.f, 175.f, 35.f);
		}
	}

	/** archetypeStats() do protótipo (arredonda a vida base antes do multiplicador, como a tabela §8.3). */
	inline FDSEnemyStats ComputeStats(const FDSEnemyArchetypeStats& A, int32 Level, bool bElite, float EliteHp, float EliteDps,
		float EliteRadius, float ProjectileDpsFactor, float SpeedMultiplier)
	{
		const int32 SafeLevel = FMath::Max(1, Level);
		FDSEnemyStats S;
		S.MaxHealth = FMath::RoundToFloat(FMath::RoundToFloat(BaseHealth(SafeLevel)) * A.HealthMultiplier * (bElite ? EliteHp : 1.f));
		const float Dps = FMath::RoundToFloat(BaseDps(SafeLevel));
		S.ContactDps = FMath::RoundToFloat(Dps * A.DpsMultiplier * (bElite ? EliteDps : 1.f));
		S.ProjectileDamage = A.bRanged ? FMath::RoundToFloat(Dps * ProjectileDpsFactor * (bElite ? EliteDps : 1.f)) : 0.f;
		S.Armor = BaseArmor(SafeLevel);
		S.MoveSpeed = A.MoveSpeed * SpeedMultiplier;
		S.Radius = A.Radius + (bElite ? EliteRadius : 0.f);
		S.AttackRange = A.bRanged ? A.AttackRange : 0.f;
		S.AttackCooldown = A.AttackCooldown;
		S.ProjectileSpeed = A.ProjectileSpeed;
		S.CombatXP = CombatXP(S.MaxHealth);
		S.AverageGold = AverageGold(SafeLevel);
		return S;
	}
}
