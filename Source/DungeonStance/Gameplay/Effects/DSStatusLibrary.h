#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "GameplayTagContainer.h"
#include "ActiveGameplayEffectHandle.h"
#include "DSStatusLibrary.generated.h"

class UAbilitySystemComponent;

UENUM(BlueprintType)
enum class EDSDotKind : uint8
{
	Bleed,
	Burn,
	Poison
};

/**
 * Aplicação e consulta de status (port de status.ts). Só servidor para aplicar.
 * Source = ASC de quem aplica (crédito do dano / provocação), Target = ASC do alvo.
 */
UCLASS()
class DUNGEONSTANCE_API UDSStatusLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "DS|Status")
	static FActiveGameplayEffectHandle ApplyMark(UAbilitySystemComponent* Source, UAbilitySystemComponent* Target, float Vulnerability, float Duration = 4.f);

	/** Adiciona um acúmulo de DoT que causa TotalDamage ao longo de Duration (status.ts addDot). */
	UFUNCTION(BlueprintCallable, Category = "DS|Status")
	static FActiveGameplayEffectHandle ApplyDot(UAbilitySystemComponent* Source, UAbilitySystemComponent* Target, EDSDotKind Kind, float TotalDamage, float Duration);

	/** Chefes recebem metade; teto de 70%. */
	UFUNCTION(BlueprintCallable, Category = "DS|Status")
	static FActiveGameplayEffectHandle ApplySlow(UAbilitySystemComponent* Source, UAbilitySystemComponent* Target, float Pct, float Duration);

	/** Máx. 1,5 s; chefes recebem lentidão de 30% em vez de atordoamento. */
	UFUNCTION(BlueprintCallable, Category = "DS|Status")
	static FActiveGameplayEffectHandle ApplyStun(UAbilitySystemComponent* Source, UAbilitySystemComponent* Target, float Duration);

	UFUNCTION(BlueprintCallable, Category = "DS|Status")
	static FActiveGameplayEffectHandle ApplyWeaken(UAbilitySystemComponent* Source, UAbilitySystemComponent* Target, float Pct, float Duration);

	UFUNCTION(BlueprintCallable, Category = "DS|Status")
	static FActiveGameplayEffectHandle ApplyExposed(UAbilitySystemComponent* Source, UAbilitySystemComponent* Target, float Vulnerability, float Duration);

	/** A IA do alvo passa a mirar o avatar de Source. */
	UFUNCTION(BlueprintCallable, Category = "DS|Status")
	static FActiveGameplayEffectHandle ApplyTaunt(UAbilitySystemComponent* Source, UAbilitySystemComponent* Target, float Duration);

	/** Maior Data.Magnitude entre os efeitos ativos que concedem StatusTag (0 se nenhum). */
	UFUNCTION(BlueprintPure, Category = "DS|Status")
	static float GetStrongestStatusMagnitude(const UAbilitySystemComponent* ASC, FGameplayTag StatusTag);

	/** Multiplicador de velocidade: 0 atordoado, senão 1 − maior lentidão. */
	UFUNCTION(BlueprintPure, Category = "DS|Status")
	static float GetSpeedMultiplier(const UAbilitySystemComponent* ASC);

	/** Avatar de quem provocou (efeito Status.Taunt mais recente), ou nulo. */
	UFUNCTION(BlueprintPure, Category = "DS|Status")
	static AActor* GetTauntInstigator(const UAbilitySystemComponent* ASC);

	static bool IsBoss(const UAbilitySystemComponent* ASC);

private:
	static FActiveGameplayEffectHandle ApplyStatusEffect(UAbilitySystemComponent* Source, UAbilitySystemComponent* Target,
		TSubclassOf<class UGameplayEffect> EffectClass, float Duration, float Magnitude, float DamagePerTick = 0.f);
};
