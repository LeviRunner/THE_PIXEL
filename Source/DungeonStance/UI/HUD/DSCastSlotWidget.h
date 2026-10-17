#pragma once

#include "CoreMinimal.h"
#include "CommonUserWidget.h"
#include "GameplayAbilitySpec.h"
#include "GameplayTagContainer.h"
#include "DSCastSlotWidget.generated.h"

class UAbilitySystemComponent;
class UCommonActionWidget;
class UGameplayAbility;
class UImage;
class UInputAction;
class UProgressBar;
class UTextBlock;
class UWidget;

/**
 * Um slot da barra de casts (ataque, skill, esquiva, Q/E/R/F).
 * Acha a habilidade no ASC pela tag de input (DynamicSpecSourceTags do spec) ou pela
 * tag de asset da habilidade; mostra ícone, recarga restante, custo insuficiente e cargas.
 * WBP: WBP_DSCastSlot.
 */
UCLASS(Abstract)
class DUNGEONSTANCE_API UDSCastSlotWidget : public UCommonUserWidget
{
	GENERATED_BODY()

public:
	void BindAbilitySystem(UAbilitySystemComponent* InAbilitySystem);

	/** Usado pela barra quando o designer não preencheu InputTag na instância. */
	void SetDefaultInputTag(const FGameplayTag& DefaultInputTag);

	/** Atualiza recarga/custo/cargas. Chamado pela UDSCastBarWidget num timer leve. */
	void RefreshSlot();

protected:
	virtual void NativeOnInitialized() override;

	UFUNCTION(BlueprintImplementableEvent, Category = "DS|UI")
	void BP_OnCooldownFinished();

	UFUNCTION(BlueprintImplementableEvent, Category = "DS|UI")
	void BP_OnAbilityChanged(bool bHasAbility);

	/** Tag de input do slot (Input.Attack, Input.Support.1...). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "DS|Cast", meta = (Categories = "Input"))
	FGameplayTag InputTag;

	/** Alternativa: tag de asset da habilidade (Ability.Dodge...). Usada se o spec não tiver a tag de input. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "DS|Cast", meta = (Categories = "Ability"))
	FGameplayTag AbilityTag;

	/** Usada quando a habilidade não declara Cooldown GE (ex.: Cooldown.Dodge). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "DS|Cast", meta = (Categories = "Cooldown"))
	FGameplayTag FallbackCooldownTag;

	/**
	 * Opcional: tag cuja contagem no ASC representa as cargas disponíveis.
	 * TODO(Gameplay): confirmar como as cargas serão modeladas (stack de GE / tag count).
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "DS|Cast")
	FGameplayTag ChargesTag;

	/** Ação do Enhanced Input para mostrar o glifo (teclado/gamepad) via CommonUI. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "DS|Cast")
	TObjectPtr<UInputAction> InputAction;

	UPROPERTY(BlueprintReadOnly, Category = "DS|UI", meta = (BindWidget))
	TObjectPtr<UImage> AbilityIcon;

	UPROPERTY(BlueprintReadOnly, Category = "DS|UI", meta = (BindWidgetOptional))
	TObjectPtr<UProgressBar> CooldownBar;

	UPROPERTY(BlueprintReadOnly, Category = "DS|UI", meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> CooldownText;

	UPROPERTY(BlueprintReadOnly, Category = "DS|UI", meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> ChargesText;

	/** Visível quando o recurso não paga o custo. */
	UPROPERTY(BlueprintReadOnly, Category = "DS|UI", meta = (BindWidgetOptional))
	TObjectPtr<UWidget> InsufficientResourceOverlay;

	/** Visível quando não há habilidade no slot. */
	UPROPERTY(BlueprintReadOnly, Category = "DS|UI", meta = (BindWidgetOptional))
	TObjectPtr<UWidget> EmptySlotOverlay;

	UPROPERTY(BlueprintReadOnly, Category = "DS|UI", meta = (BindWidgetOptional))
	TObjectPtr<UCommonActionWidget> InputHint;

private:
	const FGameplayAbilitySpec* FindSpec();
	bool SpecMatchesSlot(const FGameplayAbilitySpec& Spec) const;

	void RefreshIdentity(const UGameplayAbility* Ability);
	void RefreshCooldown(const FGameplayAbilitySpec* Spec);
	void RefreshCost(const FGameplayAbilitySpec* Spec);
	void RefreshCharges();
	void GetCooldown(const FGameplayAbilitySpec* Spec, float& OutRemaining, float& OutDuration) const;

	TWeakObjectPtr<UAbilitySystemComponent> AbilitySystem;
	TWeakObjectPtr<const UGameplayAbility> ShownAbility;
	FGameplayAbilitySpecHandle CachedHandle;
	bool bHasShownIdentity = false;
	bool bWasOnCooldown = false;
};
