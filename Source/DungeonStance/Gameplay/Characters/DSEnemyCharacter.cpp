#include "Gameplay/Characters/DSEnemyCharacter.h"

#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/SkeletalMesh.h"
#include "Engine/World.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Net/UnrealNetwork.h"
#include "TimerManager.h"
#include "Core/DSGameplayTags.h"
#include "Gameplay/AI/DSEnemyAIController.h"
#include "Gameplay/Abilities/DSAbilitySystemComponent.h"
#include "Gameplay/Attributes/DSAttributeSet.h"
#include "Gameplay/Combat/DSProjectile.h"
#include "Gameplay/DSGameplayTagsGameplay.h"
#include "Gameplay/Effects/DSGameplayEffects.h"

namespace
{
	/** Altura do disparo em relação ao centro da cápsula (protótipo: peito). */
	constexpr float ShotForwardOffset = 60.f;
}

ADSEnemyCharacter::ADSEnemyCharacter(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	EnemyAbilitySystem = CreateDefaultSubobject<UDSAbilitySystemComponent>(TEXT("AbilitySystem"));
	EnemyAbilitySystem->SetIsReplicated(true);
	EnemyAbilitySystem->SetReplicationMode(EGameplayEffectReplicationMode::Minimal);
	EnemyAttributes = CreateDefaultSubobject<UDSAttributeSet>(TEXT("Attributes"));

	AbilitySystemComponent = EnemyAbilitySystem;
	AttributeSet = EnemyAttributes;

	AIControllerClass = ADSEnemyAIController::StaticClass();
	AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;
	bUseControllerRotationYaw = false;

	UCharacterMovementComponent* Movement = GetCharacterMovement();
	Movement->bOrientRotationToMovement = true;
	Movement->RotationRate = FRotator(0.f, 540.f, 0.f);
	Movement->bUseRVOAvoidance = true;
	Movement->AvoidanceConsiderationRadius = 200.f;
}

void ADSEnemyCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME_CONDITION(ADSEnemyCharacter, EnemyData, COND_InitialOnly);
	DOREPLIFETIME_CONDITION(ADSEnemyCharacter, bElite, COND_InitialOnly);
}

void ADSEnemyCharacter::PostInitializeComponents()
{
	Super::PostInitializeComponents();
	EnemyAbilitySystem->InitAbilityActorInfo(this, this);
	OnAbilitySystemInitialized();
}

void ADSEnemyCharacter::InitializeFromData(const UDSEnemyData* Data, int32 Level, float HealthMultiplier, float DamageMultiplier, bool bInElite)
{
	if (!HasAuthority() || !Data)
	{
		return;
	}
	EnemyData = const_cast<UDSEnemyData*>(Data); // assets são imutáveis em runtime; o ponteiro só é lido
	bElite = bInElite;
	Stats = Data->ComputeStats(Level, bInElite);
	ApplyStats(HealthMultiplier, DamageMultiplier);

	EnemyAbilitySystem->SetReplicatedLooseTag(DSGameplayTags::Character_Enemy, true);
	EnemyAbilitySystem->SetReplicatedLooseTag(DSGameplayTags::Character_Elite, bInElite);
	EnemyAbilitySystem->SetReplicatedLooseTag(DSGameplayTags::Character_Boss, Data->bIsBoss);
	EnemyAbilitySystem->GrantAbilitySet(Data->AbilitySet, this);

	// Primeira rajada escalonada: recarga + aleatório(0..recarga).
	if (IsRanged())
	{
		DelayNextAttack(Stats.AttackCooldown + FMath::FRandRange(0.f, Stats.AttackCooldown));
	}
	OnRep_EnemyData();
}

void ADSEnemyCharacter::ApplyStats(float HealthMultiplier, float DamageMultiplier)
{
	Stats.MaxHealth *= FMath::Max(0.01f, HealthMultiplier);
	Stats.ContactDps *= FMath::Max(0.f, DamageMultiplier);
	Stats.ProjectileDamage *= FMath::Max(0.f, DamageMultiplier);

	UDSAbilitySystemComponent* ASC = EnemyAbilitySystem;
	ASC->SetNumericAttributeBase(UDSAttributeSet::GetMaxHealthAttribute(), Stats.MaxHealth);
	ASC->SetNumericAttributeBase(UDSAttributeSet::GetHealthAttribute(), Stats.MaxHealth);
	ASC->SetNumericAttributeBase(UDSAttributeSet::GetDamageAttribute(), Stats.ContactDps);
	ASC->SetNumericAttributeBase(UDSAttributeSet::GetArmorAttribute(), Stats.Armor);
	ASC->SetNumericAttributeBase(UDSAttributeSet::GetMoveSpeedAttribute(), Stats.MoveSpeed);
	ASC->SetNumericAttributeBase(UDSAttributeSet::GetAttackRangeAttribute(), Stats.AttackRange);
	ASC->SetNumericAttributeBase(UDSAttributeSet::GetCritChanceAttribute(), 0.f);
	ASC->SetNumericAttributeBase(UDSAttributeSet::GetResourceRegenAttribute(), 0.f);

	GetCapsuleComponent()->SetCapsuleRadius(FMath::Max(10.f, Stats.Radius));
	RefreshMovementSpeed();
}

void ADSEnemyCharacter::OnRep_EnemyData()
{
	ApplyMesh();
	K2_OnEnemyDataSet(EnemyData, bElite);
}

