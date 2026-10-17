#include "Gameplay/Abilities/DSAbilitySystemComponent.h"

#include "Gameplay/Abilities/DSGameplayAbility.h"
#include "Core/DSGameplayTags.h"
#include "Core/DSTypes.h"

UDSAbilitySystemComponent::UDSAbilitySystemComponent()
{
	SetIsReplicatedByDefault(true);
}

void UDSAbilitySystemComponent::InitAbilityActorInfo(AActor* InOwnerActor, AActor* InAvatarActor)
{
	const bool bAvatarChanged = AbilityActorInfo.IsValid() && AbilityActorInfo->AvatarActor != InAvatarActor;
	Super::InitAbilityActorInfo(InOwnerActor, InAvatarActor);
	if (bAvatarChanged || InAvatarActor)
	{
		TryActivateAbilitiesOnSpawn();
	}
}

// ---------------------------------------------------------------------------
// Input
// ---------------------------------------------------------------------------

void UDSAbilitySystemComponent::CollectSpecsForInputTag(FGameplayTag InputTag, TArray<FGameplayAbilitySpecHandle>& OutHandles) const
{
	if (!InputTag.IsValid())
	{
		return;
	}
	for (const FGameplayAbilitySpec& Spec : ActivatableAbilities.Items)
	{
		if (Spec.Ability && Spec.GetDynamicSpecSourceTags().HasTagExact(InputTag))
		{
			OutHandles.AddUnique(Spec.Handle);
		}
	}
}

void UDSAbilitySystemComponent::AbilityInputTagPressed(FGameplayTag InputTag)
{
	TArray<FGameplayAbilitySpecHandle> Handles;
	CollectSpecsForInputTag(InputTag, Handles);
	for (const FGameplayAbilitySpecHandle& Handle : Handles)
	{
		InputPressedSpecHandles.AddUnique(Handle);
		InputHeldSpecHandles.AddUnique(Handle);
	}
}

void UDSAbilitySystemComponent::AbilityInputTagReleased(FGameplayTag InputTag)
{
	TArray<FGameplayAbilitySpecHandle> Handles;
	CollectSpecsForInputTag(InputTag, Handles);
	for (const FGameplayAbilitySpecHandle& Handle : Handles)
	{
		InputReleasedSpecHandles.AddUnique(Handle);
		InputHeldSpecHandles.Remove(Handle);
	}
}

void UDSAbilitySystemComponent::ActivatePressedAndHeld(TArray<FGameplayAbilitySpecHandle>& ToActivate)
{
	// Segurando: habilidades WhileInputActive tentam ativar a cada frame (ataque básico contínuo).
	for (const FGameplayAbilitySpecHandle& Handle : InputHeldSpecHandles)
	{
		const FGameplayAbilitySpec* Spec = FindAbilitySpecFromHandle(Handle);
		const UDSGameplayAbility* Ability = Spec ? Cast<UDSGameplayAbility>(Spec->Ability) : nullptr;
		if (Ability && !Spec->IsActive() && Ability->GetActivationPolicy() == EDSAbilityActivationPolicy::WhileInputActive)
		{
			ToActivate.AddUnique(Handle);
		}
	}

	// Pressionado neste frame.
	for (const FGameplayAbilitySpecHandle& Handle : InputPressedSpecHandles)
	{
		FGameplayAbilitySpec* Spec = FindAbilitySpecFromHandle(Handle);
		if (!Spec || !Spec->Ability)
		{
			continue;
		}
		Spec->InputPressed = true;
		if (Spec->IsActive())
		{
			AbilitySpecInputPressed(*Spec);
			continue;
		}
		const UDSGameplayAbility* Ability = Cast<UDSGameplayAbility>(Spec->Ability);
		if (Ability && Ability->GetActivationPolicy() == EDSAbilityActivationPolicy::OnInputTriggered)
		{
			ToActivate.AddUnique(Handle);
		}
	}
}

void UDSAbilitySystemComponent::ProcessAbilityInput(float DeltaTime, bool bGamePaused)
{
	if (bGamePaused || HasMatchingGameplayTag(DSTags::State_Dead) || HasMatchingGameplayTag(DSTags::State_Downed))
	{
		ClearAbilityInput();
		return;
	}

	TArray<FGameplayAbilitySpecHandle> ToActivate;
	ActivatePressedAndHeld(ToActivate);
	for (const FGameplayAbilitySpecHandle& Handle : ToActivate)
	{
		TryActivateAbility(Handle);
	}

	for (const FGameplayAbilitySpecHandle& Handle : InputReleasedSpecHandles)
	{
		FGameplayAbilitySpec* Spec = FindAbilitySpecFromHandle(Handle);
		if (!Spec || !Spec->Ability)
		{
			continue;
		}
		Spec->InputPressed = false;
		if (Spec->IsActive())
		{
			AbilitySpecInputReleased(*Spec);
		}
	}

	InputPressedSpecHandles.Reset();
	InputReleasedSpecHandles.Reset();
}

void UDSAbilitySystemComponent::ClearAbilityInput()
{
	InputPressedSpecHandles.Reset();
	InputReleasedSpecHandles.Reset();
	InputHeldSpecHandles.Reset();
}

