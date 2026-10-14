#include "Gameplay/Abilities/DSGA_Dodge.h"

#include "AbilitySystemComponent.h"
#include "Abilities/Tasks/AbilityTask_ApplyRootMotionConstantForce.h"
#include "Animation/AnimMontage.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Gameplay/Attributes/DSAttributeSet.h"
#include "Gameplay/Characters/DSCharacterBase.h"
#include "Gameplay/Data/DSClassData.h"
#include "Gameplay/DSGameplayTagsGameplay.h"
#include "Gameplay/Effects/DSGameplayEffects.h"
#include "Core/DSGameplayTags.h"

UDSGA_Dodge::UDSGA_Dodge(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	SetAssetTags(FGameplayTagContainer(DSTags::Ability_Dodge));
	CooldownTags.AddTag(DSTags::Cooldown_Dodge);
	ActivationOwnedTags.AddTag(DSGameplayTags::State_Dodging);
	BaseCooldown = 1.2f;
	bUseCooldownReduction = false;
}

float UDSGA_Dodge::GetCooldownDuration() const
{
	const UDSClassData* ClassData = GetClassData();
	return ClassData ? ClassData->BaseStats.DodgeCooldown : BaseCooldown;
}

FVector UDSGA_Dodge::ComputeDashDirection() const
{
	const ADSCharacterBase* Character = GetDSCharacter();
	if (!Character)
	{
		return FVector::ForwardVector;
	}
	// Aceleração atual: input local no cliente, último ServerMove no servidor (quase idênticos).
	FVector Direction = Character->GetCharacterMovement() ? Character->GetCharacterMovement()->GetCurrentAcceleration() : FVector::ZeroVector;
	Direction.Z = 0.f;
	if (Direction.IsNearlyZero())
	{
		Direction = GetAimDirection();
	}
	return Direction.GetSafeNormal();
}

void UDSGA_Dodge::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	const UDSClassData* ClassData = GetClassData();
	const float Distance = ClassData ? ClassData->BaseStats.DodgeDistance : DefaultDistance;
	const float Invulnerability = ClassData ? ClassData->BaseStats.DodgeInvulnerability : DefaultInvulnerability;
	const float Duration = FMath::Max(0.05f, DashDuration);

	// I-frames (GE predito: aplica no cliente local e confirma no servidor).
	ApplyTimedEffectToOwner(UDSGE_Invulnerable::StaticClass(), Invulnerability);

	if (DodgeMontage)
	{
		if (UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo())
		{
			ASC->PlayMontage(this, ActivationInfo, DodgeMontage, 1.f);
		}
	}

	const float ExitSpeed = GetOwnerAttribute(UDSAttributeSet::GetMoveSpeedAttribute());
	UAbilityTask_ApplyRootMotionConstantForce* Task = UAbilityTask_ApplyRootMotionConstantForce::ApplyRootMotionConstantForce(
		this, TEXT("DSDodge"), ComputeDashDirection(), Distance / Duration, Duration, /*bIsAdditive*/ false, /*StrengthOverTime*/ nullptr,
		ERootMotionFinishVelocityMode::ClampVelocity, FVector::ZeroVector, ExitSpeed, /*bEnableGravity*/ false);
	if (!Task)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
		return;
	}
	Task->OnFinish.AddDynamic(this, &UDSGA_Dodge::OnDashFinished);
	Task->ReadyForActivation();
}

void UDSGA_Dodge::OnDashFinished()
{
	EndAbility(GetCurrentAbilitySpecHandle(), GetCurrentActorInfo(), GetCurrentActivationInfo(), true, false);
}
