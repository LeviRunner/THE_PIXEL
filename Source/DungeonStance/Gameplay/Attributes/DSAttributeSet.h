#pragma once

#include "CoreMinimal.h"
#include "AttributeSet.h"
#include "AbilitySystemComponent.h"
#include "DSAttributeSet.generated.h"

#define DS_ATTRIBUTE_ACCESSORS(ClassName, PropertyName) \
	GAMEPLAYATTRIBUTE_PROPERTY_GETTER(ClassName, PropertyName) \
	GAMEPLAYATTRIBUTE_VALUE_GETTER(PropertyName) \
	GAMEPLAYATTRIBUTE_VALUE_SETTER(PropertyName) \
	GAMEPLAYATTRIBUTE_VALUE_INITTER(PropertyName)

#ifndef ATTRIBUTE_ACCESSORS
#define ATTRIBUTE_ACCESSORS(ClassName, PropertyName) DS_ATTRIBUTE_ACCESSORS(ClassName, PropertyName)
#endif

struct FGameplayEffectSpec;

/**
 * Evento de vida/dano disparado pelo AttributeSet (só no servidor).
 * Instigator = quem causou (controller/pawn do atacante), Causer = ator que aplicou (projétil, pawn).
 */
DECLARE_MULTICAST_DELEGATE_FourParams(FDSAttributeEvent, AActor* /*Instigator*/, AActor* /*Causer*/, const FGameplayEffectSpec* /*Spec*/, float /*Magnitude*/);

/**
 * Atributos de heróis e inimigos (classes.ts BaseStats + estamina do sprint).
 * Unidades: distâncias em cm, velocidades em cm/s, frações em 0..1.
 *
 * Fluxo de dano: GE de dano (UDSDamageExecution) → meta IncomingDamage →
 * PostGameplayEffectExecute consome Shield, depois Health. Health 0 → OnOutOfHealth.
 */
UCLASS()
class DUNGEONSTANCE_API UDSAttributeSet : public UAttributeSet
{
	GENERATED_BODY()

public:
	UDSAttributeSet();

