#include "Gameplay/Abilities/DSGA_SupportAura.h"

#include "AbilitySystemComponent.h"
#include "Engine/World.h"
#include "Gameplay/Abilities/DSAbilitySystemComponent.h"
#include "Gameplay/Abilities/DSSupportAbilityData.h"
#include "Gameplay/Attributes/DSAttributeSet.h"
#include "Gameplay/Characters/DSCharacterBase.h"
#include "Gameplay/Characters/DSHeroCharacter.h"
#include "Gameplay/Combat/DSSupportZone.h"
#include "Gameplay/DSGameplayTagsGameplay.h"
#include "Gameplay/Effects/DSBuffHelpers.h"
#include "Gameplay/Effects/DSGameplayEffects.h"
#include "Gameplay/Stance/DSStanceComponent.h"
#include "Core/DSGameplayTags.h"

UDSGA_SupportAura::UDSGA_SupportAura(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	SetAssetTags(FGameplayTagContainer(DSTags::Ability_Support));
	CooldownTags.AddTag(DSGameplayTags::Cooldown_Support);
	MaxCooldownReduction = DSSupportResolver::MaxSupportCooldownReduction;
	bUseCooldownReduction = false; // apoio usa a redução das cartas (support_cooldown_reduction)
}

FDSResolvedSupport UDSGA_SupportAura::ResolveForCaster() const
{
	const ADSHeroCharacter* Hero = GetHero();
	const UDSStanceComponent* Stance = Hero ? Hero->GetStanceComponent() : nullptr;
	auto CardStat = [Stance](FName Stat, float Base)
	{
		return Stance ? Stance->GetCardStat(Stat, Base) : Base;
	};
	return SupportData ? DSSupportResolver::Resolve(*SupportData, CardStat) : FDSResolvedSupport();
}

float UDSGA_SupportAura::GetCooldownDuration() const
{
	return SupportData ? ResolveForCaster().Cooldown : BaseCooldown;
}

FGameplayTagContainer UDSGA_SupportAura::GetAbilityCooldownTags() const
{
	// Só a tag própria: cada habilidade de apoio tem recarga independente.
	FGameplayTagContainer Tags;
	if (SupportData && SupportData->CooldownTag.IsValid())
	{
		Tags.AddTag(SupportData->CooldownTag);
	}
	else
	{
		Tags.AppendTags(CooldownTags);
	}
	return Tags;
}

void UDSGA_SupportAura::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	if (!SupportData || !CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}
	if (IsServer())
	{
		const FDSResolvedSupport Resolved = ResolveForCaster();
		if (Resolved.Kind == EDSSupportKind::Zone)
		{
			SpawnZone(Resolved);
		}
		else
		{
			ApplyToAllies(Resolved);
		}
		if (SupportData->CastCueTag.IsValid())
		{
			FGameplayCueParameters Cue;
			Cue.Location = GetDSCharacter() ? GetDSCharacter()->GetActorLocation() : FVector::ZeroVector;
			Cue.RawMagnitude = Resolved.Radius;
			GetAbilitySystemComponentFromActorInfo()->ExecuteGameplayCue(SupportData->CastCueTag, Cue);
		}
	}
	EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
}

void UDSGA_SupportAura::ApplyToAllies(const FDSResolvedSupport& Resolved)
{
	const ADSCharacterBase* Caster = GetDSCharacter();
	if (!Caster)
	{
		return;
	}
	TArray<ADSCharacterBase*> Allies;
	FindAlliesInRadius(Caster->GetActorLocation(), Resolved.Radius, Allies);
	for (ADSCharacterBase* Ally : Allies)
	{
		ApplyToAlly(Ally, Resolved);
	}
}

void UDSGA_SupportAura::ApplyToAlly(ADSCharacterBase* Ally, const FDSResolvedSupport& Resolved) const
{
	UAbilitySystemComponent* SourceASC = GetAbilitySystemComponentFromActorInfo();
	UAbilitySystemComponent* TargetASC = Ally ? Ally->GetAbilitySystemComponent() : nullptr;
	if (!SourceASC || !TargetASC)
	{
		return;
	}

	FDSBuffValues Values;
	Values.DamageMultiplier = Resolved.DamageMultiplier;
	Values.AttackSpeedMultiplier = Resolved.AttackSpeedMultiplier;
	Values.MoveSpeedMultiplier = Resolved.MoveSpeedMultiplier;
	Values.ArmorBonus = Resolved.ArmorBonus;
	Values.CritChanceBonus = Resolved.CritChanceBonus;
	Values.DamageReduction = Resolved.Kind == EDSSupportKind::Zone ? 0.f : Resolved.DamageReduction;
	DSBuffHelpers::ApplyBuff(SourceASC, TargetASC, UDSGE_SupportBuff::StaticClass(), Values, Resolved.Duration);

	const float MaxHealth = TargetASC->GetNumericAttribute(UDSAttributeSet::GetMaxHealthAttribute());
	switch (Resolved.Kind)
	{
	case EDSSupportKind::Heal:
		DSBuffHelpers::ApplyHeal(SourceASC, TargetASC, MaxHealth * Resolved.Magnitude);
		break;
	case EDSSupportKind::Shield:
		// TODO: o escudo do protótipo expira após Duration; aqui ele dura até ser consumido.
		DSBuffHelpers::ApplyShield(SourceASC, TargetASC, Resolved.Magnitude);
		break;
	case EDSSupportKind::Rally:
		if (UDSAbilitySystemComponent* DSTarget = Cast<UDSAbilitySystemComponent>(TargetASC))
		{
			DSTarget->ResetCooldownsWithTags(FGameplayTagContainer(DSTags::Cooldown_Dodge));
		}
		break;
	default:
		break;
	}
	// Cura contínua (Flor da Vida, Sentinela): TODO aplicar como GE periódico; hoje só a parte instantânea.
}

void UDSGA_SupportAura::SpawnZone(const FDSResolvedSupport& Resolved) const
{
	ADSCharacterBase* Caster = GetDSCharacter();
	UWorld* World = Caster ? Caster->GetWorld() : nullptr;
	if (!World)
	{
		return;
	}
	const TSubclassOf<ADSSupportZone> ZoneClass = SupportData->ZoneClass ? SupportData->ZoneClass : TSubclassOf<ADSSupportZone>(ADSSupportZone::StaticClass());
	FActorSpawnParameters Params;
	Params.Owner = Caster;
	Params.Instigator = Caster;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	const FVector Ground = Caster->GetActorLocation() - FVector(0.f, 0.f, Caster->GetSimpleCollisionHalfHeight());
	if (ADSSupportZone* Zone = World->SpawnActor<ADSSupportZone>(ZoneClass, Ground, FRotator::ZeroRotator, Params))
	{
		Zone->InitZone(Caster, Resolved);
	}
}
