#include "Gameplay/Abilities/DSGameplayAbility.h"

#include "AbilitySystemComponent.h"
#include "AbilitySystemGlobals.h"
#include "Gameplay/Abilities/DSAbilitySystemComponent.h"
#include "Gameplay/Attributes/DSAttributeSet.h"
#include "Gameplay/Characters/DSCharacterBase.h"
#include "Gameplay/Characters/DSHeroCharacter.h"
#include "Gameplay/Data/DSClassData.h"
#include "Gameplay/Effects/DSGameplayEffects.h"
#include "Core/DSGameplayTags.h"

UDSGameplayAbility::UDSGameplayAbility(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalPredicted;
	ReplicationPolicy = EGameplayAbilityReplicationPolicy::ReplicateNo;
	NetSecurityPolicy = EGameplayAbilityNetSecurityPolicy::ClientOrServer;

	ActivationBlockedTags.AddTag(DSTags::State_Dead);
	ActivationBlockedTags.AddTag(DSTags::State_Downed);
	ActivationBlockedTags.AddTag(DSTags::Status_Stun);

	CooldownGameplayEffectClass = UDSGE_Cooldown::StaticClass();
	CostGameplayEffectClass = UDSGE_Cost::StaticClass();
	DamageEffectClass = UDSGE_Damage::StaticClass();
}

// ---------------------------------------------------------------------------
// Recarga e custo
// ---------------------------------------------------------------------------

const FGameplayTagContainer* UDSGameplayAbility::GetCooldownTags() const
{
	FGameplayTagContainer* MutableTags = const_cast<FGameplayTagContainer*>(&TempCooldownTags);
	MutableTags->Reset();
	if (const FGameplayTagContainer* ParentTags = Super::GetCooldownTags())
	{
		MutableTags->AppendTags(*ParentTags);
	}
	MutableTags->AppendTags(GetAbilityCooldownTags());
	return MutableTags;
}

float UDSGameplayAbility::GetCooldownDuration() const
{
	if (BaseCooldown <= 0.f)
	{
		return 0.f;
	}
	const float Reduction = bUseCooldownReduction
		? FMath::Clamp(GetOwnerAttribute(UDSAttributeSet::GetCooldownReductionAttribute()), 0.f, MaxCooldownReduction)
		: 0.f;
	return BaseCooldown * (1.f - Reduction);
}

void UDSGameplayAbility::ApplyCooldown(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo) const
{
	const float Duration = GetCooldownDuration();
	const UGameplayEffect* CooldownGE = GetCooldownGameplayEffect();
	const FGameplayTagContainer AbilityCooldownTags = GetAbilityCooldownTags();
	if (!CooldownGE || Duration <= 0.f || AbilityCooldownTags.IsEmpty())
	{
		return;
	}
	const FGameplayEffectSpecHandle Spec = MakeOutgoingGameplayEffectSpec(Handle, ActorInfo, ActivationInfo, CooldownGE->GetClass(), GetAbilityLevel(Handle, ActorInfo));
	if (Spec.IsValid())
	{
		Spec.Data->DynamicGrantedTags.AppendTags(AbilityCooldownTags);
		Spec.Data->SetSetByCallerMagnitude(DSTags::Data_Duration, Duration);
		ApplyGameplayEffectSpecToOwner(Handle, ActorInfo, ActivationInfo, Spec);
	}
}

bool UDSGameplayAbility::CheckCost(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, FGameplayTagContainer* OptionalRelevantTags) const
{
	const float Cost = GetResourceCost();
	if (Cost <= 0.f)
	{
		return true;
	}
	const UAbilitySystemComponent* ASC = ActorInfo ? ActorInfo->AbilitySystemComponent.Get() : nullptr;
	const bool bCanPay = ASC && ASC->GetNumericAttribute(UDSAttributeSet::GetResourceAttribute()) >= Cost;
	if (!bCanPay && OptionalRelevantTags)
	{
		OptionalRelevantTags->AddTag(UAbilitySystemGlobals::Get().ActivateFailCostTag);
	}
	return bCanPay;
}

void UDSGameplayAbility::ApplyCost(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo) const
{
	const float Cost = GetResourceCost();
	const UGameplayEffect* CostGE = GetCostGameplayEffect();
	if (Cost <= 0.f || !CostGE)
	{
		return;
	}
	const FGameplayEffectSpecHandle Spec = MakeOutgoingGameplayEffectSpec(Handle, ActorInfo, ActivationInfo, CostGE->GetClass(), GetAbilityLevel(Handle, ActorInfo));
	if (Spec.IsValid())
	{
		Spec.Data->SetSetByCallerMagnitude(DSTags::Data_Magnitude, -Cost);
		ApplyGameplayEffectSpecToOwner(Handle, ActorInfo, ActivationInfo, Spec);
	}
}

void UDSGameplayAbility::OnGiveAbility(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpec& Spec)
{
	Super::OnGiveAbility(ActorInfo, Spec);
	// Passivas: ativam assim que concedidas (se o avatar já existe).
	if (ActivationPolicy == EDSAbilityActivationPolicy::OnSpawn && ActorInfo && ActorInfo->AvatarActor.IsValid() && !Spec.IsActive())
	{
		if (UAbilitySystemComponent* ASC = ActorInfo->AbilitySystemComponent.Get())
		{
			ASC->TryActivateAbility(Spec.Handle);
		}
	}
}

// ---------------------------------------------------------------------------
// Acesso
// ---------------------------------------------------------------------------

UDSAbilitySystemComponent* UDSGameplayAbility::GetDSAbilitySystemComponent() const
{
	return Cast<UDSAbilitySystemComponent>(GetAbilitySystemComponentFromActorInfo());
}

