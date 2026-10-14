#include "Gameplay/Attributes/DSAttributeSet.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "GameplayEffect.h"
#include "GameplayEffectExtension.h"
#include "Net/UnrealNetwork.h"
#include "Core/DSGameplayTags.h"
#include "Gameplay/DSGameplayTagsGameplay.h"

UDSAttributeSet::UDSAttributeSet()
{
	// Defaults do protótipo (classes.ts → stats()); a classe/inimigo sobrescreve na inicialização.
	InitHealth(1000.f);
	InitMaxHealth(1000.f);
	InitShield(0.f);
	InitResource(100.f);
	InitMaxResource(100.f);
	InitStamina(100.f);
	InitMaxStamina(100.f);
	InitDamage(25.f);
	InitAttackSpeed(1.f);
	InitAttackRange(200.f);
	InitCritChance(0.05f);
	InitCritDamage(1.5f);
	InitCooldownReduction(0.f);
	InitLifesteal(0.f);
	InitArmor(10.f);
	InitDamageReduction(0.f);
	InitDodgeChance(0.f);
	InitHealthRegen(0.f);
	InitResourceRegen(0.f);
	InitMoveSpeed(500.f);
	InitIncomingDamage(0.f);
	InitIncomingHeal(0.f);
}

void UDSAttributeSet::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION_NOTIFY(UDSAttributeSet, Health, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UDSAttributeSet, MaxHealth, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UDSAttributeSet, Shield, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UDSAttributeSet, Resource, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UDSAttributeSet, MaxResource, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UDSAttributeSet, Stamina, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UDSAttributeSet, MaxStamina, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UDSAttributeSet, Damage, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UDSAttributeSet, AttackSpeed, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UDSAttributeSet, AttackRange, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UDSAttributeSet, CritChance, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UDSAttributeSet, CritDamage, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UDSAttributeSet, CooldownReduction, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UDSAttributeSet, Lifesteal, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UDSAttributeSet, Armor, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UDSAttributeSet, DamageReduction, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UDSAttributeSet, DodgeChance, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UDSAttributeSet, HealthRegen, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UDSAttributeSet, ResourceRegen, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UDSAttributeSet, MoveSpeed, COND_None, REPNOTIFY_Always);
}

// ---------------------------------------------------------------------------
// Clamp
// ---------------------------------------------------------------------------

void UDSAttributeSet::ClampAttribute(const FGameplayAttribute& Attribute, float& NewValue) const
{
	if (Attribute == GetHealthAttribute())
	{
		NewValue = FMath::Clamp(NewValue, 0.f, GetMaxHealth());
	}
	else if (Attribute == GetResourceAttribute())
	{
		NewValue = FMath::Clamp(NewValue, 0.f, GetMaxResource());
	}
	else if (Attribute == GetStaminaAttribute())
	{
		NewValue = FMath::Clamp(NewValue, 0.f, GetMaxStamina());
	}
	else if (Attribute == GetMaxHealthAttribute() || Attribute == GetMaxResourceAttribute() || Attribute == GetMaxStaminaAttribute())
	{
		NewValue = FMath::Max(NewValue, 1.f);
	}
	else if (Attribute == GetShieldAttribute() || Attribute == GetArmorAttribute() || Attribute == GetDamageAttribute()
		|| Attribute == GetAttackRangeAttribute() || Attribute == GetMoveSpeedAttribute() || Attribute == GetLifestealAttribute())
	{
		NewValue = FMath::Max(NewValue, 0.f);
	}
	else if (Attribute == GetAttackSpeedAttribute())
	{
		NewValue = FMath::Max(NewValue, 0.05f);
	}
	else if (Attribute == GetCritChanceAttribute())
	{
		NewValue = FMath::Clamp(NewValue, 0.f, 1.f);
	}
	else if (Attribute == GetCritDamageAttribute())
	{
		NewValue = FMath::Max(NewValue, 1.f);
	}
	else if (Attribute == GetCooldownReductionAttribute())
	{
		NewValue = FMath::Clamp(NewValue, 0.f, MaxCooldownReduction);
	}
	else if (Attribute == GetDodgeChanceAttribute())
	{
		NewValue = FMath::Clamp(NewValue, 0.f, MaxDodgeChance);
	}
	else if (Attribute == GetDamageReductionAttribute())
	{
		NewValue = FMath::Clamp(NewValue, 0.f, MaxDamageReduction);
	}
}

