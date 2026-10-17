#pragma once

#include "CoreMinimal.h"
#include "Gameplay/Abilities/DSGameplayAbility.h"
#include "Gameplay/Abilities/DSSupportResolver.h"
#include "DSGA_SupportAura.generated.h"

class UDSSupportAbilityData;

/**
 * Habilidade de apoio genérica (Q/E/R/F), parametrizada por UDSSupportAbilityData.
 * Cobre as 8 habilidades de abilities.ts: Aura do Guerreiro Perdido, Bastião Compartilhado,
 * Pulso Revigorante, Grito de Batalha, Estandarte de Guerra, Santuário, Totem Guardião, Chamado Tático.
 * Números finais = data asset + cartas do lançador (DSSupportResolver). Efeitos só no servidor,
 * em todos os aliados (incluindo o lançador) dentro do raio.
 */
UCLASS()
class DUNGEONSTANCE_API UDSGA_SupportAura : public UDSGameplayAbility
{
	GENERATED_BODY()

public:
	UDSGA_SupportAura(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	virtual float GetCooldownDuration() const override;
	virtual FGameplayTagContainer GetAbilityCooldownTags() const override;

	const UDSSupportAbilityData* GetSupportData() const { return SupportData; }

protected:
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;

	UPROPERTY(EditDefaultsOnly, Category = "DS|Support")
	TObjectPtr<UDSSupportAbilityData> SupportData;

private:
	FDSResolvedSupport ResolveForCaster() const;
	void ApplyToAllies(const FDSResolvedSupport& Resolved);
	void ApplyToAlly(ADSCharacterBase* Ally, const FDSResolvedSupport& Resolved) const;
	void SpawnZone(const FDSResolvedSupport& Resolved) const;
};
