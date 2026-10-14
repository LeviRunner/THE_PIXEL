#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemComponent.h"
#include "Gameplay/Abilities/DSAbilitySet.h"
#include "DSAbilitySystemComponent.generated.h"

class UDSAbilitySet;

/**
 * ASC do Dungeon Stance (heróis: no ADSPlayerState, replicação Mixed; inimigos: no próprio pawn, Minimal).
 *
 * Input por tag (padrão Lyra): o pawn chama AbilityInputTagPressed/Released com DSTags::Input_*;
 * ProcessAbilityInput (chamado pelo pawn local a cada frame e/ou pelo PlayerController em PostProcessInput)
 * ativa as habilidades conforme a política de ativação de UDSGameplayAbility.
 */
UCLASS()
class DUNGEONSTANCE_API UDSAbilitySystemComponent : public UAbilitySystemComponent
{
	GENERATED_BODY()

public:
	UDSAbilitySystemComponent();

	//~ UAbilitySystemComponent
	virtual void InitAbilityActorInfo(AActor* InOwnerActor, AActor* InAvatarActor) override;
	//~ End

	void AbilityInputTagPressed(FGameplayTag InputTag);
	void AbilityInputTagReleased(FGameplayTag InputTag);

	/** Processa o input acumulado. Idempotente no mesmo frame (as listas são limpas). */
	void ProcessAbilityInput(float DeltaTime, bool bGamePaused);
	void ClearAbilityInput();

	/** Concede um set (só servidor). Não concede o mesmo set duas vezes. */
	void GrantAbilitySet(const UDSAbilitySet* AbilitySet, UObject* SourceObject = nullptr);
	/** Remove o que o set concedeu (só servidor). */
	void RemoveAbilitySet(const UDSAbilitySet* AbilitySet);
	bool HasAbilitySet(const UDSAbilitySet* AbilitySet) const;

	/** Concede uma habilidade ligada a uma tag de input (cartas de apoio). Retorna o handle. */
	FGameplayAbilitySpecHandle GrantAbilityWithInputTag(TSubclassOf<UGameplayAbility> AbilityClass, FGameplayTag InputTag, int32 Level = 1, UObject* SourceObject = nullptr);

	/** Primeira tag Input.Support.N livre (até DSConstants::MaxSupportAbilitySlots). Inválida se cheia. */
	FGameplayTag FindFreeSupportInputTag() const;

	/** Remove efeitos de recarga com as tags dadas (Chamado Tático reseta a esquiva). */
	void ResetCooldownsWithTags(const FGameplayTagContainer& CooldownTags);

	/** Adiciona/remove tag solta local + replicada (State.Dead etc.). Só servidor. */
	void SetReplicatedLooseTag(FGameplayTag Tag, bool bPresent);

	/** Tenta ativar habilidades com política OnSpawn (chamado após InitAbilityActorInfo). */
	void TryActivateAbilitiesOnSpawn();

protected:
	virtual void AbilitySpecInputPressed(FGameplayAbilitySpec& Spec) override;
	virtual void AbilitySpecInputReleased(FGameplayAbilitySpec& Spec) override;

private:
	void CollectSpecsForInputTag(FGameplayTag InputTag, TArray<FGameplayAbilitySpecHandle>& OutHandles) const;
	void ActivatePressedAndHeld(TArray<FGameplayAbilitySpecHandle>& ToActivate);

	TArray<FGameplayAbilitySpecHandle> InputPressedSpecHandles;
	TArray<FGameplayAbilitySpecHandle> InputReleasedSpecHandles;
	TArray<FGameplayAbilitySpecHandle> InputHeldSpecHandles;

	/** Sets concedidos e seus handles (servidor). */
	TMap<const UDSAbilitySet*, FDSAbilitySetGrantedHandles> GrantedSets;
};
