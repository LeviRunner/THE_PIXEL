#include "Gameplay/Combat/DSProjectile.h"

#include "AbilitySystemComponent.h"
#include "Components/SphereComponent.h"
#include "Engine/World.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Net/UnrealNetwork.h"
#include "Gameplay/Characters/DSCharacterBase.h"
#include "Gameplay/Combat/DSTargetingLibrary.h"

ADSProjectile::ADSProjectile()
{
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;
	SetReplicateMovement(true);

	Collision = CreateDefaultSubobject<USphereComponent>(TEXT("Collision"));
	Collision->InitSphereRadius(15.f);
	Collision->SetCollisionProfileName(TEXT("Projectile"));
	Collision->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	Collision->SetCollisionResponseToAllChannels(ECR_Ignore);
	Collision->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Block);
	Collision->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	Collision->SetGenerateOverlapEvents(true);
	RootComponent = Collision;

	Movement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("Movement"));
	Movement->UpdatedComponent = Collision;
	Movement->InitialSpeed = 2000.f;
	Movement->MaxSpeed = 0.f; // sem limite
	Movement->ProjectileGravityScale = 0.f;
	Movement->bRotationFollowsVelocity = true;
}

ADSProjectile* ADSProjectile::SpawnProjectile(TSubclassOf<ADSProjectile> ProjectileClass, ADSCharacterBase* InShooter, const FVector& Origin,
	const FVector& Direction, const FGameplayEffectSpecHandle& InDamageSpec, const FDSProjectileParams& InParams)
{
	UWorld* World = InShooter ? InShooter->GetWorld() : nullptr;
	if (!World || !InShooter->HasAuthority())
	{
		return nullptr;
	}
	const TSubclassOf<ADSProjectile> ClassToSpawn = ProjectileClass ? ProjectileClass : TSubclassOf<ADSProjectile>(ADSProjectile::StaticClass());
	const FTransform SpawnTransform(Direction.GetSafeNormal().Rotation(), Origin);
	ADSProjectile* Projectile = World->SpawnActorDeferred<ADSProjectile>(ClassToSpawn, SpawnTransform, InShooter, InShooter,
		ESpawnActorCollisionHandlingMethod::AlwaysSpawn);
	if (!Projectile)
	{
		return nullptr;
	}
	Projectile->DamageSpec = InDamageSpec;
	Projectile->Shooter = InShooter;
	Projectile->Params = InParams;
	Projectile->RemainingPierce = InParams.PierceCount;
	Projectile->ReplicatedSpeed = InParams.Speed;
	Projectile->Movement->InitialSpeed = InParams.Speed;
	Projectile->Collision->IgnoreActorWhenMoving(InShooter, true);
	Projectile->FinishSpawning(SpawnTransform);
	return Projectile;
}

void ADSProjectile::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME_CONDITION(ADSProjectile, ReplicatedSpeed, COND_InitialOnly);
}

void ADSProjectile::BeginPlay()
{
	Super::BeginPlay();
	StartLocation = GetActorLocation();
	if (HasAuthority())
	{
		Collision->OnComponentBeginOverlap.AddDynamic(this, &ADSProjectile::OnOverlap);
		Collision->OnComponentHit.AddDynamic(this, &ADSProjectile::OnHit);
	}
}

void ADSProjectile::OnRep_Speed()
{
	// Cliente: alinha a velocidade simulada com a do servidor.
	if (Movement && ReplicatedSpeed > 0.f)
	{
		Movement->Velocity = GetActorForwardVector() * ReplicatedSpeed;
	}
}

void ADSProjectile::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	if (HasAuthority() && !bFinished && FVector::Dist(StartLocation, GetActorLocation()) >= Params.Range)
	{
		Finish(GetActorLocation()); // fim do alcance: projétil em área explode aqui
	}
}

void ADSProjectile::OnOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (!bFinished)
	{
		TryHitCharacter(Cast<ADSCharacterBase>(OtherActor));
	}
}

void ADSProjectile::OnHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	if (!bFinished && !Cast<ADSCharacterBase>(OtherActor))
	{
		Finish(Hit.ImpactPoint); // terreno/parede
	}
}

bool ADSProjectile::TryHitCharacter(ADSCharacterBase* Target)
{
	const ADSCharacterBase* ShooterCharacter = Shooter.Get();
	if (!Target || !ShooterCharacter || !ShooterCharacter->IsHostileTo(Target) || !Target->IsAlive() || AlreadyHit.Contains(Target))
	{
		return false;
	}
	AlreadyHit.Add(Target);
	if (Params.AoERadius > 0.f)
	{
		Finish(GetActorLocation());
		return true;
	}
	ApplyDamage(Target);
	if (--RemainingPierce < 0)
	{
		Finish(GetActorLocation());
	}
	return true;
}

void ADSProjectile::ApplyDamage(ADSCharacterBase* Target) const
{
	const ADSCharacterBase* ShooterCharacter = Shooter.Get();
	UAbilitySystemComponent* SourceASC = ShooterCharacter ? ShooterCharacter->GetAbilitySystemComponent() : nullptr;
	UAbilitySystemComponent* TargetASC = Target ? Target->GetAbilitySystemComponent() : nullptr;
	if (SourceASC && TargetASC && DamageSpec.IsValid())
	{
		SourceASC->ApplyGameplayEffectSpecToTarget(*DamageSpec.Data.Get(), TargetASC);
	}
}

void ADSProjectile::Explode(const FVector& Location)
{
	TArray<ADSCharacterBase*> Targets;
	UDSTargetingLibrary::FindInRadius(Shooter.Get(), Location, Params.AoERadius, EDSTargetFilter::Hostile, Targets);
	for (ADSCharacterBase* Target : Targets)
	{
		ApplyDamage(Target);
	}
}

void ADSProjectile::Finish(const FVector& Location)
{
	if (bFinished)
	{
		return;
	}
	bFinished = true;
	if (Params.AoERadius > 0.f && Shooter.IsValid())
	{
		Explode(Location);
	}
	Multicast_Impact(Location, Params.AoERadius);
	SetActorEnableCollision(false);
	Movement->StopMovementImmediately();
	SetLifeSpan(0.2f); // dá tempo do multicast chegar antes de destruir
}

void ADSProjectile::Multicast_Impact_Implementation(FVector_NetQuantize Location, float Radius)
{
	K2_OnImpact(Location, Radius);
}
