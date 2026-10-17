#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "ActiveGameplayEffectHandle.h"
#include "AttributeSet.h"
#include "GameplayEffectTypes.h"
#include "Gameplay/Combat/DSTargetingLibrary.h"
#include "DSGameplayAbility.generated.h"

class ADSCharacterBase;
class ADSHeroCharacter;
class UDSAbilitySystemComponent;
class UDSClassData;

/** Como o input ativa a habilidade. */
UENUM(BlueprintType)
enum class EDSAbilityActivationPolicy : uint8
{
	/** Ativa uma vez ao apertar. */
	OnInputTriggered,
	/** Tenta ativar a cada frame enquanto segurado (ataque básico contínuo). */
	WhileInputActive,
	/** Ativa sozinha quando concedida/avatar definido (passivas). */
	OnSpawn
};

/**
 * Base do Cast Framework. Toda habilidade (ataque, skill, esquiva, apoio) herda daqui.
 * - Recarga: UDSGE_Cooldown com duração por SetByCaller (GetCooldownDuration, já com redução de recarga)
 *   e as CooldownTags do asset como tags concedidas dinâmicas.
 * - Custo: Recurso via UDSGE_Cost (ResourceCost).
 * - Helpers de alvo (arco/raio/linha) e de dano, cientes da party.
 * Predição local (LocalPredicted); dano e efeitos só no servidor.
 */
UCLASS(Abstract)
class DUNGEONSTANCE_API UDSGameplayAbility : public UGameplayAbility
{
	GENERATED_BODY()

public:
	UDSGameplayAbility(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	EDSAbilityActivationPolicy GetActivationPolicy() const { return ActivationPolicy; }

	//~ UGameplayAbility
	virtual const FGameplayTagContainer* GetCooldownTags() const override;
	virtual void ApplyCooldown(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo) const override;
	virtual bool CheckCost(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, FGameplayTagContainer* OptionalRelevantTags = nullptr) const override;
	virtual void ApplyCost(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo) const override;
	virtual void OnGiveAbility(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpec& Spec) override;
	//~ End

	/** Recarga final em segundos (base × (1 − redução, limitada)). */
	UFUNCTION(BlueprintPure, Category = "DS|Ability")
	virtual float GetCooldownDuration() const;

	UFUNCTION(BlueprintPure, Category = "DS|Ability")
	virtual float GetResourceCost() const { return ResourceCost; }

	/** Tags de recarga desta habilidade (padrão: CooldownTags; apoio acrescenta a tag do data asset). */
	virtual FGameplayTagContainer GetAbilityCooldownTags() const { return CooldownTags; }

protected:
	// --- Acesso ---
	UDSAbilitySystemComponent* GetDSAbilitySystemComponent() const;
	ADSCharacterBase* GetDSCharacter() const;
	ADSHeroCharacter* GetHero() const;
	const UDSClassData* GetClassData() const;
	float GetOwnerAttribute(const FGameplayAttribute& Attribute) const;
	bool IsServer() const;

	/** Direção de mira do avatar (lock-on ou câmera). */
	FVector GetAimDirection() const;

	// --- Alvos ---
	void FindHostilesInArc(float Radius, float ArcDegrees, TArray<ADSCharacterBase*>& OutTargets) const;
	void FindAlliesInRadius(const FVector& Center, float Radius, TArray<ADSCharacterBase*>& OutAllies, bool bIncludeDowned = false) const;

	// --- Dano (servidor) ---
	/** Spec de dano: Multiplier sobre o Dano do atacante, ou FlatDamage fixo se > 0. */
	FGameplayEffectSpecHandle MakeDamageSpec(float Multiplier, float FlatDamage = 0.f) const;
	/** Aplica a todos os alvos; retorna quantos foram atingidos. Ganha recurso por acerto. */
	int32 ApplyDamageToTargets(const TArray<ADSCharacterBase*>& Targets, float Multiplier, float FlatDamage = 0.f) const;
	/** Ganho de recurso ao acertar (classe). */
	void GainResourceOnHit(int32 NumHits) const;

	/** Aplica um GE com duração em si mesmo (SetByCaller Data.Duration). */
	FActiveGameplayEffectHandle ApplyTimedEffectToOwner(TSubclassOf<UGameplayEffect> EffectClass, float Duration) const;

	UPROPERTY(EditDefaultsOnly, Category = "DS|Activation")
	EDSAbilityActivationPolicy ActivationPolicy = EDSAbilityActivationPolicy::OnInputTriggered;

	/** Recarga base (s). 0 = sem recarga. */
	UPROPERTY(EditDefaultsOnly, Category = "DS|Cooldown", meta = (Units = "s", ClampMin = 0))
	float BaseCooldown = 0.f;

	/** Tags concedidas enquanto em recarga (ex.: Cooldown.Dodge). */
	UPROPERTY(EditDefaultsOnly, Category = "DS|Cooldown", meta = (Categories = "Cooldown"))
	FGameplayTagContainer CooldownTags;

	/** Aplica o atributo CooldownReduction do dono. */
	UPROPERTY(EditDefaultsOnly, Category = "DS|Cooldown")
	bool bUseCooldownReduction = true;

	/** Teto da redução (classe 0,6; apoio 0,5). */
	UPROPERTY(EditDefaultsOnly, Category = "DS|Cooldown", meta = (ClampMin = 0, ClampMax = 1))
	float MaxCooldownReduction = 0.6f;

	/** Custo em Recurso (Fúria, Mana...). */
	UPROPERTY(EditDefaultsOnly, Category = "DS|Cost", meta = (ClampMin = 0))
	float ResourceCost = 0.f;

	/** GE de dano (padrão UDSGE_Damage). */
	UPROPERTY(EditDefaultsOnly, Category = "DS|Damage")
	TSubclassOf<UGameplayEffect> DamageEffectClass;

private:
	/** Buffer para GetCooldownTags (padrão do GAS para tags dinâmicas). */
	UPROPERTY(Transient)
	FGameplayTagContainer TempCooldownTags;
};
