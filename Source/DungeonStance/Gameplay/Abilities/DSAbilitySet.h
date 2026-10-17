#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GameplayTagContainer.h"
#include "GameplayAbilitySpecHandle.h"
#include "ActiveGameplayEffectHandle.h"
#include "AttributeSet.h"
#include "DSAbilitySet.generated.h"

class UAbilitySystemComponent;
class UAttributeSet;
class UGameplayEffect;
class UDSGameplayAbility;

/** Habilidade concedida pelo set, com a tag de input que a ativa (Input.Attack, Input.Support.1...). */
USTRUCT(BlueprintType)
struct FDSAbilitySet_Ability
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, Category = "Ability")
	TSubclassOf<UDSGameplayAbility> Ability;

	UPROPERTY(EditDefaultsOnly, Category = "Ability")
	int32 AbilityLevel = 1;

	/** Vazio = habilidade passiva/ativada por evento ou OnSpawn. */
	UPROPERTY(EditDefaultsOnly, Category = "Ability", meta = (Categories = "Input"))
	FGameplayTag InputTag;
};

USTRUCT(BlueprintType)
struct FDSAbilitySet_Effect
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, Category = "Effect")
	TSubclassOf<UGameplayEffect> Effect;

	UPROPERTY(EditDefaultsOnly, Category = "Effect")
	float EffectLevel = 1.f;
};

/** Valor inicial de um atributo (aplicado como base no servidor). */
USTRUCT(BlueprintType)
struct FDSAbilitySet_Attribute
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, Category = "Attribute")
	FGameplayAttribute Attribute;

	UPROPERTY(EditDefaultsOnly, Category = "Attribute")
	float BaseValue = 0.f;
};

/** Handles do que um set concedeu, para remover depois (troca de classe, fim de run). */
USTRUCT(BlueprintType)
struct FDSAbilitySetGrantedHandles
{
	GENERATED_BODY()

	void AddAbilitySpecHandle(const FGameplayAbilitySpecHandle& Handle);
	void AddEffectHandle(const FActiveGameplayEffectHandle& Handle);
	void TakeFromAbilitySystem(UAbilitySystemComponent* ASC);

protected:
	UPROPERTY()
	TArray<FGameplayAbilitySpecHandle> AbilitySpecHandles;

	UPROPERTY()
	TArray<FActiveGameplayEffectHandle> EffectHandles;
};

/**
 * Data asset com habilidades + efeitos iniciais + valores iniciais de atributos.
 * Ex.: DA_AbilitySet_Common (esquiva, sprint, regen), DA_AbilitySet_Barbarian (ataque, Redemoinho).
 */
UCLASS(BlueprintType, Const)
class DUNGEONSTANCE_API UDSAbilitySet : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	/** Só servidor. Retorna os handles concedidos (OutHandles pode ser nulo). */
	void GiveToAbilitySystem(UAbilitySystemComponent* ASC, FDSAbilitySetGrantedHandles* OutHandles, UObject* SourceObject = nullptr) const;

	UPROPERTY(EditDefaultsOnly, Category = "Abilities", meta = (TitleProperty = "Ability"))
	TArray<FDSAbilitySet_Ability> GrantedAbilities;

	UPROPERTY(EditDefaultsOnly, Category = "Effects", meta = (TitleProperty = "Effect"))
	TArray<FDSAbilitySet_Effect> GrantedEffects;

	UPROPERTY(EditDefaultsOnly, Category = "Attributes")
	TArray<FDSAbilitySet_Attribute> InitialAttributes;
};
