#include "Gameplay/Effects/DSStatusLibrary.h"

#include "AbilitySystemComponent.h"
#include "GameplayEffect.h"
#include "Gameplay/Combat/DSDamageMath.h"
#include "Gameplay/Effects/DSStatusEffects.h"
#include "Gameplay/DSGameplayTagsGameplay.h"
#include "Core/DSGameplayTags.h"

FActiveGameplayEffectHandle UDSStatusLibrary::ApplyStatusEffect(UAbilitySystemComponent* Source, UAbilitySystemComponent* Target,
	TSubclassOf<UGameplayEffect> EffectClass, float Duration, float Magnitude, float DamagePerTick)
{
	if (!Target || !EffectClass || Duration <= 0.f || !Target->IsOwnerActorAuthoritative()
		|| Target->HasMatchingGameplayTag(DSTags::State_Dead))
	{
		return FActiveGameplayEffectHandle();
	}
	UAbilitySystemComponent* Instigator = Source ? Source : Target;
	FGameplayEffectContextHandle Context = Instigator->MakeEffectContext();
	Context.AddInstigator(Instigator->GetOwnerActor(), Instigator->GetAvatarActor());

	const FGameplayEffectSpecHandle Spec = Instigator->MakeOutgoingSpec(EffectClass, 1.f, Context);
	if (!Spec.IsValid())
	{
		return FActiveGameplayEffectHandle();
	}
	Spec.Data->SetSetByCallerMagnitude(DSTags::Data_Duration, Duration);
	Spec.Data->SetSetByCallerMagnitude(DSTags::Data_Magnitude, Magnitude);
	Spec.Data->SetSetByCallerMagnitude(DSTags::Data_Damage, DamagePerTick);
	return Instigator->ApplyGameplayEffectSpecToTarget(*Spec.Data.Get(), Target);
}

FActiveGameplayEffectHandle UDSStatusLibrary::ApplyMark(UAbilitySystemComponent* Source, UAbilitySystemComponent* Target, float Vulnerability, float Duration)
{
	return ApplyStatusEffect(Source, Target, UDSGE_Mark::StaticClass(), Duration, FMath::Max(0.f, Vulnerability));
}

FActiveGameplayEffectHandle UDSStatusLibrary::ApplyDot(UAbilitySystemComponent* Source, UAbilitySystemComponent* Target, EDSDotKind Kind, float TotalDamage, float Duration)
{
	if (TotalDamage <= 0.f || Duration <= 0.f)
	{
		return FActiveGameplayEffectHandle();
	}
	TSubclassOf<UGameplayEffect> EffectClass = UDSGE_Bleed::StaticClass();
	if (Kind == EDSDotKind::Burn)
	{
		EffectClass = UDSGE_Burn::StaticClass();
	}
	else if (Kind == EDSDotKind::Poison)
	{
		EffectClass = UDSGE_Poison::StaticClass();
	}
	// dps × intervalo, como tickStatus (dano creditado ao dono do acúmulo).
	const float DamagePerTick = (TotalDamage / Duration) * UDSGE_DamageOverTime::TickInterval;
	return ApplyStatusEffect(Source, Target, EffectClass, Duration, TotalDamage, DamagePerTick);
}

FActiveGameplayEffectHandle UDSStatusLibrary::ApplySlow(UAbilitySystemComponent* Source, UAbilitySystemComponent* Target, float Pct, float Duration)
{
	const float Value = DSDamageMath::EffectiveSlow(Pct, IsBoss(Target));
	if (Value <= 0.f)
	{
		return FActiveGameplayEffectHandle();
	}
	return ApplyStatusEffect(Source, Target, UDSGE_Slow::StaticClass(), Duration, Value);
}

