#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "AbilitySystemInterface.h"
#include "GameplayTagContainer.h"
#include "DSCharacterBase.generated.h"

class UDSAbilitySystemComponent;
class UDSAttributeSet;
class UGameplayEffect;
struct FGameplayEffectSpec;
struct FOnAttributeChangeData;

class ADSCharacterBase;
/** Morte/abate do personagem (servidor). Killer = controller de quem deu o golpe final (pode ser nulo). */
DECLARE_MULTICAST_DELEGATE_TwoParams(FDSOnDeath, ADSCharacterBase* /*Character*/, AController* /*Killer*/);

/**
 * Base de heróis e inimigos: ponte com GAS, morte, velocidade a partir de atributos/status
 * e consultas de time usadas pelo targeting.
 */
UCLASS(Abstract)
class DUNGEONSTANCE_API ADSCharacterBase : public ACharacter, public IAbilitySystemInterface
{
	GENERATED_BODY()

public:
	ADSCharacterBase(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	//~ IAbilitySystemInterface
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;
	//~ End

	UDSAbilitySystemComponent* GetDSAbilitySystemComponent() const { return AbilitySystemComponent; }
	UDSAttributeSet* GetAttributeSet() const { return AttributeSet; }

	UFUNCTION(BlueprintPure, Category = "DS|Character")
	virtual bool IsAlive() const;

	/** Heróis = true, inimigos = false. */
	UFUNCTION(BlueprintPure, Category = "DS|Character")
	virtual bool IsHeroTeam() const { return false; }

	bool IsHostileTo(const ADSCharacterBase* Other) const;

	/** Direção de mira no plano (lock-on > controle > frente do ator). */
	UFUNCTION(BlueprintPure, Category = "DS|Character")
	virtual FVector GetAimDirection() const;

	/** Ponto do corpo para mirar (centro da cápsula). */
	FVector GetAimPoint() const;

	/** Recalcula MaxWalkSpeed = MoveSpeed × lentidão/atordoamento × sprint. */
	void RefreshMovementSpeed();

	/** Delegate nativo de morte/abate (contrato). */
	FDSOnDeath OnDeath;

protected:
	/** Chamado depois que ASC e AttributeSet estão definidos e o ActorInfo foi inicializado. */
	virtual void OnAbilitySystemInitialized();
	/** Vida chegou a 0 no servidor. */
	virtual void HandleOutOfHealth(AActor* DamageInstigator, AActor* DamageCauser, const FGameplayEffectSpec* Spec, float Magnitude);
	/** Dano efetivo recebido (servidor). */
	virtual void HandleDamageTaken(AActor* DamageInstigator, AActor* DamageCauser, const FGameplayEffectSpec* Spec, float Magnitude) {}
	/** Fator extra de velocidade (sprint do herói). */
	virtual float GetExtraSpeedMultiplier() const { return 1.f; }

	/** Mata o personagem (servidor): State.Dead, cancela habilidades, desativa colisão e dispara OnDeath. */
	virtual void Die(AController* Killer);

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_OnDied();
	/** Gancho visual (ragdoll, animação, VFX) em todas as máquinas. */
	UFUNCTION(BlueprintImplementableEvent, Category = "DS|Character", meta = (DisplayName = "On Died"))
	void K2_OnDied();

	static AController* ResolveKillerController(AActor* DamageInstigator);

	/** ASC e AttributeSet em uso (do PlayerState nos heróis, próprios nos inimigos). Não são donos. */
	UPROPERTY(Transient)
	TObjectPtr<UDSAbilitySystemComponent> AbilitySystemComponent;

	UPROPERTY(Transient)
	TObjectPtr<UDSAttributeSet> AttributeSet;

	/** Atraso para destruir o corpo após a morte (s). 0 = não destrói. */
	UPROPERTY(EditDefaultsOnly, Category = "DS|Character")
	float CorpseLifeSpan = 5.f;

	bool bDied = false;

private:
	void BindAttributeCallbacks();
	void OnMoveSpeedChanged(const FOnAttributeChangeData& Data);
	void OnSpeedTagChanged(const FGameplayTag Tag, int32 NewCount);

	FDelegateHandle OutOfHealthHandle;
	FDelegateHandle DamageTakenHandle;
};
