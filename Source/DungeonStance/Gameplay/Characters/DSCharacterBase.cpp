#include "Gameplay/Characters/DSCharacterBase.h"

#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Controller.h"
#include "Gameplay/Abilities/DSAbilitySystemComponent.h"
#include "Gameplay/Attributes/DSAttributeSet.h"
#include "Gameplay/Effects/DSStatusLibrary.h"
#include "Core/DSGameplayTags.h"

ADSCharacterBase::ADSCharacterBase(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;
	SetNetUpdateFrequency(30.f);
}

UAbilitySystemComponent* ADSCharacterBase::GetAbilitySystemComponent() const
{
	return AbilitySystemComponent;
}

bool ADSCharacterBase::IsAlive() const
{
	if (bDied)
	{
		return false;
	}
	if (!AbilitySystemComponent)
	{
		return true;
	}
	return !AbilitySystemComponent->HasMatchingGameplayTag(DSTags::State_Dead)
		&& !AbilitySystemComponent->HasMatchingGameplayTag(DSTags::State_Downed);
}

bool ADSCharacterBase::IsHostileTo(const ADSCharacterBase* Other) const
{
	return Other && Other != this && Other->IsHeroTeam() != IsHeroTeam();
}

FVector ADSCharacterBase::GetAimDirection() const
{
	FVector Direction = GetActorForwardVector();
	if (const AController* MyController = GetController())
	{
		Direction = MyController->GetControlRotation().Vector();
	}
	Direction.Z = 0.f;
	return Direction.GetSafeNormal(UE_SMALL_NUMBER, GetActorForwardVector());
}

FVector ADSCharacterBase::GetAimPoint() const
{
	return GetActorLocation();
}

void ADSCharacterBase::OnAbilitySystemInitialized()
{
	BindAttributeCallbacks();
	RefreshMovementSpeed();
}

void ADSCharacterBase::BindAttributeCallbacks()
{
	if (!AbilitySystemComponent || !AttributeSet)
	{
		return;
	}
	AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(UDSAttributeSet::GetMoveSpeedAttribute()).RemoveAll(this);
	AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(UDSAttributeSet::GetMoveSpeedAttribute()).AddUObject(this, &ADSCharacterBase::OnMoveSpeedChanged);

	const FGameplayTag SpeedTags[] = { DSTags::Status_Slow, DSTags::Status_Stun, DSTags::State_Sprinting };
	for (const FGameplayTag& Tag : SpeedTags)
	{
		AbilitySystemComponent->RegisterGameplayTagEvent(Tag, EGameplayTagEventType::NewOrRemoved).RemoveAll(this);
		AbilitySystemComponent->RegisterGameplayTagEvent(Tag, EGameplayTagEventType::NewOrRemoved).AddUObject(this, &ADSCharacterBase::OnSpeedTagChanged);
	}

	AttributeSet->OnOutOfHealth.Remove(OutOfHealthHandle);
	AttributeSet->OnDamageTaken.Remove(DamageTakenHandle);
	OutOfHealthHandle = AttributeSet->OnOutOfHealth.AddUObject(this, &ADSCharacterBase::HandleOutOfHealth);
	DamageTakenHandle = AttributeSet->OnDamageTaken.AddUObject(this, &ADSCharacterBase::HandleDamageTaken);
}

void ADSCharacterBase::OnMoveSpeedChanged(const FOnAttributeChangeData& Data)
{
	RefreshMovementSpeed();
}

void ADSCharacterBase::OnSpeedTagChanged(const FGameplayTag Tag, int32 NewCount)
{
	RefreshMovementSpeed();
}

void ADSCharacterBase::RefreshMovementSpeed()
{
	UCharacterMovementComponent* Movement = GetCharacterMovement();
	if (!Movement || !AttributeSet)
	{
		return;
	}
	const float StatusMultiplier = UDSStatusLibrary::GetSpeedMultiplier(AbilitySystemComponent);
	Movement->MaxWalkSpeed = AttributeSet->GetMoveSpeed() * StatusMultiplier * GetExtraSpeedMultiplier();
}

AController* ADSCharacterBase::ResolveKillerController(AActor* DamageInstigator)
{
	if (AController* AsController = Cast<AController>(DamageInstigator))
	{
		return AsController;
	}
	if (const APawn* AsPawn = Cast<APawn>(DamageInstigator))
	{
		return AsPawn->GetController();
	}
	// Heróis: o instigador original é o PlayerState (dono do ASC).
	if (AActor* Owner = DamageInstigator ? DamageInstigator->GetOwner() : nullptr)
	{
		return Cast<AController>(Owner);
	}
	return nullptr;
}

void ADSCharacterBase::HandleOutOfHealth(AActor* DamageInstigator, AActor* DamageCauser, const FGameplayEffectSpec* Spec, float Magnitude)
{
	if (HasAuthority())
	{
		Die(ResolveKillerController(DamageInstigator));
	}
}

void ADSCharacterBase::Die(AController* Killer)
{
	if (bDied || !HasAuthority())
	{
		return;
	}
	bDied = true;
	if (AbilitySystemComponent)
	{
		AbilitySystemComponent->SetReplicatedLooseTag(DSTags::State_Dead, true);
		AbilitySystemComponent->CancelAllAbilities();
		AbilitySystemComponent->ClearAbilityInput();

		FGameplayEventData Payload;
		Payload.EventTag = DSTags::Event_Death;
		Payload.Target = this;
		Payload.Instigator = Killer ? Killer->GetPawn() : nullptr;
		AbilitySystemComponent->HandleGameplayEvent(Payload.EventTag, &Payload);
	}
	Multicast_OnDied();
	OnDeath.Broadcast(this, Killer);
	if (CorpseLifeSpan > 0.f)
	{
		SetLifeSpan(CorpseLifeSpan);
	}
}

void ADSCharacterBase::Multicast_OnDied_Implementation()
{
	bDied = true;
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	if (UCharacterMovementComponent* Movement = GetCharacterMovement())
	{
		Movement->StopMovementImmediately();
		Movement->DisableMovement();
	}
	K2_OnDied();
}
