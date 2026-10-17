#include "Gameplay/AI/DSEnemyAIController.h"

#include "AbilitySystemComponent.h"
#include "Components/CapsuleComponent.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "NavigationSystem.h"
#include "Navigation/PathFollowingComponent.h"
#include "Gameplay/Characters/DSEnemyCharacter.h"
#include "Gameplay/Characters/DSHeroCharacter.h"
#include "Gameplay/Combat/DSSupportZone.h"
#include "Gameplay/Combat/DSTargetingLibrary.h"
#include "Gameplay/Effects/DSStatusLibrary.h"

ADSEnemyAIController::ADSEnemyAIController()
{
	PrimaryActorTick.bCanEverTick = true;
	bWantsPlayerState = false;
	bSetControlRotationFromPawnOrientation = true;
}

void ADSEnemyAIController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);
	Enemy = Cast<ADSEnemyCharacter>(InPawn);
	SwarmSide = FMath::RandBool() ? 1.f : -1.f;
	ThinkAccumulator = FMath::FRandRange(0.f, ThinkInterval); // espalha as decisões entre frames
	bBlinked = false;
	BroodThresholdsUsed = 0;
}

void ADSEnemyAIController::OnUnPossess()
{
	StopMovement();
	Enemy.Reset();
	CurrentTarget.Reset();
	Super::OnUnPossess();
}

void ADSEnemyAIController::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	if (!HasAuthority())
	{
		return;
	}
	ThinkAccumulator += DeltaSeconds;
	if (ThinkAccumulator >= ThinkInterval)
	{
		ThinkAccumulator = 0.f;
		Think();
	}
}

// ---------------------------------------------------------------------------
// Alvo
// ---------------------------------------------------------------------------

ADSCharacterBase* ADSEnemyAIController::FindNearestHero() const
{
	const ADSEnemyCharacter* Self = Enemy.Get();
	ADSCharacterBase* Best = nullptr;
	float BestDistSq = TNumericLimits<float>::Max();
	for (TActorIterator<ADSHeroCharacter> It(GetWorld()); It; ++It)
	{
		ADSHeroCharacter* Hero = *It;
		if (!Hero->IsAlive())
		{
			continue;
		}
		const float DistSq = FVector::DistSquared2D(Hero->GetActorLocation(), Self->GetActorLocation());
		if (DistSq < BestDistSq)
		{
			BestDistSq = DistSq;
			Best = Hero;
		}
	}
	return Best;
}

ADSCharacterBase* ADSEnemyAIController::SelectTarget() const
{
	const ADSEnemyCharacter* Self = Enemy.Get();
	// 1) Provocação (status.ts tauntTarget).
	if (ADSCharacterBase* Taunter = Cast<ADSCharacterBase>(UDSStatusLibrary::GetTauntInstigator(Self->GetAbilitySystemComponent())))
	{
		if (Taunter->IsAlive())
		{
			return Taunter;
		}
	}
	// 2) Totem Guardião: inimigos dentro da área preferem o dono.
	if (const ADSSupportZone* Zone = ADSSupportZone::FindTauntZoneAt(GetWorld(), Self->GetActorLocation()))
	{
		if (Zone->GetCaster() && Zone->GetCaster()->IsAlive())
		{
			return Zone->GetCaster();
		}
	}
	// 3) Herói vivo mais próximo (nearestTarget).
	return FindNearestHero();
}

// ---------------------------------------------------------------------------
// Decisão
// ---------------------------------------------------------------------------

void ADSEnemyAIController::Think()
{
	ADSEnemyCharacter* Self = Enemy.Get();
	if (!Self || !Self->IsAlive() || !Self->GetEnemyData())
	{
		return;
	}
	if (Self->IsStunned())
	{
		StopMovement();
		return;
	}
	ADSCharacterBase* Target = SelectTarget();
	CurrentTarget = Target;
	const UDSEnemyData* Data = Self->GetEnemyData();
	const FDSEnemyAITuning& Tuning = Data->AITuning;

	if (Data->Behavior == EDSEnemyBehavior::Passive)
	{
		ThinkPassive(Target, Tuning);
		return;
	}
	if (!Target)
	{
		StopMovement();
		return;
	}
	switch (Data->Behavior)
	{
	case EDSEnemyBehavior::Swarm:   ThinkSwarm(Target, Tuning); break;
	case EDSEnemyBehavior::Charger: ThinkCharger(Target, Tuning); break;
	case EDSEnemyBehavior::Blinker: ThinkBlinker(Target, Tuning); break;
	case EDSEnemyBehavior::Kiter:   ThinkRanged(Target, Tuning, true); break;
	case EDSEnemyBehavior::Ranged:  ThinkRanged(Target, Tuning, false); break;
	case EDSEnemyBehavior::Brood:   ThinkBrood(Target, Tuning); break;
	case EDSEnemyBehavior::Melee:
	default:                        ThinkMelee(Target, Tuning); break;
	}
}

