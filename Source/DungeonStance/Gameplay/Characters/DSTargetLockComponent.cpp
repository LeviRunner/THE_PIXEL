#include "Gameplay/Characters/DSTargetLockComponent.h"

#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Controller.h"
#include "Net/UnrealNetwork.h"
#include "Gameplay/Characters/DSCharacterBase.h"
#include "Gameplay/Combat/DSTargetingLibrary.h"

UDSTargetLockComponent::UDSTargetLockComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.TickGroup = TG_PostPhysics;
	SetIsReplicatedByDefault(true);
}

void UDSTargetLockComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	// O dono escolhe localmente (sem esperar ida e volta); servidor valida e replica para os demais.
	DOREPLIFETIME_CONDITION(UDSTargetLockComponent, LockedTarget, COND_SkipOwner);
}

ADSCharacterBase* UDSTargetLockComponent::GetOwnerCharacter() const
{
	return Cast<ADSCharacterBase>(GetOwner());
}

FVector UDSTargetLockComponent::GetViewForward() const
{
	const ADSCharacterBase* Owner = GetOwnerCharacter();
	const AController* Controller = Owner ? Owner->GetController() : nullptr;
	FVector Forward = Controller ? Controller->GetControlRotation().Vector() : (Owner ? Owner->GetActorForwardVector() : FVector::ForwardVector);
	Forward.Z = 0.f;
	return Forward.GetSafeNormal();
}

bool UDSTargetLockComponent::IsTargetValid(const ADSCharacterBase* Target) const
{
	const ADSCharacterBase* Owner = GetOwnerCharacter();
	return Owner && Target && IsValid(Target) && Target->IsAlive() && Owner->IsHostileTo(Target)
		&& FVector::Dist2D(Owner->GetActorLocation(), Target->GetActorLocation()) <= BreakRange;
}

void UDSTargetLockComponent::GatherCandidates(TArray<ADSCharacterBase*>& OutCandidates) const
{
	const ADSCharacterBase* Owner = GetOwnerCharacter();
	if (Owner)
	{
		UDSTargetingLibrary::FindInRadius(Owner, Owner->GetActorLocation(), LockRange, EDSTargetFilter::Hostile, OutCandidates);
	}
}

ADSCharacterBase* UDSTargetLockComponent::FindBestTarget() const
{
	const ADSCharacterBase* Owner = GetOwnerCharacter();
	TArray<ADSCharacterBase*> Candidates;
	GatherCandidates(Candidates);
	if (!Owner || Candidates.Num() == 0)
	{
		return nullptr;
	}
	const FVector Forward = GetViewForward();
	ADSCharacterBase* Best = nullptr;
	float BestScore = TNumericLimits<float>::Max();
	for (ADSCharacterBase* Candidate : Candidates)
	{
		const FVector ToTarget = (Candidate->GetActorLocation() - Owner->GetActorLocation()) * FVector(1.f, 1.f, 0.f);
		const float Angle = FMath::RadiansToDegrees(FMath::Acos(FMath::Clamp(FVector::DotProduct(Forward, ToTarget.GetSafeNormal()), -1.f, 1.f)));
		if (Angle > MaxLockAngle)
		{
			continue;
		}
		const float Score = Angle + DistanceWeight * (ToTarget.Size() / FMath::Max(1.f, LockRange));
		if (Score < BestScore)
		{
			BestScore = Score;
			Best = Candidate;
		}
	}
	return Best;
}

void UDSTargetLockComponent::ToggleLock()
{
	SetLockedTargetLocal(IsLocked() ? nullptr : FindBestTarget());
}

void UDSTargetLockComponent::ClearLock()
{
	SetLockedTargetLocal(nullptr);
}

