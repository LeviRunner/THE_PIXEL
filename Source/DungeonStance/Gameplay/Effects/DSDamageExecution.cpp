#include "Gameplay/Effects/DSDamageExecution.h"

#include "AbilitySystemComponent.h"
#include "Gameplay/Attributes/DSAttributeSet.h"
#include "Gameplay/Combat/DSDamageMath.h"
#include "Gameplay/Effects/DSStatusLibrary.h"
#include "Gameplay/DSGameplayTagsGameplay.h"
#include "Core/DSGameplayTags.h"

namespace
{
	struct FDSDamageStatics
	{
		FGameplayEffectAttributeCaptureDefinition SourceDamage;
		FGameplayEffectAttributeCaptureDefinition SourceCritChance;
		FGameplayEffectAttributeCaptureDefinition SourceCritDamage;
		FGameplayEffectAttributeCaptureDefinition TargetArmor;
		FGameplayEffectAttributeCaptureDefinition TargetDamageReduction;
		FGameplayEffectAttributeCaptureDefinition TargetDodgeChance;

		FDSDamageStatics()
		{
			// Atacante: snapshot na criação do spec (projéteis levam o dano do momento do disparo).
			SourceDamage = FGameplayEffectAttributeCaptureDefinition(UDSAttributeSet::GetDamageAttribute(), EGameplayEffectAttributeCaptureSource::Source, true);
			SourceCritChance = FGameplayEffectAttributeCaptureDefinition(UDSAttributeSet::GetCritChanceAttribute(), EGameplayEffectAttributeCaptureSource::Source, true);
			SourceCritDamage = FGameplayEffectAttributeCaptureDefinition(UDSAttributeSet::GetCritDamageAttribute(), EGameplayEffectAttributeCaptureSource::Source, true);
			TargetArmor = FGameplayEffectAttributeCaptureDefinition(UDSAttributeSet::GetArmorAttribute(), EGameplayEffectAttributeCaptureSource::Target, false);
			TargetDamageReduction = FGameplayEffectAttributeCaptureDefinition(UDSAttributeSet::GetDamageReductionAttribute(), EGameplayEffectAttributeCaptureSource::Target, false);
			TargetDodgeChance = FGameplayEffectAttributeCaptureDefinition(UDSAttributeSet::GetDodgeChanceAttribute(), EGameplayEffectAttributeCaptureSource::Target, false);
		}
	};

	const FDSDamageStatics& DamageStatics()
	{
		static FDSDamageStatics Statics;
		return Statics;
	}

	float CaptureOrZero(const FGameplayEffectCustomExecutionParameters& Params, const FGameplayEffectAttributeCaptureDefinition& Def, const FAggregatorEvaluateParameters& Eval)
	{
		float Value = 0.f;
		Params.AttemptCalculateCapturedAttributeMagnitude(Def, Eval, Value);
		return Value;
	}

	void SendCritEvent(UAbilitySystemComponent* SourceASC, UAbilitySystemComponent* TargetASC, float Damage)
	{
		if (!SourceASC)
		{
			return;
		}
		// Gatilho para cartas "ao critar" (Sede Crítica, Ponto Fraco...).
		FGameplayEventData Payload;
		Payload.EventTag = DSGameplayTags::Event_Crit;
		Payload.Instigator = SourceASC->GetAvatarActor();
		Payload.Target = TargetASC ? TargetASC->GetAvatarActor() : nullptr;
		Payload.EventMagnitude = Damage;
		SourceASC->HandleGameplayEvent(Payload.EventTag, &Payload);
	}
}

UDSDamageExecution::UDSDamageExecution()
{
	RelevantAttributesToCapture.Add(DamageStatics().SourceDamage);
	RelevantAttributesToCapture.Add(DamageStatics().SourceCritChance);
	RelevantAttributesToCapture.Add(DamageStatics().SourceCritDamage);
	RelevantAttributesToCapture.Add(DamageStatics().TargetArmor);
	RelevantAttributesToCapture.Add(DamageStatics().TargetDamageReduction);
	RelevantAttributesToCapture.Add(DamageStatics().TargetDodgeChance);
}

void UDSDamageExecution::Execute_Implementation(const FGameplayEffectCustomExecutionParameters& ExecutionParams,
	FGameplayEffectCustomExecutionOutput& OutExecutionOutput) const
{
	const FGameplayEffectSpec& Spec = ExecutionParams.GetOwningSpec();
	UAbilitySystemComponent* SourceASC = ExecutionParams.GetSourceAbilitySystemComponent();
	UAbilitySystemComponent* TargetASC = ExecutionParams.GetTargetAbilitySystemComponent();

	FAggregatorEvaluateParameters Eval;
	Eval.SourceTags = Spec.CapturedSourceTags.GetAggregatedTags();
	Eval.TargetTags = Spec.CapturedTargetTags.GetAggregatedTags();

	const float FlatDamage = Spec.GetSetByCallerMagnitude(DSTags::Data_Damage, false, 0.f);
	const float AbilityMultiplier = Spec.GetSetByCallerMagnitude(DSTags::Data_Magnitude, false, 1.f);
	const bool bFlat = FlatDamage > 0.f;

	DSDamageMath::FDamageParams Params;
	Params.BaseDamage = bFlat ? FlatDamage : CaptureOrZero(ExecutionParams, DamageStatics().SourceDamage, Eval) * AbilityMultiplier;
	if (!bFlat)
	{
		const float CritChance = CaptureOrZero(ExecutionParams, DamageStatics().SourceCritChance, Eval);
		Params.bCrit = FMath::FRand() < CritChance;
		Params.CritMultiplier = CaptureOrZero(ExecutionParams, DamageStatics().SourceCritDamage, Eval);
	}
	Params.WeakenPct = UDSStatusLibrary::GetStrongestStatusMagnitude(SourceASC, DSTags::Status_Weaken);
	Params.MarkVulnerability = UDSStatusLibrary::GetStrongestStatusMagnitude(TargetASC, DSTags::Status_Mark);
	Params.ExposedVulnerability = UDSStatusLibrary::GetStrongestStatusMagnitude(TargetASC, DSTags::Status_Exposed);
	Params.TargetArmor = CaptureOrZero(ExecutionParams, DamageStatics().TargetArmor, Eval);
	Params.TargetDamageReduction = CaptureOrZero(ExecutionParams, DamageStatics().TargetDamageReduction, Eval);

	const bool bTargetIsHero = TargetASC && TargetASC->HasMatchingGameplayTag(DSGameplayTags::Character_Hero);
	Params.ArmorConstant = bTargetIsHero ? DSDamageMath::HeroArmorConstant : DSDamageMath::MonsterArmorConstant;

	const float DodgeChance = CaptureOrZero(ExecutionParams, DamageStatics().TargetDodgeChance, Eval);
	if (DodgeChance > 0.f && FMath::FRand() < DodgeChance)
	{
		return; // esquiva passiva (Caçador de Demônios)
	}

	const float Damage = DSDamageMath::ComputeDamage(Params);
	if (Damage <= 0.f)
	{
		return;
	}
	OutExecutionOutput.AddOutputModifier(FGameplayModifierEvaluatedData(UDSAttributeSet::GetIncomingDamageAttribute(), EGameplayModOp::Additive, Damage));

	if (Params.bCrit)
	{
		SendCritEvent(SourceASC, TargetASC, Damage);
	}
}
