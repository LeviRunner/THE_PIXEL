#include "Gameplay/Abilities/DSSupportResolver.h"

namespace DSSupportResolver
{
	float SupportCooldownReduction(const UDSSupportAbilityData& Data, FCardStatFn CardStat)
	{
		const float Cap = MaxSupportCooldownReduction + CardStat(TEXT("support_cdr_cap_bonus"), 0.f);
		float Reduction = CardStat(TEXT("support_cooldown_reduction"), 0.f);
		if (Data.bIsHeal)
		{
			Reduction += CardStat(TEXT("heal_cooldown_reduction"), 0.f);
		}
		return FMath::Clamp(Reduction, 0.f, Cap);
	}

	namespace
	{
		void ResolveKind(const UDSSupportAbilityData& Data, FCardStatFn CardStat, float Power, FDSResolvedSupport& R)
		{
			const float Heals = CardStat(TEXT("support_heal_power"), 0.f);
			const float HealMult = CardStat(TEXT("support_heal_mult"), 1.f);
			switch (Data.Kind)
			{
			case EDSSupportKind::DamageBuff:
				R.DamageMultiplier = 1.f + R.Magnitude;
				R.CritChanceBonus = CardStat(TEXT("aura_crit_bonus"), 0.f);
				break;
			case EDSSupportKind::Haste:
				R.AttackSpeedMultiplier = 1.f + R.Magnitude;
				break;
			case EDSSupportKind::Heal:
				R.Magnitude = Data.Magnitude * (1.f + Power + Heals) * HealMult;
				R.RegenPct = CardStat(TEXT("pulse_hot"), 0.f) * (1.f + Heals) * HealMult / 4.f;
				if (R.RegenPct > 0.f)
				{
					R.Duration = 4.f;
				}
				break;
			case EDSSupportKind::Shield:
				R.Magnitude *= CardStat(TEXT("support_shield_power"), 1.f);
				R.ArmorBonus = CardStat(TEXT("bastion_armor"), 0.f);
				R.RegenPct = CardStat(TEXT("sentinel_regen"), 0.f);
				if (CardStat(TEXT("aegis_of_many"), 0.f) > 0.f)
				{
					R.Duration += 3.f;
				}
				break;
			case EDSSupportKind::Zone:
				if (Data.bZoneDamageBuff)
				{
					R.DamageMultiplier = 1.f + R.Magnitude;
					R.ArmorBonus = FMath::RoundToFloat(Data.ZoneArmorBonus * (1.f + Power));
				}
				if (Data.bZoneRegen)
				{
					R.Magnitude = Data.Magnitude * (1.f + Power + Heals) * HealMult;
					R.RegenPct = R.Magnitude;
				}
				if (Data.bZoneMagnitudeIsDamageReduction)
				{
					R.DamageReduction = FMath::Min(MaxZoneDamageReduction, R.Magnitude);
					R.RegenPct = CardStat(TEXT("sentinel_regen"), 0.f);
				}
				break;
			case EDSSupportKind::Rally:
				R.MoveSpeedMultiplier = 1.f + R.Magnitude;
				break;
			default:
				break;
			}
		}
	}

	FDSResolvedSupport Resolve(const UDSSupportAbilityData& Data, FCardStatFn CardStat)
	{
		FDSResolvedSupport R;
		R.Kind = Data.Kind;
		R.bTaunts = Data.bTaunts;
		R.DamageReduction = Data.DamageReduction;

		const float Power = CardStat(TEXT("support_power"), 0.f);
		R.Radius = FMath::RoundToFloat(Data.Radius * CardStat(TEXT("support_radius"), 1.f));
		R.Duration = Data.Duration > 0.f
			? (Data.Duration + CardStat(TEXT("support_duration"), 0.f)) * CardStat(TEXT("support_duration_mult"), 1.f)
			: 0.f;
		R.Magnitude = Data.Magnitude * (1.f + Power);

		ResolveKind(Data, CardStat, Power, R);

		// casterBuffMods: Vanguarda (+movimento) e Lenda Viva (+dano) em todo buff com duração.
		if (R.Duration > 0.f)
		{
			R.MoveSpeedMultiplier *= 1.f + CardStat(TEXT("buffed_ally_move"), 0.f);
			R.DamageMultiplier *= 1.f + CardStat(TEXT("buff_stack_damage"), 0.f);
		}
		R.Cooldown = Data.Cooldown * (1.f - SupportCooldownReduction(Data, CardStat));
		return R;
	}
}