FActiveGameplayEffectHandle UDSStatusLibrary::ApplyStun(UAbilitySystemComponent* Source, UAbilitySystemComponent* Target, float Duration)
{
	if (IsBoss(Target))
	{
		// applyStun: chefe vira lentidão de 30% (o fator de chefe é desfeito antes de aplicar).
		return ApplySlow(Source, Target, DSDamageMath::BossStunAsSlow / DSDamageMath::BossSlowFactor, Duration);
	}
	return ApplyStatusEffect(Source, Target, UDSGE_Stun::StaticClass(), FMath::Min(Duration, DSDamageMath::MaxStunSeconds), 1.f);
}

FActiveGameplayEffectHandle UDSStatusLibrary::ApplyWeaken(UAbilitySystemComponent* Source, UAbilitySystemComponent* Target, float Pct, float Duration)
{
	return ApplyStatusEffect(Source, Target, UDSGE_Weaken::StaticClass(), Duration, FMath::Clamp(Pct, 0.f, 1.f));
}

FActiveGameplayEffectHandle UDSStatusLibrary::ApplyExposed(UAbilitySystemComponent* Source, UAbilitySystemComponent* Target, float Vulnerability, float Duration)
{
	return ApplyStatusEffect(Source, Target, UDSGE_Exposed::StaticClass(), Duration, FMath::Max(0.f, Vulnerability));
}

FActiveGameplayEffectHandle UDSStatusLibrary::ApplyTaunt(UAbilitySystemComponent* Source, UAbilitySystemComponent* Target, float Duration)
{
	return ApplyStatusEffect(Source, Target, UDSGE_Taunt::StaticClass(), Duration, 1.f);
}

float UDSStatusLibrary::GetStrongestStatusMagnitude(const UAbilitySystemComponent* ASC, FGameplayTag StatusTag)
{
	if (!ASC || !StatusTag.IsValid() || !ASC->HasMatchingGameplayTag(StatusTag))
	{
		return 0.f;
	}
	float Strongest = 0.f;
	const FGameplayEffectQuery Query = FGameplayEffectQuery::MakeQuery_MatchAnyOwningTags(FGameplayTagContainer(StatusTag));
	for (const FActiveGameplayEffectHandle& Handle : ASC->GetActiveEffects(Query))
	{
		if (const FActiveGameplayEffect* Active = ASC->GetActiveGameplayEffect(Handle))
		{
			if (!Active->bIsInhibited)
			{
				Strongest = FMath::Max(Strongest, Active->Spec.GetSetByCallerMagnitude(DSTags::Data_Magnitude, false, 0.f));
			}
		}
	}
	return Strongest;
}

float UDSStatusLibrary::GetSpeedMultiplier(const UAbilitySystemComponent* ASC)
{
	if (!ASC)
	{
		return 1.f;
	}
	if (ASC->HasMatchingGameplayTag(DSTags::Status_Stun))
	{
		return 0.f;
	}
	return 1.f - FMath::Clamp(GetStrongestStatusMagnitude(ASC, DSTags::Status_Slow), 0.f, DSDamageMath::MaxSlow);
}

AActor* UDSStatusLibrary::GetTauntInstigator(const UAbilitySystemComponent* ASC)
{
	if (!ASC || !ASC->HasMatchingGameplayTag(DSTags::Status_Taunt))
	{
		return nullptr;
	}
	AActor* Latest = nullptr;
	float LatestStart = -1.f;
	const FGameplayEffectQuery Query = FGameplayEffectQuery::MakeQuery_MatchAnyOwningTags(FGameplayTagContainer(DSTags::Status_Taunt));
	for (const FActiveGameplayEffectHandle& Handle : ASC->GetActiveEffects(Query))
	{
		const FActiveGameplayEffect* Active = ASC->GetActiveGameplayEffect(Handle);
		if (Active && Active->StartWorldTime > LatestStart)
		{
			LatestStart = Active->StartWorldTime;
			Latest = Active->Spec.GetEffectContext().GetEffectCauser();
		}
	}
	return Latest;
}

bool UDSStatusLibrary::IsBoss(const UAbilitySystemComponent* ASC)
{
	return ASC && ASC->HasMatchingGameplayTag(DSGameplayTags::Character_Boss);
}