void UDSTargetLockComponent::SwitchTarget(float Direction)
{
	const ADSCharacterBase* Owner = GetOwnerCharacter();
	if (!Owner || FMath::IsNearlyZero(Direction))
	{
		return;
	}
	if (!IsLocked())
	{
		ToggleLock();
		return;
	}
	const FVector Origin = Owner->GetActorLocation();
	const float CurrentYaw = (LockedTarget->GetActorLocation() - Origin).Rotation().Yaw;

	TArray<ADSCharacterBase*> Candidates;
	GatherCandidates(Candidates);
	ADSCharacterBase* Best = nullptr;
	float BestDelta = TNumericLimits<float>::Max();
	for (ADSCharacterBase* Candidate : Candidates)
	{
		if (Candidate == LockedTarget)
		{
			continue;
		}
		// Delta de yaw com sinal: positivo = à direita do alvo atual.
		const float Delta = FMath::FindDeltaAngleDegrees(CurrentYaw, (Candidate->GetActorLocation() - Origin).Rotation().Yaw);
		const float Signed = Direction > 0.f ? Delta : -Delta;
		if (Signed > 0.f && Signed < BestDelta)
		{
			BestDelta = Signed;
			Best = Candidate;
		}
	}
	if (Best)
	{
		SetLockedTargetLocal(Best);
	}
}

void UDSTargetLockComponent::SetLockedTargetLocal(ADSCharacterBase* NewTarget)
{
	if (LockedTarget == NewTarget)
	{
		return;
	}
	LockedTarget = NewTarget;
	OnRep_LockedTarget();
	if (!GetOwner()->HasAuthority())
	{
		Server_SetLockedTarget(NewTarget);
	}
}

void UDSTargetLockComponent::Server_SetLockedTarget_Implementation(ADSCharacterBase* NewTarget)
{
	// Servidor valida: só alvo hostil, vivo e ao alcance.
	ADSCharacterBase* Validated = (NewTarget && IsTargetValid(NewTarget)) ? NewTarget : nullptr;
	if (LockedTarget != Validated)
	{
		LockedTarget = Validated;
		OnRep_LockedTarget();
	}
}

void UDSTargetLockComponent::OnRep_LockedTarget()
{
	UpdateRotationMode();
	OnTargetLockChanged.Broadcast(LockedTarget);
}

void UDSTargetLockComponent::UpdateRotationMode() const
{
	const ADSCharacterBase* Owner = GetOwnerCharacter();
	if (UCharacterMovementComponent* Movement = Owner ? Owner->GetCharacterMovement() : nullptr)
	{
		// Travado: o corpo encara a mira (controle); livre: encara o movimento.
		Movement->bOrientRotationToMovement = !IsLocked();
		Movement->bUseControllerDesiredRotation = IsLocked();
	}
}

void UDSTargetLockComponent::RotateCameraTowardTarget(float DeltaTime) const
{
	const ADSCharacterBase* Owner = GetOwnerCharacter();
	AController* Controller = Owner ? Owner->GetController() : nullptr;
	if (!Controller || !LockedTarget)
	{
		return;
	}
	const FVector ToTarget = LockedTarget->GetActorLocation() - Owner->GetActorLocation();
	const FRotator Desired(LockedCameraPitch, ToTarget.Rotation().Yaw, 0.f);
	Controller->SetControlRotation(FMath::RInterpTo(Controller->GetControlRotation(), Desired, DeltaTime, CameraInterpSpeed));
}

void UDSTargetLockComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	if (!LockedTarget)
	{
		return;
	}
	const ADSCharacterBase* Owner = GetOwnerCharacter();
	const bool bLocal = Owner && Owner->IsLocallyControlled();
	const bool bAuthority = GetOwner()->HasAuthority();
	if (!IsTargetValid(LockedTarget))
	{
		// Alvo morreu/saiu do alcance: dono local (ou servidor) solta.
		if (bLocal || bAuthority)
		{
			SetLockedTargetLocal(nullptr);
		}
		return;
	}
	if (bLocal)
	{
		RotateCameraTowardTarget(DeltaTime);
	}
}
