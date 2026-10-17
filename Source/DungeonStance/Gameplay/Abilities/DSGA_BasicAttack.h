#pragma once

#include "CoreMinimal.h"
#include "Gameplay/Abilities/DSGameplayAbility.h"
#include "Gameplay/Data/DSClassData.h"
#include "DSGA_BasicAttack.generated.h"

class UAnimMontage;

/**
 * Ataque rápido (segurar botão esquerdo / RT). A forma vem do UDSClassData (BasicAttack):
 *  Melee         — arco de 110°, acerta só o mais próximo
 *  MeleeAoE      — mesmo arco, acerta todos
 *  Projectile    — projétil na direção da mira, alcance da classe
 *  ProjectileAoE — explode ao acertar, no terreno ou no fim do alcance
 * Cadência: recarga = 1 / AttackSpeed (WhileInputActive reativa enquanto segurado).
 */
UCLASS()
class DUNGEONSTANCE_API UDSGA_BasicAttack : public UDSGameplayAbility
{
	GENERATED_BODY()

public:
	UDSGA_BasicAttack(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	virtual float GetCooldownDuration() const override;

protected:
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;

	/** Usada quando o herói não tem UDSClassData (inimigos/testes). */
	UPROPERTY(EditDefaultsOnly, Category = "DS|Attack")
	FDSBasicAttackInfo FallbackAttackInfo;

	UPROPERTY(EditDefaultsOnly, Category = "DS|Attack")
	TObjectPtr<UAnimMontage> AttackMontage;

	/** Montagem toca na velocidade de ataque (AttackSpeed × este fator). */
	UPROPERTY(EditDefaultsOnly, Category = "DS|Attack")
	float MontageRateScale = 1.f;

private:
	const FDSBasicAttackInfo& GetAttackInfo() const;
	float NextComboMultiplier();
	void PerformMelee(const FDSBasicAttackInfo& Info, bool bHitAll);
	void PerformProjectile(const FDSBasicAttackInfo& Info, bool bAoE);
	void FaceAim() const;

	/** Contador de combo do Monge (só servidor). */
	int32 ComboCounter = 0;
};