ADSCharacterBase* UDSGameplayAbility::GetDSCharacter() const
{
	return Cast<ADSCharacterBase>(GetAvatarActorFromActorInfo());
}

ADSHeroCharacter* UDSGameplayAbility::GetHero() const
{
	return Cast<ADSHeroCharacter>(GetAvatarActorFromActorInfo());
}

const UDSClassData* UDSGameplayAbility::GetClassData() const
{
	const ADSHeroCharacter* Hero = GetHero();
	return Hero ? Hero->GetClassData() : nullptr;
}

float UDSGameplayAbility::GetOwnerAttribute(const FGameplayAttribute& Attribute) const
{
	const UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo();
	return ASC ? ASC->GetNumericAttribute(Attribute) : 0.f;
}

bool UDSGameplayAbility::IsServer() const
{
	const FGameplayAbilityActorInfo* Info = GetCurrentActorInfo();
	return Info && Info->IsNetAuthority();
}

FVector UDSGameplayAbility::GetAimDirection() const
{
	const ADSCharacterBase* Character = GetDSCharacter();
	return Character ? Character->GetAimDirection() : FVector::ForwardVector;
}

// ---------------------------------------------------------------------------
// Alvos e dano
// ---------------------------------------------------------------------------

void UDSGameplayAbility::FindHostilesInArc(float Radius, float ArcDegrees, TArray<ADSCharacterBase*>& OutTargets) const
{
	const ADSCharacterBase* Character = GetDSCharacter();
	if (!Character)
	{
		OutTargets.Reset();
		return;
	}
	UDSTargetingLibrary::FindInArc(Character, Character->GetActorLocation(), GetAimDirection(), Radius, ArcDegrees, EDSTargetFilter::Hostile, OutTargets);
}

void UDSGameplayAbility::FindAlliesInRadius(const FVector& Center, float Radius, TArray<ADSCharacterBase*>& OutAllies, bool bIncludeDowned) const
{
	UDSTargetingLibrary::FindInRadius(GetDSCharacter(), Center, Radius, EDSTargetFilter::Ally, OutAllies, bIncludeDowned);
}

FGameplayEffectSpecHandle UDSGameplayAbility::MakeDamageSpec(float Multiplier, float FlatDamage) const
{
	if (!DamageEffectClass)
	{
		return FGameplayEffectSpecHandle();
	}
	FGameplayEffectSpecHandle Spec = MakeOutgoingGameplayEffectSpec(DamageEffectClass, GetAbilityLevel());
	if (Spec.IsValid())
	{
		float FinalMultiplier = Multiplier;
		// Bônus de recurso cheio (Fúria do Bárbaro: até +30%).
		if (const UDSClassData* ClassData = GetClassData())
		{
			const float MaxBonus = ClassData->Resource.FullResourceDamageBonus;
			const float MaxResource = GetOwnerAttribute(UDSAttributeSet::GetMaxResourceAttribute());
			if (MaxBonus > 0.f && MaxResource > 0.f)
			{
				FinalMultiplier *= 1.f + MaxBonus * FMath::Clamp(GetOwnerAttribute(UDSAttributeSet::GetResourceAttribute()) / MaxResource, 0.f, 1.f);
			}
		}
		Spec.Data->SetSetByCallerMagnitude(DSTags::Data_Magnitude, FinalMultiplier);
		Spec.Data->SetSetByCallerMagnitude(DSTags::Data_Damage, FlatDamage);
	}
	return Spec;
}

int32 UDSGameplayAbility::ApplyDamageToTargets(const TArray<ADSCharacterBase*>& Targets, float Multiplier, float FlatDamage) const
{
	if (!IsServer() || Targets.Num() == 0)
	{
		return 0;
	}
	UAbilitySystemComponent* SourceASC = GetAbilitySystemComponentFromActorInfo();
	const FGameplayEffectSpecHandle Spec = MakeDamageSpec(Multiplier, FlatDamage);
	if (!SourceASC || !Spec.IsValid())
	{
		return 0;
	}
	int32 Hits = 0;
	for (ADSCharacterBase* Target : Targets)
	{
		UAbilitySystemComponent* TargetASC = Target ? Target->GetAbilitySystemComponent() : nullptr;
		if (TargetASC)
		{
			SourceASC->ApplyGameplayEffectSpecToTarget(*Spec.Data.Get(), TargetASC);
			++Hits;
		}
	}
	GainResourceOnHit(Hits);
	return Hits;
}

void UDSGameplayAbility::GainResourceOnHit(int32 NumHits) const
{
	const UDSClassData* ClassData = GetClassData();
	UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo();
	if (NumHits <= 0 || !ClassData || !ASC || !IsServer() || ClassData->Resource.GainOnHit <= 0.f)
	{
		return;
	}
	// Protótipo: ganho por ataque que acerta (não por alvo).
	ASC->ApplyModToAttribute(UDSAttributeSet::GetResourceAttribute(), EGameplayModOp::Additive, ClassData->Resource.GainOnHit);
}

FActiveGameplayEffectHandle UDSGameplayAbility::ApplyTimedEffectToOwner(TSubclassOf<UGameplayEffect> EffectClass, float Duration) const
{
	if (!EffectClass || Duration <= 0.f)
	{
		return FActiveGameplayEffectHandle();
	}
	const FGameplayEffectSpecHandle Spec = MakeOutgoingGameplayEffectSpec(EffectClass, GetAbilityLevel());
	if (!Spec.IsValid())
	{
		return FActiveGameplayEffectHandle();
	}
	Spec.Data->SetSetByCallerMagnitude(DSTags::Data_Duration, Duration);
	return ApplyGameplayEffectSpecToOwner(GetCurrentAbilitySpecHandle(), GetCurrentActorInfo(), GetCurrentActivationInfo(), Spec);
}
