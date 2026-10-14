#pragma once

#include "CoreMinimal.h"
#include "GameplayEffectExecutionCalculation.h"
#include "DSDamageExecution.generated.h"

/**
 * Cálculo de dano (servidor):
 *   base   = Data.Damage (fixo, inimigos/DoT) ou Dano do atacante × Data.Magnitude (habilidade)
 *   × crítico (CritChance/CritDamage do atacante; Data.Damage fixo não crita)
 *   × (1 − Enfraquecido do atacante) × (1 + Marca) × (1 + Exposto) do alvo
 *   × mitigação de armadura (100/(100+A) herói, 200/(200+A) monstro)
 *   × (1 − DamageReduction do alvo); DodgeChance do alvo pode zerar.
 * Saída: meta IncomingDamage do alvo. Fórmulas em Combat/DSDamageMath.h.
 */
UCLASS()
class DUNGEONSTANCE_API UDSDamageExecution : public UGameplayEffectExecutionCalculation
{
	GENERATED_BODY()

public:
	UDSDamageExecution();

	virtual void Execute_Implementation(const FGameplayEffectCustomExecutionParameters& ExecutionParams,
		FGameplayEffectCustomExecutionOutput& OutExecutionOutput) const override;
};
