#include "Gameplay/Abilities/DSGA_Sprint.h"

#include "AbilitySystemComponent.h"
#include "Gameplay/Attributes/DSAttributeSet.h"
#include "Gameplay/DSGameplayTagsGameplay.h"
#include "Gameplay/Effects/DSGameplayEffects.h"
#include "Core/DSGameplayTags.h"

UDSGA_Sprint::UDSGA_Sprint(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	SetAssetTags(FGameplayTagContainer(DSGameplayTags::Ability_Sprint));
	ActivationOwnedTags.AddTag(DSTags::State_Sprinting);
	DrainEffectClass = UDSGE_SprintDrain::StaticClass();
}

bool UDSGA_Sprint::CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayTagContainer* SourceTags, const FGameplayTagContainer* TargetTags, FGameplayTagContainer* OptionalRelevantTags) const
{
	if (!Super::CanActivateAbility(Handle, ActorInfo, SourceTags, TargetTags, OptionalRelevantTags))
	{
		return false;
	}
	const UAbilitySystemComponent* ASC = ActorInfo ? ActorInfo->AbilitySystemComponent.Get() : nullptr;
	return ASC && ASC->GetNumericAttribute(UDSAttributeSet::GetStaminaAttribute()) >= MinStaminaToStart;
}

void UDSGA_Sprint::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}
	UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo();
	if (IsServer() && ASC && DrainEffectClass)
	{
		const FGameplayEffectSpecHandle Spec = MakeOutgoingGameplayEffectSpec(DrainEffectClass, GetAbilityLevel());
		if (Spec.IsValid())
		{
			DrainHandle = ApplyGameplayEffectSpecToOwner(Handle, ActorInfo, ActivationInfo, Spec);
		}
	}
	if (ASC)
	{
		StaminaChangedHandle = ASC->GetGameplayAttributeValueChangeDelegate(UDSAttributeSet::GetStaminaAttribute()).AddUObject(this, &UDSGA_Sprint::OnStaminaChanged);
	}
}

void UDSGA_Sprint::OnStaminaChanged(const FOnAttributeChangeData& Data)
{
	if (Data.NewValue <= 0.f && IsActive())
	{
		EndAbility(GetCurrentAbilitySpecHandle(), GetCurrentActorInfo(), GetCurrentActivationInfo(), true, false);
	}
}

void UDSGA_Sprint::InputReleased(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo)
{
	Super::InputReleased(Handle, ActorInfo, ActivationInfo);
	if (IsActive())
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
	}
}

void UDSGA_Sprint::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	if (UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo())
	{
		ASC->GetGameplayAttributeValueChangeDelegate(UDSAttributeSet::GetStaminaAttribute()).Remove(StaminaChangedHandle);
		if (DrainHandle.IsValid() && ASC->IsOwnerActorAuthoritative())
		{
			ASC->RemoveActiveGameplayEffect(DrainHandle);
		}
	}
	DrainHandle.Invalidate();
	StaminaChangedHandle.Reset();
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}
