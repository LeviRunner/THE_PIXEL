#pragma once

#include "CoreMinimal.h"
#include "Gameplay/Characters/DSCharacterBase.h"
#include "GameplayTagContainer.h"
#include "DSHeroCharacter.generated.h"

class ADSPlayerState;
class UCameraComponent;
class UDSClassData;
class UDSStanceComponent;
class UDSTargetLockComponent;
class UInputAction;
class UInputMappingContext;
class USpringArmComponent;
struct FInputActionValue;

/** Ação de input → tag de habilidade (Input.Attack, Input.Dodge, Input.Support.1...). */
USTRUCT(BlueprintType)
struct FDSInputAbilityBinding
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	TObjectPtr<UInputAction> Action;

	UPROPERTY(EditDefaultsOnly, Category = "Input", meta = (Categories = "Input"))
	FGameplayTag InputTag;
};

/**
 * Herói jogável. Um Blueprint por classe (BP_Hero_Barbarian...) com o UDSClassData correspondente;
 * o ADSGameMode escolhe o pawn por EDSHeroClass (HeroPawnClasses).
 * ASC/AttributeSet vêm do ADSPlayerState (PossessedBy no servidor, OnRep_PlayerState no cliente).
 * Abatido (vida 0) → State.Downed; levanta quando a vida volta a subir (ReviveHero do Core) ou via Revive().
 */
UCLASS()
class DUNGEONSTANCE_API ADSHeroCharacter : public ADSCharacterBase
{
	GENERATED_BODY()

public:
	ADSHeroCharacter(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	//~ APawn / AActor
	virtual void PossessedBy(AController* NewController) override;
	virtual void OnRep_PlayerState() override;
	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;
	virtual void NotifyControllerChanged() override;
	virtual void Tick(float DeltaSeconds) override;
	//~ End

	virtual bool IsHeroTeam() const override { return true; }
	virtual FVector GetAimDirection() const override;

	UFUNCTION(BlueprintPure, Category = "DS|Hero")
	UDSTargetLockComponent* GetTargetLock() const { return TargetLock; }

	UFUNCTION(BlueprintPure, Category = "DS|Hero")
	UDSStanceComponent* GetStanceComponent() const { return StanceComponent; }

	UFUNCTION(BlueprintPure, Category = "DS|Hero")
	const UDSClassData* GetClassData() const { return ClassData; }

	UFUNCTION(BlueprintPure, Category = "DS|Hero")
	bool IsDowned() const;

	/** Levanta o herói com uma fração da vida máxima (servidor). */
	UFUNCTION(BlueprintCallable, Category = "DS|Hero")
	void Revive(float HealthRatio = 1.f);

protected:
	virtual void HandleOutOfHealth(AActor* DamageInstigator, AActor* DamageCauser, const FGameplayEffectSpec* Spec, float Magnitude) override;
	virtual void HandleDamageTaken(AActor* DamageInstigator, AActor* DamageCauser, const FGameplayEffectSpec* Spec, float Magnitude) override;
	virtual float GetExtraSpeedMultiplier() const override;
	virtual void OnAbilitySystemInitialized() override;

	UFUNCTION(BlueprintImplementableEvent, Category = "DS|Hero", meta = (DisplayName = "On Downed Changed"))
	void K2_OnDownedChanged(bool bIsDowned);

	// --- Componentes ---
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "DS|Hero")
	TObjectPtr<USpringArmComponent> CameraBoom;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "DS|Hero")
	TObjectPtr<UCameraComponent> FollowCamera;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "DS|Hero")
	TObjectPtr<UDSTargetLockComponent> TargetLock;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "DS|Hero")
	TObjectPtr<UDSStanceComponent> StanceComponent;

	// --- Dados ---
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "DS|Hero")
	TObjectPtr<UDSClassData> ClassData;

	/** Multiplicador de velocidade com State.Sprinting (squad_logic.ts SPRINT_SPEED_MULT). */
	UPROPERTY(EditDefaultsOnly, Category = "DS|Hero")
	float SprintSpeedMultiplier = 1.6f;

	/** Efeitos sempre ativos (regeneração de vida/recurso/estamina). Padrão: UDSGE_Regen + UDSGE_StaminaRegen. */
	UPROPERTY(EditDefaultsOnly, Category = "DS|Hero")
	TArray<TSubclassOf<UGameplayEffect>> PassiveEffects;

	// --- Enhanced Input ---
	UPROPERTY(EditDefaultsOnly, Category = "DS|Input")
	TObjectPtr<UInputMappingContext> DefaultMappingContext;

	UPROPERTY(EditDefaultsOnly, Category = "DS|Input")
	int32 MappingPriority = 0;

	UPROPERTY(EditDefaultsOnly, Category = "DS|Input")
	TObjectPtr<UInputAction> MoveAction;

	UPROPERTY(EditDefaultsOnly, Category = "DS|Input")
	TObjectPtr<UInputAction> LookAction;

	UPROPERTY(EditDefaultsOnly, Category = "DS|Input")
	TObjectPtr<UInputAction> JumpAction;

	/** Clique do scroll / R3 (Input.TargetLock). */
	UPROPERTY(EditDefaultsOnly, Category = "DS|Input")
	TObjectPtr<UInputAction> TargetLockAction;

	/** Scroll do mouse / analógico direito X (Axis1D). */
	UPROPERTY(EditDefaultsOnly, Category = "DS|Input")
	TObjectPtr<UInputAction> TargetSwitchAction;

	/** Ataque, skill, esquiva, sprint, apoio 1–4. */
	UPROPERTY(EditDefaultsOnly, Category = "DS|Input", meta = (TitleProperty = "InputTag"))
	TArray<FDSInputAbilityBinding> AbilityInputBindings;

private:
	void InitAbilitySystemFromPlayerState();
	void InitializeServerState(ADSPlayerState* DSPlayerState);
	void ApplyPassiveEffects();
	void SetDowned(bool bNewDowned);
	void OnHealthChanged(const FOnAttributeChangeData& Data);
	void OnDownedTagChanged(const FGameplayTag Tag, int32 NewCount);

	void Input_Move(const FInputActionValue& Value);
	void Input_Look(const FInputActionValue& Value);
	void Input_TargetLock(const FInputActionValue& Value);
	void Input_TargetSwitch(const FInputActionValue& Value);
	void Input_TargetSwitchReleased(const FInputActionValue& Value);
	void Input_AbilityPressed(FGameplayTag InputTag);
	void Input_AbilityReleased(FGameplayTag InputTag);

	/** Analógico: troca uma vez por "empurrão". */
	bool bTargetSwitchLatched = false;
	bool bDowned = false;
	FDelegateHandle HealthChangedHandle;
};
