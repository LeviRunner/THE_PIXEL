#pragma once

#include "CoreMinimal.h"
#include "Gameplay/Abilities/DSGameplayAbility.h"
#include "DSGA_Sprint.generated.h"

struct FOnAttributeChangeData;

/**
 * Corrida (squad_logic.ts sprintStep): ativa segurando Input.Sprint; precisa de estamina mínima
 * para começar, gasta por segundo (UDSGE_SprintDrain) e termina ao soltar ou zerar.
 * Velocidade ×SpeedMultiplier enquanto State.Sprinting (ADSHeroCharacter::GetExtraSpeedMultiplier).
 * A regeneração (UDSGE_StaminaRegen) fica inibida durante o sprint.
 */
UCLASS()
class DUNGEONSTANCE_API UDSGA_Sprint : public UDSGameplayAbility
{
	GENERATED_BODY()

public:
	UDSGA_Sprint(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	virtual bool CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayTagContainer* SourceTags = nullptr, const FGameplayTagContainer* TargetTags = nullptr,
		FGameplayTagContainer* OptionalRelevantTags = nullptr) const override;

	/** Multiplicador de velocidade (lido pelo herói). */
	UPROPERTY(EditDefaultsOnly, Category = "DS|Sprint")
	float SpeedMultiplier = 1.6f;

protected:
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	virtual void InputReleased(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo) override;
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;

	/** Estamina mínima para (re)começar a correr. */
	UPROPERTY(EditDefaultsOnly, Category = "DS|Sprint")
	float MinStaminaToStart = 10.f;

	UPROPERTY(EditDefaultsOnly, Category = "DS|Sprint")
	TSubclassOf<UGameplayEffect> DrainEffectClass;

private:
	void OnStaminaChanged(const FOnAttributeChangeData& Data);

	FActiveGameplayEffectHandle DrainHandle;
	FDelegateHandle StaminaChangedHandle;
};
