#pragma once

#include "CoreMinimal.h"
#include "GameplayEffect.h"
#include "GameplayEffectComponent.h"
#include "DSGameplayEffectBase.generated.h"

/**
 * Base dos GEs configurados em C++ (construtor). Os valores numéricos aqui são os
 * defaults do protótipo; para balancear, crie um Blueprint filho e edite no editor,
 * ou passe os valores por SetByCaller (preferível: vêm dos Data Assets).
 *
 * Helpers protegidos encapsulam a API de componentes de GE (UE 5.3+).
 */
UCLASS(Abstract)
class DUNGEONSTANCE_API UDSGameplayEffectBase : public UGameplayEffect
{
	GENERATED_BODY()

protected:
	/** Tags concedidas ao alvo enquanto o efeito está ativo (UTargetTagsGameplayEffectComponent). */
	void AddGrantedTags(const FGameplayTagContainer& Tags);
	/** Asset tags (identificam o efeito em queries). */
	void AddAssetTags(const FGameplayTagContainer& Tags);
	/** Efeito fica inibido enquanto o alvo tiver alguma destas tags. */
	void AddOngoingIgnoreTags(const FGameplayTagContainer& Tags);

	void SetDurationBySetByCaller(FGameplayTag DataTag);
	void AddSetByCallerModifier(const FGameplayAttribute& Attribute, EGameplayModOp::Type Op, FGameplayTag DataTag);
	void AddScalarModifier(const FGameplayAttribute& Attribute, EGameplayModOp::Type Op, float Value);
	/** Magnitude = Coeficiente × atributo do alvo (não-snapshot), ex.: regeneração periódica. */
	void AddTargetAttributeBasedModifier(const FGameplayAttribute& Attribute, EGameplayModOp::Type Op, const FGameplayAttribute& Backing, float Coefficient);
	/** Stacking por alvo (status.ts: acúmulos com teto; renova a duração a cada aplicação). */
	void SetTargetStacking(int32 Limit);
	/** Uma instância por fonte; reaplicar só renova a duração (zonas reaplicam a cada tique). */
	void SetSourceStacking(int32 Limit);

private:
	/** Só pode ser chamado durante o construtor (usa CreateDefaultSubobject). Reaproveita o componente do pai. */
	template <typename TComponent>
	TComponent* FindOrCreateComponent(FName Name)
	{
		for (UGameplayEffectComponent* Existing : GEComponents)
		{
			if (TComponent* Typed = Cast<TComponent>(Existing))
			{
				return Typed;
			}
		}
		TComponent* Created = CreateDefaultSubobject<TComponent>(Name);
		GEComponents.Add(Created);
		return Created;
	}
};
