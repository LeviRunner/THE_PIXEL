#pragma once

#include "CoreMinimal.h"

/**
 * Fórmulas puras de dano (portadas do protótipo), sem dependência de GAS — testáveis
 * em Automation Tests (Gameplay/Tests/DSDamageMathTests.cpp).
 *
 * Mitigação de armadura (BasePlayer.gd → take_damage e PLANO.md §8):
 *   herói:   dano × 100 / (100 + armadura)
 *   monstro: dano × 200 / (200 + armadura)
 */
namespace DSDamageMath
{
	inline constexpr float HeroArmorConstant = 100.f;
	inline constexpr float MonsterArmorConstant = 200.f;
	/** status.ts: lentidão máx. 70%, chefes recebem metade, atordoamento máx. 1,5 s, chefe → lentidão 30%. */
	inline constexpr float MaxSlow = 0.7f;
	inline constexpr float BossSlowFactor = 0.5f;
	inline constexpr float BossStunAsSlow = 0.3f;
	inline constexpr float MaxStunSeconds = 1.5f;

	struct FDamageParams
	{
		/** Dano base já com multiplicador da habilidade (ou dano fixo de inimigo). */
		float BaseDamage = 0.f;
		bool bCrit = false;
		float CritMultiplier = 1.5f;
		/** Marca/Exposto no alvo (frações, ex.: 0,06 = +6%). */
		float MarkVulnerability = 0.f;
		float ExposedVulnerability = 0.f;
		/** Enfraquecido no atacante (fração de redução). */
		float WeakenPct = 0.f;
		float TargetArmor = 0.f;
		float ArmorConstant = HeroArmorConstant;
		/** Redução de dano do alvo (cartas, Santuário, Totem), já limitada. */
		float TargetDamageReduction = 0.f;
	};

	inline float ArmorMitigation(float Armor, float ArmorConstant)
	{
		const float SafeConstant = FMath::Max(1.f, ArmorConstant);
		return SafeConstant / (SafeConstant + FMath::Max(0.f, Armor));
	}

	/** status.ts incomingDamageMultiplier: (1 + marca) × (1 + exposto). */
	inline float IncomingVulnerabilityMultiplier(float Mark, float Exposed)
	{
		return (1.f + FMath::Max(0.f, Mark)) * (1.f + FMath::Max(0.f, Exposed));
	}

	/** status.ts outgoingDamageMultiplier: 1 − enfraquecido. */
	inline float OutgoingWeakenMultiplier(float WeakenPct)
	{
		return 1.f - FMath::Clamp(WeakenPct, 0.f, 1.f);
	}

	inline float ComputeDamage(const FDamageParams& P)
	{
		float Damage = FMath::Max(0.f, P.BaseDamage);
		if (P.bCrit)
		{
			Damage *= FMath::Max(1.f, P.CritMultiplier);
		}
		Damage *= OutgoingWeakenMultiplier(P.WeakenPct);
		Damage *= IncomingVulnerabilityMultiplier(P.MarkVulnerability, P.ExposedVulnerability);
		Damage *= ArmorMitigation(P.TargetArmor, P.ArmorConstant);
		Damage *= 1.f - FMath::Clamp(P.TargetDamageReduction, 0.f, 1.f);
		return Damage;
	}

	/** status.ts applySlow: chefes recebem metade, teto de 70%. */
	inline float EffectiveSlow(float Pct, bool bIsBoss)
	{
		return FMath::Min(MaxSlow, FMath::Max(0.f, Pct) * (bIsBoss ? BossSlowFactor : 1.f));
	}

	/** Fúria do Bárbaro (CLASSES.md): até +30% de dano proporcional à barra. */
	inline float ResourceDamageBonus(float Resource, float MaxResource, float MaxBonus)
	{
		return MaxResource > 0.f ? 1.f + MaxBonus * FMath::Clamp(Resource / MaxResource, 0.f, 1.f) : 1.f;
	}
}
