#include "Gameplay/Effects/DSGameplayEffects.h"

#include "Gameplay/Attributes/DSAttributeSet.h"
#include "Gameplay/Cards/DSCardStatBindings.h"
#include "Gameplay/DSGameplayTagsGameplay.h"
#include "Gameplay/Effects/DSDamageExecution.h"
#include "Core/DSGameplayTags.h"

UDSGE_Damage::UDSGE_Damage()
{
	DurationPolicy = EGameplayEffectDurationType::Instant;
	FGameplayEffectExecutionDefinition Execution;
	Execution.CalculationClass = UDSDamageExecution::StaticClass();
	Executions.Add(Execution);
}

UDSGE_Heal::UDSGE_Heal()
{
	DurationPolicy = EGameplayEffectDurationType::Instant;
	AddSetByCallerModifier(UDSAttributeSet::GetIncomingHealAttribute(), EGameplayModOp::Additive, DSTags::Data_Heal);
}

UDSGE_AddShield::UDSGE_AddShield()
{
	DurationPolicy = EGameplayEffectDurationType::Instant;
	AddSetByCallerModifier(UDSAttributeSet::GetShieldAttribute(), EGameplayModOp::Additive, DSTags::Data_Magnitude);
}

UDSGE_Cost::UDSGE_Cost()
{
	DurationPolicy = EGameplayEffectDurationType::Instant;
	AddSetByCallerModifier(UDSAttributeSet::GetResourceAttribute(), EGameplayModOp::Additive, DSTags::Data_Magnitude);
}

UDSGE_Cooldown::UDSGE_Cooldown()
{
	SetDurationBySetByCaller(DSTags::Data_Duration);
}

UDSGE_Invulnerable::UDSGE_Invulnerable()
{
	SetDurationBySetByCaller(DSTags::Data_Duration);
	AddGrantedTags(FGameplayTagContainer(DSTags::State_Invulnerable));
}

UDSGE_SupportBuff::UDSGE_SupportBuff()
{
	SetDurationBySetByCaller(DSTags::Data_Duration);
	AddGrantedTags(FGameplayTagContainer(DSGameplayTags::Buff_Support));
	AddSetByCallerModifier(UDSAttributeSet::GetDamageAttribute(), EGameplayModOp::Multiplicitive, DSGameplayTags::Data_Buff_Damage);
	AddSetByCallerModifier(UDSAttributeSet::GetAttackSpeedAttribute(), EGameplayModOp::Multiplicitive, DSGameplayTags::Data_Buff_AttackSpeed);
	AddSetByCallerModifier(UDSAttributeSet::GetMoveSpeedAttribute(), EGameplayModOp::Multiplicitive, DSGameplayTags::Data_Buff_MoveSpeed);
	AddSetByCallerModifier(UDSAttributeSet::GetArmorAttribute(), EGameplayModOp::Additive, DSGameplayTags::Data_Buff_Armor);
	AddSetByCallerModifier(UDSAttributeSet::GetDamageReductionAttribute(), EGameplayModOp::Additive, DSGameplayTags::Data_Buff_DamageReduction);
	AddSetByCallerModifier(UDSAttributeSet::GetCritChanceAttribute(), EGameplayModOp::Additive, DSGameplayTags::Data_Buff_CritChance);
}

UDSGE_ZoneBuff::UDSGE_ZoneBuff()
{
	SetSourceStacking(1);
}

UDSGE_CardStats::UDSGE_CardStats()
{
	DurationPolicy = EGameplayEffectDurationType::Infinite;
	for (const FDSCardStatBinding& Binding : DSCardStats::GetBindings())
	{
		AddSetByCallerModifier(Binding.Attribute, EGameplayModOp::Additive, DSGameplayTags::GetCardSetByCallerTag(Binding.AttributeName, false));
		AddSetByCallerModifier(Binding.Attribute, EGameplayModOp::Multiplicitive, DSGameplayTags::GetCardSetByCallerTag(Binding.AttributeName, true));
	}
}

UDSGE_Regen::UDSGE_Regen()
{
	DurationPolicy = EGameplayEffectDurationType::Infinite;
	Period = FScalableFloat(TickPeriod);
	bExecutePeriodicEffectOnApplication = false;
	AddTargetAttributeBasedModifier(UDSAttributeSet::GetIncomingHealAttribute(), EGameplayModOp::Additive, UDSAttributeSet::GetHealthRegenAttribute(), TickPeriod);
	AddTargetAttributeBasedModifier(UDSAttributeSet::GetResourceAttribute(), EGameplayModOp::Additive, UDSAttributeSet::GetResourceRegenAttribute(), TickPeriod);
	// Abatido/morto não regenera.
	FGameplayTagContainer Ignore;
	Ignore.AddTag(DSTags::State_Dead);
	Ignore.AddTag(DSTags::State_Downed);
	AddOngoingIgnoreTags(Ignore);
}

UDSGE_StaminaRegen::UDSGE_StaminaRegen()
{
	DurationPolicy = EGameplayEffectDurationType::Infinite;
	Period = FScalableFloat(TickPeriod);
	bExecutePeriodicEffectOnApplication = false;
	AddScalarModifier(UDSAttributeSet::GetStaminaAttribute(), EGameplayModOp::Additive, DefaultRegenPerSecond * TickPeriod);
	FGameplayTagContainer Ignore;
	Ignore.AddTag(DSTags::State_Sprinting);
	Ignore.AddTag(DSTags::State_Dead);
	Ignore.AddTag(DSTags::State_Downed);
	AddOngoingIgnoreTags(Ignore);
}

UDSGE_SprintDrain::UDSGE_SprintDrain()
{
	DurationPolicy = EGameplayEffectDurationType::Infinite;
	Period = FScalableFloat(TickPeriod);
	bExecutePeriodicEffectOnApplication = true;
	AddScalarModifier(UDSAttributeSet::GetStaminaAttribute(), EGameplayModOp::Additive, -DefaultDrainPerSecond * TickPeriod);
}
