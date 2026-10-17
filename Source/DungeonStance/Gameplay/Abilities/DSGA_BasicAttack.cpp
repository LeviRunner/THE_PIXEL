#include "Gameplay/Abilities/DSGA_BasicAttack.h"

#include "AbilitySystemComponent.h"
#include "Animation/AnimMontage.h"
#include "Gameplay/Attributes/DSAttributeSet.h"
#include "Gameplay/Characters/DSCharacterBase.h"
#include "Gameplay/Combat/DSProjectile.h"
#include "Gameplay/DSGameplayTagsGameplay.h"
#include "Core/DSGameplayTags.h"

UDSGA_BasicAttack::UDSGA_BasicAttack(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	ActivationPolicy = EDSAbilityActivationPolicy::WhileInputActive;
	SetAssetTags(FGameplayTagContainer(DSTags::Ability_Attack));
	CooldownTags.AddTag(DSGameplayTags::Cooldown_Attack);
	bUseCooldownReduction = false;
	BaseCooldown = 1.f;
	ActivationBlockedTags.AddTag(DSGameplayTags::State_Dodging);
}

float UDSGA_BasicAttack::GetCooldownDuration() const
{
	const float AttackSpeed = GetOwnerAttribute(UDSAttributeSet::GetAttackSpeedAttribute());
	return AttackSpeed > 0.f ? 1.f / AttackSpeed : BaseCooldown;
}

const FDSBasicAttackInfo& UDSGA_BasicAttack::GetAttackInfo() const
{
	const UDSClassData* ClassData = GetClassData();
	return ClassData ? ClassData->BasicAttack : FallbackAttackInfo;
}

void UDSGA_BasicAttack::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	FaceAim();
	if (AttackMontage)
	{
		const float Rate = FMath::Max(0.1f, GetOwnerAttribute(UDSAttributeSet::GetAttackSpeedAttribute()) * MontageRateScale);
		if (UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo())
		{
			ASC->PlayMontage(this, ActivationInfo, AttackMontage, Rate);
		}
	}

	if (IsServer())
	{
		const FDSBasicAttackInfo& Info = GetAttackInfo();
		switch (Info.AttackType)
		{
		case EDSAttackType::Melee:         PerformMelee(Info, false); break;
		case EDSAttackType::MeleeAoE:      PerformMelee(Info, true); break;
		case EDSAttackType::Projectile:    PerformProjectile(Info, false); break;
		case EDSAttackType::ProjectileAoE: PerformProjectile(Info, true); break;
		default: break;
		}
	}
	EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
}

float UDSGA_BasicAttack::NextComboMultiplier()
{
	const FDSBasicAttackInfo& Info = GetAttackInfo();
	if (Info.ComboLength <= 0)
	{
		return 1.f;
	}
	ComboCounter = (ComboCounter % Info.ComboLength) + 1;
	return ComboCounter == Info.ComboLength ? Info.ComboFinisherMultiplier : 1.f;
}

void UDSGA_BasicAttack::PerformMelee(const FDSBasicAttackInfo& Info, bool bHitAll)
{
	const float Range = GetOwnerAttribute(UDSAttributeSet::GetAttackRangeAttribute());
	TArray<ADSCharacterBase*> Targets;
	FindHostilesInArc(Range, Info.MeleeArcDegrees, Targets);
	if (!bHitAll && Targets.Num() > 1)
	{
		Targets.SetNum(1); // FindInArc ordena por distância: fica o mais próximo
	}
	if (Targets.Num() > 0)
	{
		ApplyDamageToTargets(Targets, NextComboMultiplier());
	}
}

void UDSGA_BasicAttack::PerformProjectile(const FDSBasicAttackInfo& Info, bool bAoE)
{
	ADSCharacterBase* Character = GetDSCharacter();
	if (!Character)
	{
		return;
	}
	FDSProjectileParams Params;
	Params.Speed = Info.ProjectileSpeed;
	Params.Range = GetOwnerAttribute(UDSAttributeSet::GetAttackRangeAttribute());
	Params.AoERadius = bAoE ? Info.AoERadius : 0.f;

	const FVector Direction = GetAimDirection();
	const FVector Origin = Character->GetActorLocation() + Direction * 50.f;
	const FGameplayEffectSpecHandle Spec = MakeDamageSpec(NextComboMultiplier());
	if (ADSProjectile::SpawnProjectile(Info.ProjectileClass, Character, Origin, Direction, Spec, Params))
	{
		// Recurso por disparo (classes à distância regeneram; o ganho por acerto delas é 0).
		GainResourceOnHit(1);
	}
}

void UDSGA_BasicAttack::FaceAim() const
{
	if (ADSCharacterBase* Character = GetDSCharacter())
	{
		const FVector Direction = GetAimDirection();
		if (!Direction.IsNearlyZero())
		{
			Character->SetActorRotation(FRotator(0.f, Direction.Rotation().Yaw, 0.f));
		}
	}
}