float ADSEnemyAIController::EdgeDistance(const ADSCharacterBase* Target) const
{
	const ADSEnemyCharacter* Self = Enemy.Get();
	const float Center = FVector::Dist2D(Self->GetActorLocation(), Target->GetActorLocation());
	return Center - UDSTargetingLibrary::GetCollisionRadius(Self) - UDSTargetingLibrary::GetCollisionRadius(Target);
}

void ADSEnemyAIController::FaceTarget(const ADSCharacterBase* Target) const
{
	ADSEnemyCharacter* Self = Enemy.Get();
	const FVector ToTarget = Target->GetActorLocation() - Self->GetActorLocation();
	Self->SetActorRotation(FRotator(0.f, ToTarget.Rotation().Yaw, 0.f));
}

void ADSEnemyAIController::ChaseAndHit(ADSCharacterBase* Target, const FDSEnemyAITuning& Tuning)
{
	if (EdgeDistance(Target) <= Tuning.MeleeReach)
	{
		StopMovement();
		FaceTarget(Target);
		Enemy->PerformMeleeAttack(Target);
		return;
	}
	MoveToActor(Target, Tuning.MeleeReach * 0.5f, /*bStopOnOverlap*/ true);
}

void ADSEnemyAIController::ThinkMelee(ADSCharacterBase* Target, const FDSEnemyAITuning& Tuning)
{
	ChaseAndHit(Target, Tuning);
}

void ADSEnemyAIController::ThinkSwarm(ADSCharacterBase* Target, const FDSEnemyAITuning& Tuning)
{
	const ADSEnemyCharacter* Self = Enemy.Get();
	if (EdgeDistance(Target) <= Tuning.SwarmCloseDistance)
	{
		ChaseAndHit(Target, Tuning);
		return;
	}
	// Abre em leque: avança desviado do rumo direto para cercar o alvo.
	const FVector ToTarget = (Target->GetActorLocation() - Self->GetActorLocation()) * FVector(1.f, 1.f, 0.f);
	const FVector Flanked = ToTarget.GetSafeNormal().RotateAngleAxis(FMath::RadiansToDegrees(Tuning.SwarmFlankRadians) * SwarmSide, FVector::UpVector);
	const float Step = FMath::Min(ToTarget.Size(), 400.f);
	MoveToLocation(Self->GetActorLocation() + Flanked * Step, 50.f);
}

void ADSEnemyAIController::ThinkCharger(ADSCharacterBase* Target, const FDSEnemyAITuning& Tuning)
{
	const float Now = GetWorld()->GetTimeSeconds();
	if (Now >= NextLungeTime && EdgeDistance(Target) <= Tuning.LungeRange && EdgeDistance(Target) > Tuning.MeleeReach)
	{
		NextLungeTime = Now + Tuning.LungeCooldown;
		Enemy->StartLunge(Tuning.LungeSpeedMultiplier, Tuning.LungeDuration);
	}
	ChaseAndHit(Target, Tuning);
}

void ADSEnemyAIController::ThinkBlinker(ADSCharacterBase* Target, const FDSEnemyAITuning& Tuning)
{
	ADSEnemyCharacter* Self = Enemy.Get();
	if (!bBlinked && Self->GetHealthFraction() <= Tuning.BlinkHealthFraction)
	{
		bBlinked = true;
		// Teleporta para trás do alvo (do ponto de vista do inimigo), projetado no NavMesh.
		const FVector Away = (Target->GetActorLocation() - Self->GetActorLocation()).GetSafeNormal2D();
		FVector Destination = Target->GetActorLocation() + Away * FMath::Min(Tuning.BlinkDistance, 300.f);
		if (const UNavigationSystemV1* NavSystem = FNavigationSystem::GetCurrent<UNavigationSystemV1>(GetWorld()))
		{
			FNavLocation Projected;
			if (NavSystem->ProjectPointToNavigation(Destination, Projected))
			{
				Destination = Projected.Location + FVector(0.f, 0.f, Self->GetSimpleCollisionHalfHeight());
			}
		}
		Self->TeleportTo(Destination, (-Away).Rotation());
	}
	ChaseAndHit(Target, Tuning);
}