void UDSAttributeSet::PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue)
{
	Super::PreAttributeChange(Attribute, NewValue);
	ClampAttribute(Attribute, NewValue);
}

void UDSAttributeSet::PreAttributeBaseChange(const FGameplayAttribute& Attribute, float& NewValue) const
{
	Super::PreAttributeBaseChange(Attribute, NewValue);
	ClampAttribute(Attribute, NewValue);
}

void UDSAttributeSet::PostAttributeChange(const FGameplayAttribute& Attribute, float OldValue, float NewValue)
{
	Super::PostAttributeChange(Attribute, OldValue, NewValue);

	UAbilitySystemComponent* ASC = GetOwningAbilitySystemComponent();
	if (!ASC || !ASC->IsOwnerActorAuthoritative())
	{
		return; // ajustes derivados só no servidor; o cliente recebe por replicação
	}

	// Vida máxima subiu (carta/buff): a vida atual sobe junto; se caiu, só re-clampa.
	if (Attribute == GetMaxHealthAttribute())
	{
		const float Delta = NewValue - OldValue;
		const float Current = GetHealth();
		const float Target = (Delta > 0.f && Current > 0.f) ? Current + Delta : FMath::Min(Current, NewValue);
		ASC->ApplyModToAttribute(GetHealthAttribute(), EGameplayModOp::Override, Target);
	}
	else if (Attribute == GetMaxResourceAttribute() && GetResource() > NewValue)
	{
		ASC->ApplyModToAttribute(GetResourceAttribute(), EGameplayModOp::Override, NewValue);
	}
	else if (Attribute == GetMaxStaminaAttribute() && GetStamina() > NewValue)
	{
		ASC->ApplyModToAttribute(GetStaminaAttribute(), EGameplayModOp::Override, NewValue);
	}

	if (bOutOfHealth && Attribute == GetHealthAttribute() && NewValue > 0.f)
	{
		bOutOfHealth = false; // reviveu
	}
}

// ---------------------------------------------------------------------------
// Execução de GE
// ---------------------------------------------------------------------------

bool UDSAttributeSet::PreGameplayEffectExecute(FGameplayEffectModCallbackData& Data)
{
	if (!Super::PreGameplayEffectExecute(Data))
	{
		return false;
	}

	// Invulnerável (i-frames da esquiva) ou já morto: ignora dano.
	if (Data.EvaluatedData.Attribute == GetIncomingDamageAttribute())
	{
		const UAbilitySystemComponent& ASC = Data.Target;
		if (ASC.HasMatchingGameplayTag(DSTags::State_Invulnerable) || ASC.HasMatchingGameplayTag(DSTags::State_Dead))
		{
			Data.EvaluatedData.Magnitude = 0.f;
		}
	}
	return true;
}

void UDSAttributeSet::PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data)
{
	Super::PostGameplayEffectExecute(Data);

	if (Data.EvaluatedData.Attribute == GetIncomingDamageAttribute())
	{
		HandleIncomingDamage(Data);
	}
	else if (Data.EvaluatedData.Attribute == GetIncomingHealAttribute())
	{
		HandleIncomingHeal(Data);
	}
	else if (Data.EvaluatedData.Attribute == GetHealthAttribute())
	{
		SetHealth(FMath::Clamp(GetHealth(), 0.f, GetMaxHealth()));
		BroadcastOutOfHealthIfNeeded(Data, 0.f);
	}
	else if (Data.EvaluatedData.Attribute == GetResourceAttribute())
	{
		SetResource(FMath::Clamp(GetResource(), 0.f, GetMaxResource()));
	}
	else if (Data.EvaluatedData.Attribute == GetStaminaAttribute())
	{
		SetStamina(FMath::Clamp(GetStamina(), 0.f, GetMaxStamina()));
	}
}

