#pragma once

#include "CoreMinimal.h"
#include "Gameplay/Abilities/DSGameplayAbility.h"
#include "DSGA_Dodge.generated.h"

class UAnimMontage;

/**
 * Esquiva (Shift / B): dash de 425 cm (classe), recarga 1,2 s (classe), ~0,24 s de invulnerabilidade.
 * Movimento via root motion source (predito pelo CharacterMovement). Direção = input de movimento,
 * ou mira se parado.
 */
UCLASS()
class DUNGEONSTANCE_API UDSGA_Dodge : public UDSGameplayAbility
{
	GENERATED_BODY()

public:
	UDSGA_Dodge(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	virtual float GetCooldownDuration() const override;

protected:
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;

	UFUNCTION()
	void OnDashFinished();

	/** Usados se o herói não tiver UDSClassData. */
	UPROPERTY(EditDefaultsOnly, Category = "DS|Dodge", meta = (Units = "cm"))
	float DefaultDistance = 425.f;

	UPROPERTY(EditDefaultsOnly, Category = "DS|Dodge", meta = (Units = "s"))
	float DefaultInvulnerability = 0.24f;

	/** Duração do deslocamento. */
	UPROPERTY(EditDefaultsOnly, Category = "DS|Dodge", meta = (Units = "s"))
	float DashDuration = 0.2f;

	UPROPERTY(EditDefaultsOnly, Category = "DS|Dodge")
	TObjectPtr<UAnimMontage> DodgeMontage;

private:
	FVector ComputeDashDirection() const;
};
