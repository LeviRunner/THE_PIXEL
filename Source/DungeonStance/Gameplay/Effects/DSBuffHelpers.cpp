#include "Gameplay/Effects/DSBuffHelpers.h"

#include "AbilitySystemComponent.h"
#include "GameplayEffect.h"
#include "Gameplay/DSGameplayTagsGameplay.h"
#include "Gameplay/Effects/DSGameplayEffects.h"
#include "Core/DSGameplayTags.h"

namespace
{
	FGameplayEffectSpecHandle MakeSpec(UAbilitySystemComponent* Source, TSubclassOf<UGameplayEffect> EffectClass)
	{
		if (!Source || !EffectClass || !Source->IsOwnerActorAuthoritative())
		{
			return FGameplayEffectSpecHandle();
		}
		return Source->MakeOutgoingSpec(EffectClass, 1.f, Source->MakeEffectContext());
	}

	void ApplySpec(UAbilitySystemComponent* Source, UAbilitySystemComponent* Target, const FGameplayEffectSpecHandle& Spec)
	{
		if (Spec.IsValid() && Target)
		{
			Source->ApplyGameplayEffectSpecToTarget(*Spec.Data.Get(), Target);
		}
	}
}

namespace DSBuffHelpers
{
	FActiveGameplayEffectHandle ApplyBuff(UAbilitySystemComponent* Source, UAbilitySystemComponent* Target,
		TSubclassOf<UGameplayEffect> BuffClass, const FDSBuffValues& Values, float Duration)
	{
		const FGameplayEffectSpecHandle Spec = MakeSpec(Source, BuffClass ? BuffClass : TSubclassOf<UGameplayEffect>(UDSGE_SupportBuff::StaticClass()));
		if (!Spec.IsValid() || !Target || Duration <= 0.f || Values.IsNeutral())
		{
			return FActiveGameplayEffectHandle();
		}
		FGameplayEffectSpec& Data = *Spec.Data.Get();
		Data.SetSetByCallerMagnitude(DSTags::Data_Duration, Duration);
		Data.SetSetByCallerMagnitude(DSGameplayTags::Data_Buff_Damage, Values.DamageMultiplier);
		Data.SetSetByCallerMagnitude(DSGameplayTags::Data_Buff_AttackSpeed, Values.AttackSpeedMultiplier);
		Data.SetSetByCallerMagnitude(DSGameplayTags::Data_Buff_MoveSpeed, Values.MoveSpeedMultiplier);
		Data.SetSetByCallerMagnitude(DSGameplayTags::Data_Buff_Armor, Values.ArmorBonus);
		Data.SetSetByCallerMagnitude(DSGameplayTags::Data_Buff_DamageReduction, Values.DamageReduction);
		Data.SetSetByCallerMagnitude(DSGameplayTags::Data_Buff_CritChance, Values.CritChanceBonus);
		return Source->ApplyGameplayEffectSpecToTarget(Data, Target);
	}

	void ApplyHeal(UAbilitySystemComponent* Source, UAbilitySystemComponent* Target, float Amount)
	{
		if (Amount <= 0.f)
		{
			return;
		}
		const FGameplayEffectSpecHandle Spec = MakeSpec(Source, UDSGE_Heal::StaticClass());
		if (Spec.IsValid())
		{
			Spec.Data->SetSetByCallerMagnitude(DSTags::Data_Heal, Amount);
			ApplySpec(Source, Target, Spec);
		}
	}

	void ApplyShield(UAbilitySystemComponent* Source, UAbilitySystemComponent* Target, float Amount)
	{
		if (Amount <= 0.f)
		{
			return;
		}
		const FGameplayEffectSpecHandle Spec = MakeSpec(Source, UDSGE_AddShield::StaticClass());
		if (Spec.IsValid())
		{
			Spec.Data->SetSetByCallerMagnitude(DSTags::Data_Magnitude, Amount);
			ApplySpec(Source, Target, Spec);
		}
	}
}
