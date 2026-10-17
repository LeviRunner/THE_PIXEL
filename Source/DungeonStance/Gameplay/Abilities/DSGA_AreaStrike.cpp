#include "Gameplay/Abilities/DSGA_AreaStrike.h"

#include "AbilitySystemComponent.h"
#include "Animation/AnimMontage.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Gameplay/Attributes/DSAttributeSet.h"
#include "Gameplay/Characters/DSCharacterBase.h"
#include "Gameplay/DSGameplayTagsGameplay.h"
#include "Core/DSGameplayTags.h"

UDSGA_AreaStrike::UDSGA_AreaStrike(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	SetAssetTags(FGameplayTagContainer(DSTags::Ability_ClassSkill));
	CooldownTags.AddTag(DSTags::Cooldown_ClassSkill);
	BaseCooldown = 6.f;
	ResourceCost = 30.f;
}

void UDSGA_AreaStrike::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}
	if (Montage)
	{
		if (UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo())
		{
			ASC->PlayMontage(this, ActivationInfo, Montage, 1.f);
		}
	}

	StrikesDone = 0;
	if (!IsServer())
	{
		// Cliente só prediz custo/recarga/animação; o servidor encerra a habilidade.
		if (HitCount <= 1 && ImpactDelay <= 0.f)
		{
			EndAbility(Handle, ActorInfo, ActivationInfo, false, false);
		}
		return;
	}
	if (ImpactDelay > 0.f)
	{
		ScheduleNextStrike(ImpactDelay);
		return;
	}
	OnStrikeTimer();
}

void UDSGA_AreaStrike::ScheduleNextStrike(float Delay)
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimer(StrikeTimer, this, &UDSGA_AreaStrike::OnStrikeTimer, Delay, false);
	}
}

void UDSGA_AreaStrike::OnStrikeTimer()
{
	if (!IsActive())
	{
		return;
	}
	PerformStrike();
	++StrikesDone;
	if (StrikesDone < HitCount)
	{
		ScheduleNextStrike(HitInterval);
		return;
	}
	EndAbility(GetCurrentAbilitySpecHandle(), GetCurrentActorInfo(), GetCurrentActivationInfo(), true, false);
}

int32 UDSGA_AreaStrike::PerformStrike()
{
	TArray<ADSCharacterBase*> Targets;
	FindHostilesInArc(Radius, ArcDegrees, Targets);
	const int32 Hits = ApplyDamageToTargets(Targets, DamageMultiplier);

	UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo();
	if (ASC && Hits > 0 && BonusLifesteal > 0.f)
	{
		// Aproximação: cura proporcional ao dano bruto (sem mitigação) dos atingidos.
		const float Raw = GetOwnerAttribute(UDSAttributeSet::GetDamageAttribute()) * DamageMultiplier * Hits;
		ASC->ApplyModToAttribute(UDSAttributeSet::GetIncomingHealAttribute(), EGameplayModOp::Additive, Raw * BonusLifesteal);
	}
	if (ASC && ImpactCueTag.IsValid())
	{
		FGameplayCueParameters Cue;
		Cue.Location = GetDSCharacter() ? GetDSCharacter()->GetActorLocation() : FVector::ZeroVector;
		Cue.RawMagnitude = Radius;
		ASC->ExecuteGameplayCue(ImpactCueTag, Cue);
	}
	return Hits;
}

UDSGA_Whirlwind::UDSGA_Whirlwind(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	FGameplayTagContainer Tags;
	Tags.AddTag(DSTags::Ability_ClassSkill);
	Tags.AddTag(DSGameplayTags::Ability_ClassSkill_Whirlwind);
	SetAssetTags(Tags);
	Radius = 350.f;
	ArcDegrees = 360.f;
	DamageMultiplier = 2.2f;
	ResourceCost = 30.f;
	BaseCooldown = 6.f;
}