void UDSAbilitySystemComponent::AbilitySpecInputPressed(FGameplayAbilitySpec& Spec)
{
	Super::AbilitySpecInputPressed(Spec);
	if (!Spec.IsActive())
	{
		return;
	}
	// Replica o evento de input para que WaitInputPress funcione no servidor.
	if (const UGameplayAbility* Instance = Spec.GetPrimaryInstance())
	{
		InvokeReplicatedEvent(EAbilityGenericReplicatedEvent::InputPressed, Spec.Handle,
			Instance->GetCurrentActivationInfo().GetActivationPredictionKey());
	}
}

void UDSAbilitySystemComponent::AbilitySpecInputReleased(FGameplayAbilitySpec& Spec)
{
	Super::AbilitySpecInputReleased(Spec);
	if (!Spec.IsActive())
	{
		return;
	}
	if (const UGameplayAbility* Instance = Spec.GetPrimaryInstance())
	{
		InvokeReplicatedEvent(EAbilityGenericReplicatedEvent::InputReleased, Spec.Handle,
			Instance->GetCurrentActivationInfo().GetActivationPredictionKey());
	}
}

// ---------------------------------------------------------------------------
// Concessão
// ---------------------------------------------------------------------------

void UDSAbilitySystemComponent::GrantAbilitySet(const UDSAbilitySet* AbilitySet, UObject* SourceObject)
{
	if (!AbilitySet || !IsOwnerActorAuthoritative() || GrantedSets.Contains(AbilitySet))
	{
		return;
	}
	FDSAbilitySetGrantedHandles& Handles = GrantedSets.Add(AbilitySet);
	AbilitySet->GiveToAbilitySystem(this, &Handles, SourceObject);
}

void UDSAbilitySystemComponent::RemoveAbilitySet(const UDSAbilitySet* AbilitySet)
{
	FDSAbilitySetGrantedHandles Handles;
	if (AbilitySet && IsOwnerActorAuthoritative() && GrantedSets.RemoveAndCopyValue(AbilitySet, Handles))
	{
		Handles.TakeFromAbilitySystem(this);
	}
}

bool UDSAbilitySystemComponent::HasAbilitySet(const UDSAbilitySet* AbilitySet) const
{
	return AbilitySet && GrantedSets.Contains(AbilitySet);
}

FGameplayAbilitySpecHandle UDSAbilitySystemComponent::GrantAbilityWithInputTag(TSubclassOf<UGameplayAbility> AbilityClass, FGameplayTag InputTag, int32 Level, UObject* SourceObject)
{
	if (!AbilityClass || !IsOwnerActorAuthoritative())
	{
		return FGameplayAbilitySpecHandle();
	}
	FGameplayAbilitySpec Spec(AbilityClass, Level, INDEX_NONE, SourceObject);
	if (InputTag.IsValid())
	{
		Spec.GetDynamicSpecSourceTags().AddTag(InputTag);
	}
	return GiveAbility(Spec);
}

FGameplayTag UDSAbilitySystemComponent::FindFreeSupportInputTag() const
{
	const FGameplayTag SlotTags[] = { DSTags::Input_Support1, DSTags::Input_Support2, DSTags::Input_Support3, DSTags::Input_Support4 };
	static_assert(UE_ARRAY_COUNT(SlotTags) == DSConstants::MaxSupportAbilitySlots, "Slots de apoio fora de sincronia com DSConstants");
	for (const FGameplayTag& Tag : SlotTags)
	{
		TArray<FGameplayAbilitySpecHandle> Handles;
		CollectSpecsForInputTag(Tag, Handles);
		if (Handles.Num() == 0)
		{
			return Tag;
		}
	}
	return FGameplayTag();
}

void UDSAbilitySystemComponent::ResetCooldownsWithTags(const FGameplayTagContainer& CooldownTags)
{
	if (CooldownTags.Num() > 0 && IsOwnerActorAuthoritative())
	{
		RemoveActiveEffectsWithGrantedTags(CooldownTags);
	}
}

void UDSAbilitySystemComponent::SetReplicatedLooseTag(FGameplayTag Tag, bool bPresent)
{
	if (!Tag.IsValid() || !IsOwnerActorAuthoritative())
	{
		return;
	}
	// Contagens absolutas: idempotente e tolera outras áreas mexendo só na tag solta local.
	const int32 Count = bPresent ? 1 : 0;
	SetLooseGameplayTagCount(Tag, Count);
	SetReplicatedLooseGameplayTagCount(Tag, Count);
}

void UDSAbilitySystemComponent::TryActivateAbilitiesOnSpawn()
{
	ABILITYLIST_SCOPE_LOCK();
	for (const FGameplayAbilitySpec& Spec : ActivatableAbilities.Items)
	{
		const UDSGameplayAbility* Ability = Cast<UDSGameplayAbility>(Spec.Ability);
		if (Ability && !Spec.IsActive() && Ability->GetActivationPolicy() == EDSAbilityActivationPolicy::OnSpawn)
		{
			TryActivateAbility(Spec.Handle);
		}
	}
}
