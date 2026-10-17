#include "Gameplay/Combat/DSSupportZone.h"

#include "AbilitySystemComponent.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "Net/UnrealNetwork.h"
#include "TimerManager.h"
#include "Gameplay/Attributes/DSAttributeSet.h"
#include "Gameplay/Characters/DSCharacterBase.h"
#include "Gameplay/Combat/DSTargetingLibrary.h"
#include "Gameplay/Effects/DSBuffHelpers.h"
#include "Gameplay/Effects/DSGameplayEffects.h"

ADSSupportZone::ADSSupportZone()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;
	SetReplicateMovement(false);
	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
}

void ADSSupportZone::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(ADSSupportZone, Radius);
	DOREPLIFETIME(ADSSupportZone, Duration);
	DOREPLIFETIME(ADSSupportZone, bTaunts);
}

void ADSSupportZone::InitZone(ADSCharacterBase* InCaster, const FDSResolvedSupport& InResolved)
{
	if (!HasAuthority())
	{
		return;
	}
	Caster = InCaster;
	Resolved = InResolved;
	Radius = InResolved.Radius;
	Duration = InResolved.Duration;
	bTaunts = InResolved.bTaunts;
	SetLifeSpan(FMath::Max(0.1f, Duration));
	OnRep_Radius();

	Pulse();
	GetWorldTimerManager().SetTimer(PulseTimer, this, &ADSSupportZone::Pulse, TickInterval, true);
}

void ADSSupportZone::OnRep_Radius()
{
	K2_OnZoneStarted(Radius, Duration);
}

void ADSSupportZone::Pulse()
{
	ADSCharacterBase* CasterCharacter = Caster.Get();
	UAbilitySystemComponent* SourceASC = CasterCharacter ? CasterCharacter->GetAbilitySystemComponent() : nullptr;
	if (!SourceASC)
	{
		Destroy();
		return;
	}

	TArray<ADSCharacterBase*> Allies;
	UDSTargetingLibrary::FindInRadius(CasterCharacter, GetActorLocation(), Radius, EDSTargetFilter::Ally, Allies);

	FDSBuffValues Values;
	Values.DamageMultiplier = Resolved.DamageMultiplier;
	Values.AttackSpeedMultiplier = Resolved.AttackSpeedMultiplier;
	Values.MoveSpeedMultiplier = Resolved.MoveSpeedMultiplier;
	Values.ArmorBonus = Resolved.ArmorBonus;
	Values.DamageReduction = Resolved.DamageReduction;
	Values.CritChanceBonus = Resolved.CritChanceBonus;

	for (ADSCharacterBase* Ally : Allies)
	{
		UAbilitySystemComponent* TargetASC = Ally->GetAbilitySystemComponent();
		// Duração = 2 tiques: some logo depois que o aliado sai da área.
		DSBuffHelpers::ApplyBuff(SourceASC, TargetASC, UDSGE_ZoneBuff::StaticClass(), Values, TickInterval * 2.f);
		if (Resolved.RegenPct > 0.f && TargetASC)
		{
			const float MaxHealth = TargetASC->GetNumericAttribute(UDSAttributeSet::GetMaxHealthAttribute());
			DSBuffHelpers::ApplyHeal(SourceASC, TargetASC, MaxHealth * Resolved.RegenPct * TickInterval);
		}
	}
}

ADSSupportZone* ADSSupportZone::FindTauntZoneAt(const UWorld* World, const FVector& Location)
{
	if (!World)
	{
		return nullptr;
	}
	ADSSupportZone* Best = nullptr;
	float BestDistSq = TNumericLimits<float>::Max();
	for (TActorIterator<ADSSupportZone> It(World); It; ++It)
	{
		ADSSupportZone* Zone = *It;
		const float DistSq = FVector::DistSquared2D(Zone->GetActorLocation(), Location);
		if (Zone->IsTaunting() && Zone->GetCaster() && DistSq <= FMath::Square(Zone->GetRadius()) && DistSq < BestDistSq)
		{
			BestDistSq = DistSq;
			Best = Zone;
		}
	}
	return Best;
}
