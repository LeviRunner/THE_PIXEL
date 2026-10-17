#include "Gameplay/Combat/DSTargetingLibrary.h"

#include "Components/CapsuleComponent.h"
#include "Engine/OverlapResult.h"
#include "Engine/World.h"
#include "Gameplay/Characters/DSCharacterBase.h"

namespace
{
	/** Folga extra de altura nas consultas (pulo de até 140 cm). */
	constexpr float QueryHalfHeight = 300.f;

	float Distance2D(const FVector& A, const FVector& B)
	{
		return FVector::Dist2D(A, B);
	}
}

float UDSTargetingLibrary::GetCollisionRadius(const ADSCharacterBase* Character)
{
	const UCapsuleComponent* Capsule = Character ? Character->GetCapsuleComponent() : nullptr;
	return Capsule ? Capsule->GetScaledCapsuleRadius() : 0.f;
}

bool UDSTargetingLibrary::PassesFilter(const ADSCharacterBase* Querier, const ADSCharacterBase* Candidate, EDSTargetFilter Filter)
{
	if (!Candidate)
	{
		return false;
	}
	switch (Filter)
	{
	case EDSTargetFilter::Hostile: return Querier && Querier->IsHostileTo(Candidate);
	case EDSTargetFilter::Ally:    return Querier && Candidate->IsHeroTeam() == Querier->IsHeroTeam();
	default:                       return true;
	}
}

void UDSTargetingLibrary::FindInRadius(const ADSCharacterBase* Querier, const FVector& Center, float Radius, EDSTargetFilter Filter,
	TArray<ADSCharacterBase*>& OutTargets, bool bIncludeDowned)
{
	OutTargets.Reset();
	const UWorld* World = Querier ? Querier->GetWorld() : nullptr;
	if (!World || Radius <= 0.f)
	{
		return;
	}

	// Cápsula vertical larga o bastante para pegar o raio + cápsulas vizinhas.
	const float SearchRadius = Radius + 150.f;
	TArray<FOverlapResult> Overlaps;
	FCollisionObjectQueryParams ObjectParams(ECC_Pawn);
	FCollisionQueryParams Params(SCENE_QUERY_STAT(DSFindInRadius), false);
	World->OverlapMultiByObjectType(Overlaps, Center, FQuat::Identity, ObjectParams,
		FCollisionShape::MakeCapsule(SearchRadius, QueryHalfHeight + SearchRadius), Params);

	for (const FOverlapResult& Overlap : Overlaps)
	{
		ADSCharacterBase* Candidate = Cast<ADSCharacterBase>(Overlap.GetActor());
		if (!Candidate || OutTargets.Contains(Candidate) || !PassesFilter(Querier, Candidate, Filter))
		{
			continue;
		}
		const bool bUsable = Candidate->IsAlive() || (bIncludeDowned && Candidate->IsHeroTeam());
		if (bUsable && Distance2D(Candidate->GetActorLocation(), Center) <= Radius + GetCollisionRadius(Candidate))
		{
			OutTargets.Add(Candidate);
		}
	}
}

void UDSTargetingLibrary::FindInArc(const ADSCharacterBase* Querier, const FVector& Origin, const FVector& Forward, float Radius, float ArcDegrees,
	EDSTargetFilter Filter, TArray<ADSCharacterBase*>& OutTargets)
{
	TArray<ADSCharacterBase*> InRadius;
	FindInRadius(Querier, Origin, Radius, Filter, InRadius);

	const FVector Forward2D = FVector(Forward.X, Forward.Y, 0.f).GetSafeNormal();
	const float HalfArcCos = FMath::Cos(FMath::DegreesToRadians(FMath::Clamp(ArcDegrees, 0.f, 360.f) * 0.5f));
	OutTargets.Reset();
	for (ADSCharacterBase* Candidate : InRadius)
	{
		const FVector ToTarget = FVector(Candidate->GetActorLocation() - Origin) * FVector(1.f, 1.f, 0.f);
		const bool bOverlapsOrigin = ToTarget.Size() <= GetCollisionRadius(Candidate);
		if (ArcDegrees >= 360.f || bOverlapsOrigin || FVector::DotProduct(ToTarget.GetSafeNormal(), Forward2D) >= HalfArcCos)
		{
			OutTargets.Add(Candidate);
		}
	}
	OutTargets.Sort([&Origin](const ADSCharacterBase& A, const ADSCharacterBase& B)
	{
		return FVector::DistSquared2D(A.GetActorLocation(), Origin) < FVector::DistSquared2D(B.GetActorLocation(), Origin);
	});
}

void UDSTargetingLibrary::FindInLine(const ADSCharacterBase* Querier, const FVector& Origin, const FVector& Forward, float Length, float HalfWidth,
	EDSTargetFilter Filter, TArray<ADSCharacterBase*>& OutTargets)
{
	const FVector Forward2D = FVector(Forward.X, Forward.Y, 0.f).GetSafeNormal();
	TArray<ADSCharacterBase*> InRadius;
	FindInRadius(Querier, Origin + Forward2D * Length * 0.5f, Length * 0.5f + HalfWidth, Filter, InRadius);

	OutTargets.Reset();
	for (ADSCharacterBase* Candidate : InRadius)
	{
		const FVector ToTarget = (Candidate->GetActorLocation() - Origin) * FVector(1.f, 1.f, 0.f);
		const float Along = FVector::DotProduct(ToTarget, Forward2D);
		const float Across = FMath::Abs(FVector::CrossProduct(Forward2D, ToTarget).Z);
		const float Slack = GetCollisionRadius(Candidate);
		if (Along >= -Slack && Along <= Length + Slack && Across <= HalfWidth + Slack)
		{
			OutTargets.Add(Candidate);
		}
	}
}

ADSCharacterBase* UDSTargetingLibrary::FindNearest(const ADSCharacterBase* Querier, const FVector& Center, float Radius, EDSTargetFilter Filter)
{
	TArray<ADSCharacterBase*> Targets;
	FindInRadius(Querier, Center, Radius, Filter, Targets);
	ADSCharacterBase* Best = nullptr;
	float BestDistSq = TNumericLimits<float>::Max();
	for (ADSCharacterBase* Candidate : Targets)
	{
		const float DistSq = FVector::DistSquared2D(Candidate->GetActorLocation(), Center);
		if (Candidate != Querier && DistSq < BestDistSq)
		{
			BestDistSq = DistSq;
			Best = Candidate;
		}
	}
	return Best;
}