	//~ UAttributeSet
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue) override;
	virtual void PreAttributeBaseChange(const FGameplayAttribute& Attribute, float& NewValue) const override;
	virtual void PostAttributeChange(const FGameplayAttribute& Attribute, float OldValue, float NewValue) override;
	virtual bool PreGameplayEffectExecute(FGameplayEffectModCallbackData& Data) override;
	virtual void PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data) override;
	//~ End UAttributeSet

	/** Vida chegou a 0 (dispara uma vez até a vida voltar a subir). */
	mutable FDSAttributeEvent OnOutOfHealth;
	/** Dano efetivo recebido (depois do escudo), para ganho de recurso, números de dano etc. */
	mutable FDSAttributeEvent OnDamageTaken;

	// --- Vitais ---
	UPROPERTY(BlueprintReadOnly, Category = "DS|Vital", ReplicatedUsing = OnRep_Health)
	FGameplayAttributeData Health;
	ATTRIBUTE_ACCESSORS(UDSAttributeSet, Health)

	UPROPERTY(BlueprintReadOnly, Category = "DS|Vital", ReplicatedUsing = OnRep_MaxHealth)
	FGameplayAttributeData MaxHealth;
	ATTRIBUTE_ACCESSORS(UDSAttributeSet, MaxHealth)

	UPROPERTY(BlueprintReadOnly, Category = "DS|Vital", ReplicatedUsing = OnRep_Shield)
	FGameplayAttributeData Shield;
	ATTRIBUTE_ACCESSORS(UDSAttributeSet, Shield)

	UPROPERTY(BlueprintReadOnly, Category = "DS|Vital", ReplicatedUsing = OnRep_Resource)
	FGameplayAttributeData Resource;
	ATTRIBUTE_ACCESSORS(UDSAttributeSet, Resource)

	UPROPERTY(BlueprintReadOnly, Category = "DS|Vital", ReplicatedUsing = OnRep_MaxResource)
	FGameplayAttributeData MaxResource;
	ATTRIBUTE_ACCESSORS(UDSAttributeSet, MaxResource)

	UPROPERTY(BlueprintReadOnly, Category = "DS|Vital", ReplicatedUsing = OnRep_Stamina)
	FGameplayAttributeData Stamina;
	ATTRIBUTE_ACCESSORS(UDSAttributeSet, Stamina)

	UPROPERTY(BlueprintReadOnly, Category = "DS|Vital", ReplicatedUsing = OnRep_MaxStamina)
	FGameplayAttributeData MaxStamina;
	ATTRIBUTE_ACCESSORS(UDSAttributeSet, MaxStamina)

	// --- Ofensivos ---
	UPROPERTY(BlueprintReadOnly, Category = "DS|Offense", ReplicatedUsing = OnRep_Damage)
	FGameplayAttributeData Damage;
	ATTRIBUTE_ACCESSORS(UDSAttributeSet, Damage)

	/** Ataques por segundo. */
	UPROPERTY(BlueprintReadOnly, Category = "DS|Offense", ReplicatedUsing = OnRep_AttackSpeed)
	FGameplayAttributeData AttackSpeed;
	ATTRIBUTE_ACCESSORS(UDSAttributeSet, AttackSpeed)

	/** Alcance do ataque básico em cm. */
	UPROPERTY(BlueprintReadOnly, Category = "DS|Offense", ReplicatedUsing = OnRep_AttackRange)
	FGameplayAttributeData AttackRange;
	ATTRIBUTE_ACCESSORS(UDSAttributeSet, AttackRange)

	UPROPERTY(BlueprintReadOnly, Category = "DS|Offense", ReplicatedUsing = OnRep_CritChance)
	FGameplayAttributeData CritChance;
	ATTRIBUTE_ACCESSORS(UDSAttributeSet, CritChance)

	/** Multiplicador do crítico (1,5 = +50%). */
	UPROPERTY(BlueprintReadOnly, Category = "DS|Offense", ReplicatedUsing = OnRep_CritDamage)
	FGameplayAttributeData CritDamage;
	ATTRIBUTE_ACCESSORS(UDSAttributeSet, CritDamage)

	UPROPERTY(BlueprintReadOnly, Category = "DS|Offense", ReplicatedUsing = OnRep_CooldownReduction)
	FGameplayAttributeData CooldownReduction;
	ATTRIBUTE_ACCESSORS(UDSAttributeSet, CooldownReduction)

	UPROPERTY(BlueprintReadOnly, Category = "DS|Offense", ReplicatedUsing = OnRep_Lifesteal)
	FGameplayAttributeData Lifesteal;
	ATTRIBUTE_ACCESSORS(UDSAttributeSet, Lifesteal)

	// --- Defensivos / mobilidade ---
	UPROPERTY(BlueprintReadOnly, Category = "DS|Defense", ReplicatedUsing = OnRep_Armor)
	FGameplayAttributeData Armor;
	ATTRIBUTE_ACCESSORS(UDSAttributeSet, Armor)

	/** Redução de dano recebido (fração, máx. MaxDamageReduction). */
	UPROPERTY(BlueprintReadOnly, Category = "DS|Defense", ReplicatedUsing = OnRep_DamageReduction)
	FGameplayAttributeData DamageReduction;
	ATTRIBUTE_ACCESSORS(UDSAttributeSet, DamageReduction)

	/** Chance de evitar o golpe (Caçador de Demônios). Máx. 75% como no protótipo. */
	UPROPERTY(BlueprintReadOnly, Category = "DS|Defense", ReplicatedUsing = OnRep_DodgeChance)
	FGameplayAttributeData DodgeChance;
	ATTRIBUTE_ACCESSORS(UDSAttributeSet, DodgeChance)

	/** Vida por segundo. */
	UPROPERTY(BlueprintReadOnly, Category = "DS|Defense", ReplicatedUsing = OnRep_HealthRegen)
	FGameplayAttributeData HealthRegen;
	ATTRIBUTE_ACCESSORS(UDSAttributeSet, HealthRegen)

	/** Recurso por segundo (negativo = escoa, ex.: Fúria do Bárbaro). */
	UPROPERTY(BlueprintReadOnly, Category = "DS|Defense", ReplicatedUsing = OnRep_ResourceRegen)
	FGameplayAttributeData ResourceRegen;
	ATTRIBUTE_ACCESSORS(UDSAttributeSet, ResourceRegen)

	/** Velocidade de movimento em cm/s (o personagem aplica no CharacterMovement). */
	UPROPERTY(BlueprintReadOnly, Category = "DS|Defense", ReplicatedUsing = OnRep_MoveSpeed)
	FGameplayAttributeData MoveSpeed;
	ATTRIBUTE_ACCESSORS(UDSAttributeSet, MoveSpeed)

	// --- Meta (só servidor, não replicados) ---
	UPROPERTY(BlueprintReadOnly, Category = "DS|Meta")
	FGameplayAttributeData IncomingDamage;
	ATTRIBUTE_ACCESSORS(UDSAttributeSet, IncomingDamage)

	UPROPERTY(BlueprintReadOnly, Category = "DS|Meta")
	FGameplayAttributeData IncomingHeal;
	ATTRIBUTE_ACCESSORS(UDSAttributeSet, IncomingHeal)

	/** Limites do protótipo (classes.ts): CDR máx. 60%, esquiva passiva máx. 75%. */
	static constexpr float MaxCooldownReduction = 0.6f;
	static constexpr float MaxDodgeChance = 0.75f;
	static constexpr float MaxDamageReduction = 0.9f;