void ADSEnemyCharacter::ApplyMesh()
{
	if (EnemyData && !EnemyData->Mesh.IsNull())
	{
		if (USkeletalMesh* LoadedMesh = EnemyData->Mesh.LoadSynchronous())
		{
			GetMesh()->SetSkeletalMesh(LoadedMesh);
		}
	}
}

bool ADSEnemyCharacter::IsBoss() const
{
	return EnemyData && EnemyData->bIsBoss;
}

bool ADSEnemyCharacter::IsStunned() const
{
	return AbilitySystemComponent && AbilitySystemComponent->HasMatchingGameplayTag(DSTags::Status_Stun);
}

float ADSEnemyCharacter::GetHealthFraction() const
{
	return AttributeSet && AttributeSet->GetMaxHealth() > 0.f ? AttributeSet->GetHealth() / AttributeSet->GetMaxHealth() : 0.f;
}

// ---------------------------------------------------------------------------
// Ataques
// ---------------------------------------------------------------------------

bool ADSEnemyCharacter::IsAttackReady() const
{
	return IsAlive() && !IsStunned() && GetWorld()->GetTimeSeconds() >= NextAttackTime;
}

void ADSEnemyCharacter::DelayNextAttack(float Seconds)
{
	NextAttackTime = GetWorld()->GetTimeSeconds() + FMath::Max(0.f, Seconds);
}

FGameplayEffectSpecHandle ADSEnemyCharacter::MakeFlatDamageSpec(float Damage) const
{
	FGameplayEffectContextHandle Context = EnemyAbilitySystem->MakeEffectContext();
	Context.AddSourceObject(this);
	FGameplayEffectSpecHandle Spec = EnemyAbilitySystem->MakeOutgoingSpec(UDSGE_Damage::StaticClass(), 1.f, Context);
	if (Spec.IsValid())
	{
		Spec.Data->SetSetByCallerMagnitude(DSTags::Data_Damage, Damage);
		Spec.Data->SetSetByCallerMagnitude(DSTags::Data_Magnitude, 1.f);
	}
	return Spec;
}

bool ADSEnemyCharacter::PerformMeleeAttack(ADSCharacterBase* Target)
{
	if (!HasAuthority() || !Target || !IsAttackReady() || !EnemyData)
	{
		return false;
	}
	const float Interval = FMath::Max(0.1f, EnemyData->AITuning.MeleeInterval);
	DelayNextAttack(Interval);
	const FGameplayEffectSpecHandle Spec = MakeFlatDamageSpec(Stats.ContactDps * Interval);
	if (UAbilitySystemComponent* TargetASC = Target->GetAbilitySystemComponent(); TargetASC && Spec.IsValid())
	{
		EnemyAbilitySystem->ApplyGameplayEffectSpecToTarget(*Spec.Data.Get(), TargetASC);
	}
	Multicast_OnAttack(false);
	return true;
}

bool ADSEnemyCharacter::PerformRangedAttack(ADSCharacterBase* Target)
{
	if (!HasAuthority() || !Target || !IsRanged() || !IsAttackReady() || !EnemyData)
	{
		return false;
	}
	DelayNextAttack(Stats.AttackCooldown);
	FVector Direction = Target->GetActorLocation() - GetActorLocation();
	Direction.Z = 0.f;
	Direction = Direction.GetSafeNormal(UE_SMALL_NUMBER, GetActorForwardVector());

	FDSProjectileParams Params;
	Params.Speed = Stats.ProjectileSpeed;
	Params.Range = Stats.AttackRange * 1.25f; // o tiro vai um pouco além do alcance de disparo
	const FVector Origin = GetActorLocation() + Direction * (GetCapsuleComponent()->GetScaledCapsuleRadius() + ShotForwardOffset);
	ADSProjectile::SpawnProjectile(EnemyData->ProjectileClass, this, Origin, Direction, MakeFlatDamageSpec(Stats.ProjectileDamage), Params);
	Multicast_OnAttack(true);
	return true;
}

void ADSEnemyCharacter::Multicast_OnAttack_Implementation(bool bRanged)
{
	K2_OnAttack(bRanged);
}

void ADSEnemyCharacter::StartLunge(float SpeedMultiplier, float Duration)
{
	LungeMultiplier = FMath::Max(1.f, SpeedMultiplier);
	LungeEndTime = GetWorld()->GetTimeSeconds() + Duration;
	RefreshMovementSpeed();
	FTimerHandle Handle;
	GetWorldTimerManager().SetTimer(Handle, FTimerDelegate::CreateWeakLambda(this, [this]() { RefreshMovementSpeed(); }), Duration, false);
}

float ADSEnemyCharacter::GetExtraSpeedMultiplier() const
{
	const UWorld* World = GetWorld();
	return (World && World->GetTimeSeconds() < LungeEndTime) ? LungeMultiplier : 1.f;
}

// ---------------------------------------------------------------------------
// Morte
// ---------------------------------------------------------------------------

void ADSEnemyCharacter::HandleDamageTaken(AActor* DamageInstigator, AActor* DamageCauser, const FGameplayEffectSpec* Spec, float Magnitude)
{
	bHasBeenDamaged = true;
}

void ADSEnemyCharacter::Die(AController* Killer)
{
	if (bDied)
	{
		return;
	}
	Super::Die(Killer);
	if (AController* MyController = GetController())
	{
		MyController->UnPossess();
	}
	OnEnemyDied.Broadcast(this, Killer);
}