void ADSEnemyAIController::ThinkRanged(ADSCharacterBase* Target, const FDSEnemyAITuning& Tuning, bool bKite)
{
	ADSEnemyCharacter* Self = Enemy.Get();
	const float Range = Self->GetStats().AttackRange;
	const float Distance = FVector::Dist2D(Self->GetActorLocation(), Target->GetActorLocation());
	if (bKite && Distance < Tuning.KiteDistance)
	{
		MoveAwayFrom(Target, Tuning.KiteStep); // recua quando um herói chega perto
		return;
	}
	const float StopDistance = Range * Tuning.RangedStopFraction;
	if (Range <= 0.f || Distance > StopDistance)
	{
		MoveToActor(Target, FMath::Max(50.f, StopDistance - 50.f), true);
		return;
	}
	StopMovement();
	FaceTarget(Target);
	Self->PerformRangedAttack(Target);
}

void ADSEnemyAIController::ThinkPassive(ADSCharacterBase* Target, const FDSEnemyAITuning& Tuning)
{
	ADSEnemyCharacter* Self = Enemy.Get();
	if (!Self->HasBeenDamaged() || !Target)
	{
		Wander(Tuning);
		return;
	}
	if (Self->GetHealthFraction() <= Tuning.FleeHealthFraction)
	{
		MoveAwayFrom(Target, Tuning.FleeStep);
		return;
	}
	ChaseAndHit(Target, Tuning);
}

void ADSEnemyAIController::ThinkBrood(ADSCharacterBase* Target, const FDSEnemyAITuning& Tuning)
{
	const ADSEnemyCharacter* Self = Enemy.Get();
	if (Tuning.BroodHealthThresholds.IsValidIndex(BroodThresholdsUsed)
		&& Self->GetHealthFraction() <= Tuning.BroodHealthThresholds[BroodThresholdsUsed])
	{
		++BroodThresholdsUsed;
		SummonBrood(Tuning);
	}
	ThinkCharger(Target, Tuning);
}

void ADSEnemyAIController::MoveAwayFrom(const ADSCharacterBase* Threat, float Distance)
{
	const ADSEnemyCharacter* Self = Enemy.Get();
	const FVector Away = (Self->GetActorLocation() - Threat->GetActorLocation()).GetSafeNormal2D();
	MoveToLocation(Self->GetActorLocation() + Away * Distance, 50.f);
}

void ADSEnemyAIController::Wander(const FDSEnemyAITuning& Tuning)
{
	const float Now = GetWorld()->GetTimeSeconds();
	if (Now < NextWanderTime)
	{
		return;
	}
	NextWanderTime = Now + 6.f; // WANDER_PERIOD_TICKS (60 tiques a 10 Hz)
	const FVector Offset = FVector(FMath::RandPointInCircle(Tuning.WanderRadius), 0.f);
	MoveToLocation(Enemy->GetActorLocation() + Offset, 50.f);
}

void ADSEnemyAIController::SummonBrood(const FDSEnemyAITuning& Tuning)
{
	ADSEnemyCharacter* Self = Enemy.Get();
	const UDSEnemyData* Minion = Self->GetEnemyData()->BroodMinion;
	UWorld* World = GetWorld();
	if (!Minion || !World)
	{
		return;
	}
	// TODO(Core): invocações não entram na contagem da onda; pedir um hook de spawn ao ADSGameMode.
	const int32 Count = FMath::Max(0, Tuning.BroodSummonCount);
	for (int32 i = 0; i < Count; ++i)
	{
		const float Angle = (2.f * PI * i) / FMath::Max(1, Count);
		const FVector Location = Self->GetActorLocation() + FVector(FMath::Cos(Angle), FMath::Sin(Angle), 0.f) * Tuning.BroodRing;
		FActorSpawnParameters Params;
		Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
		if (ADSEnemyCharacter* Spawned = World->SpawnActor<ADSEnemyCharacter>(Self->GetClass(), Location, Self->GetActorRotation(), Params))
		{
			Spawned->InitializeFromData(Minion, Minion->BaseLevel, 1.f, 1.f, false);
		}
	}
}
