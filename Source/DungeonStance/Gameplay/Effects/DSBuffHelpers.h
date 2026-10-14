#pragma once

#include "CoreMinimal.h"
#include "GameplayEffectTypes.h"
#include "ActiveGameplayEffectHandle.h"
#include "Templates/SubclassOf.h"

class UAbilitySystemComponent;
class UGameplayEffect;

/** Valores de um buff de apoio (todos os SetByCaller do UDSGE_SupportBuff). */
struct FDSBuffValues
{
	float DamageMultiplier = 1.f;
	float AttackSpeedMultiplier = 1.f;
	float MoveSpeedMultiplier = 1.f;
	float ArmorBonus = 0.f;
	float DamageReduction = 0.f;
	float CritChanceBonus = 0.f;

	bool IsNeutral() const
	{
		return DamageMultiplier == 1.f && AttackSpeedMultiplier == 1.f && MoveSpeedMultiplier == 1.f
			&& ArmorBonus == 0.f && DamageReduction == 0.f && CritChanceBonus == 0.f;
	}
};

namespace DSBuffHelpers
{
	/** Aplica um buff (UDSGE_SupportBuff ou filho) de Source em Target. Só servidor. */
	DUNGEONSTANCE_API FActiveGameplayEffectHandle ApplyBuff(UAbilitySystemComponent* Source, UAbilitySystemComponent* Target,
		TSubclassOf<UGameplayEffect> BuffClass, const FDSBuffValues& Values, float Duration);

	/** Cura instantânea (UDSGE_Heal). */
	DUNGEONSTANCE_API void ApplyHeal(UAbilitySystemComponent* Source, UAbilitySystemComponent* Target, float Amount);

	/** Escudo instantâneo (UDSGE_AddShield). */
	DUNGEONSTANCE_API void ApplyShield(UAbilitySystemComponent* Source, UAbilitySystemComponent* Target, float Amount);
}