void UDSAttributeSet::HandleIncomingDamage(const FGameplayEffectModCallbackData& Data)
{
	const float Incoming = GetIncomingDamage();
	SetIncomingDamage(0.f);
	if (Incoming <= 0.f || bOutOfHealth)
	{
		return;
	}

	// Escudo absorve primeiro (BasePlayer.gd → take_damage).
	const float FromShield = FMath::Min(GetShield(), Incoming);
	SetShield(GetShield() - FromShield);
	const float ToHealth = Incoming - FromShield;
	SetHealth(FMath::Clamp(GetHealth() - ToHealth, 0.f, GetMaxHealth()));

	const FGameplayEffectContextHandle& Context = Data.EffectSpec.GetEffectContext();
	OnDamageTaken.Broadcast(Context.GetOriginalInstigator(), Context.GetEffectCauser(), &Data.EffectSpec, Incoming);

	ApplyLifesteal(Data, Incoming);
	BroadcastOutOfHealthIfNeeded(Data, Incoming);
}

void UDSAttributeSet::HandleIncomingHeal(const FGameplayEffectModCallbackData& Data)
{
	const float Heal = GetIncomingHeal();
	SetIncomingHeal(0.f);
	if (Heal <= 0.f || bOutOfHealth)
	{
		return; // abatido não recebe cura (só revive)
	}
	SetHealth(FMath::Clamp(GetHealth() + Heal, 0.f, GetMaxHealth()));
}

void UDSAttributeSet::ApplyLifesteal(const FGameplayEffectModCallbackData& Data, float DamageDealt) const
{
	UAbilitySystemComponent* SourceASC = Data.EffectSpec.GetEffectContext().GetOriginalInstigatorAbilitySystemComponent();
	if (!SourceASC || SourceASC == GetOwningAbilitySystemComponent())
	{
		return;
	}
	const float Lifesteal = SourceASC->GetNumericAttribute(GetLifestealAttribute());
	if (Lifesteal > 0.f)
	{
		SourceASC->ApplyModToAttribute(GetIncomingHealAttribute(), EGameplayModOp::Additive, DamageDealt * Lifesteal);
	}
}

void UDSAttributeSet::BroadcastOutOfHealthIfNeeded(const FGameplayEffectModCallbackData& Data, float DamageDealt)
{
	if (bOutOfHealth || GetHealth() > 0.f)
	{
		return;
	}
	bOutOfHealth = true;
	const FGameplayEffectContextHandle& Context = Data.EffectSpec.GetEffectContext();
	OnOutOfHealth.Broadcast(Context.GetOriginalInstigator(), Context.GetEffectCauser(), &Data.EffectSpec, DamageDealt);
}

// ---------------------------------------------------------------------------
// OnRep
// ---------------------------------------------------------------------------

#define DS_IMPLEMENT_ONREP(Prop) \
	void UDSAttributeSet::OnRep_##Prop(const FGameplayAttributeData& OldValue) \
	{ \
		GAMEPLAYATTRIBUTE_REPNOTIFY(UDSAttributeSet, Prop, OldValue); \
	}

DS_IMPLEMENT_ONREP(Health)
DS_IMPLEMENT_ONREP(MaxHealth)
DS_IMPLEMENT_ONREP(Shield)
DS_IMPLEMENT_ONREP(Resource)
DS_IMPLEMENT_ONREP(MaxResource)
DS_IMPLEMENT_ONREP(Stamina)
DS_IMPLEMENT_ONREP(MaxStamina)
DS_IMPLEMENT_ONREP(Damage)
DS_IMPLEMENT_ONREP(AttackSpeed)
DS_IMPLEMENT_ONREP(AttackRange)
DS_IMPLEMENT_ONREP(CritChance)
DS_IMPLEMENT_ONREP(CritDamage)
DS_IMPLEMENT_ONREP(CooldownReduction)
DS_IMPLEMENT_ONREP(Lifesteal)
DS_IMPLEMENT_ONREP(Armor)
DS_IMPLEMENT_ONREP(DamageReduction)
DS_IMPLEMENT_ONREP(DodgeChance)
DS_IMPLEMENT_ONREP(HealthRegen)
DS_IMPLEMENT_ONREP(ResourceRegen)
DS_IMPLEMENT_ONREP(MoveSpeed)

#undef DS_IMPLEMENT_ONREP
