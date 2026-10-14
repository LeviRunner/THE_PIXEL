#include "Gameplay/Cards/DSCardStatBindings.h"

#include "Gameplay/Attributes/DSAttributeSet.h"

namespace DSCardStats
{
	const TArray<FDSCardStatBinding>& GetBindings()
	{
		// Manter em sincronia com DS_CARD_TAG_PAIR em DSGameplayTagsGameplay.cpp.
		static const TArray<FDSCardStatBinding> Bindings = {
			{ TEXT("max_health"), TEXT("MaxHealth"), UDSAttributeSet::GetMaxHealthAttribute() },
			{ TEXT("damage"), TEXT("Damage"), UDSAttributeSet::GetDamageAttribute() },
			{ TEXT("attack_speed"), TEXT("AttackSpeed"), UDSAttributeSet::GetAttackSpeedAttribute() },
			{ TEXT("attack_range"), TEXT("AttackRange"), UDSAttributeSet::GetAttackRangeAttribute() },
			{ TEXT("move_speed"), TEXT("MoveSpeed"), UDSAttributeSet::GetMoveSpeedAttribute() },
			{ TEXT("crit_chance"), TEXT("CritChance"), UDSAttributeSet::GetCritChanceAttribute() },
			{ TEXT("crit_damage"), TEXT("CritDamage"), UDSAttributeSet::GetCritDamageAttribute() },
			{ TEXT("armor"), TEXT("Armor"), UDSAttributeSet::GetArmorAttribute() },
			{ TEXT("cooldown_reduction"), TEXT("CooldownReduction"), UDSAttributeSet::GetCooldownReductionAttribute() },
			{ TEXT("resource_max"), TEXT("MaxResource"), UDSAttributeSet::GetMaxResourceAttribute() },
			{ TEXT("lifesteal"), TEXT("Lifesteal"), UDSAttributeSet::GetLifestealAttribute() },
			{ TEXT("health_regen"), TEXT("HealthRegen"), UDSAttributeSet::GetHealthRegenAttribute() },
			{ TEXT("resource_regen"), TEXT("ResourceRegen"), UDSAttributeSet::GetResourceRegenAttribute() },
			{ TEXT("damage_reduction"), TEXT("DamageReduction"), UDSAttributeSet::GetDamageReductionAttribute() },
			{ TEXT("dodge_chance"), TEXT("DodgeChance"), UDSAttributeSet::GetDodgeChanceAttribute() },
		};
		return Bindings;
	}

	const FDSCardStatBinding* FindByCardStat(FName CardStat)
	{
		return GetBindings().FindByPredicate([CardStat](const FDSCardStatBinding& Binding)
		{
			return Binding.CardStat == CardStat;
		});
	}
}