protected:
	UFUNCTION() void OnRep_Health(const FGameplayAttributeData& OldValue);
	UFUNCTION() void OnRep_MaxHealth(const FGameplayAttributeData& OldValue);
	UFUNCTION() void OnRep_Shield(const FGameplayAttributeData& OldValue);
	UFUNCTION() void OnRep_Resource(const FGameplayAttributeData& OldValue);
	UFUNCTION() void OnRep_MaxResource(const FGameplayAttributeData& OldValue);
	UFUNCTION() void OnRep_Stamina(const FGameplayAttributeData& OldValue);
	UFUNCTION() void OnRep_MaxStamina(const FGameplayAttributeData& OldValue);
	UFUNCTION() void OnRep_Damage(const FGameplayAttributeData& OldValue);
	UFUNCTION() void OnRep_AttackSpeed(const FGameplayAttributeData& OldValue);
	UFUNCTION() void OnRep_AttackRange(const FGameplayAttributeData& OldValue);
	UFUNCTION() void OnRep_CritChance(const FGameplayAttributeData& OldValue);
	UFUNCTION() void OnRep_CritDamage(const FGameplayAttributeData& OldValue);
	UFUNCTION() void OnRep_CooldownReduction(const FGameplayAttributeData& OldValue);
	UFUNCTION() void OnRep_Lifesteal(const FGameplayAttributeData& OldValue);
	UFUNCTION() void OnRep_Armor(const FGameplayAttributeData& OldValue);
	UFUNCTION() void OnRep_DamageReduction(const FGameplayAttributeData& OldValue);
	UFUNCTION() void OnRep_DodgeChance(const FGameplayAttributeData& OldValue);
	UFUNCTION() void OnRep_HealthRegen(const FGameplayAttributeData& OldValue);
	UFUNCTION() void OnRep_ResourceRegen(const FGameplayAttributeData& OldValue);
	UFUNCTION() void OnRep_MoveSpeed(const FGameplayAttributeData& OldValue);

private:
	void ClampAttribute(const FGameplayAttribute& Attribute, float& NewValue) const;
	void HandleIncomingDamage(const FGameplayEffectModCallbackData& Data);
	void HandleIncomingHeal(const FGameplayEffectModCallbackData& Data);
	void ApplyLifesteal(const FGameplayEffectModCallbackData& Data, float DamageDealt) const;
	void BroadcastOutOfHealthIfNeeded(const FGameplayEffectModCallbackData& Data, float DamageDealt);

	/** Evita disparar OnOutOfHealth várias vezes. */
	bool bOutOfHealth = false;
};
