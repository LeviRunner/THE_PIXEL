#pragma once

#include "CoreMinimal.h"
#include "CommonUserWidget.h"
#include "Engine/TimerHandle.h"
#include "DSTargetIndicatorWidget.generated.h"

class AActor;
class UAbilitySystemComponent;
class UProgressBar;
class UTextBlock;
struct FOnAttributeChangeData;

/**
 * Painel do alvo travado (lock-on): vida e escudo do alvo.
 * A troca de alvo é consultada num timer leve até o componente de lock-on expor um delegate;
 * a vida do alvo atualiza por evento do ASC.
 * WBP: WBP_DSTargetIndicator.
 */
UCLASS(Abstract)
class DUNGEONSTANCE_API UDSTargetIndicatorWidget : public UCommonUserWidget
{
	GENERATED_BODY()

protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

	/**
	 * Retorna o alvo travado do jogador local.
	 * TODO(Gameplay): implementação C++ padrão retorna nulo até existir a API de
	 * UDSTargetLockComponent (ex.: GetLockedTarget()). Pode ser sobrescrito no WBP.
	 */
	UFUNCTION(BlueprintNativeEvent, Category = "DS|UI")
	AActor* ResolveLockedTarget() const;

	/** Nome/retrato do alvo ficam no WBP (o C++ não conhece os dados do inimigo). */
	UFUNCTION(BlueprintImplementableEvent, Category = "DS|UI")
	void BP_OnTargetChanged(AActor* NewTarget);

	UPROPERTY(BlueprintReadOnly, Category = "DS|UI", meta = (BindWidget))
	TObjectPtr<UProgressBar> TargetHealthBar;

	UPROPERTY(BlueprintReadOnly, Category = "DS|UI", meta = (BindWidgetOptional))
	TObjectPtr<UProgressBar> TargetShieldBar;

	UPROPERTY(BlueprintReadOnly, Category = "DS|UI", meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> TargetHealthText;

	UPROPERTY(EditDefaultsOnly, Category = "DS|UI", meta = (ClampMin = "0.05"))
	float TargetPollInterval = 0.15f;

private:
	void PollTarget();
	void SetTarget(AActor* NewTarget);
	void HandleTargetAttributeChanged(const FOnAttributeChangeData& Data);
	void RefreshTargetVitals();
	void UnbindTargetAbilitySystem();

	TWeakObjectPtr<AActor> CurrentTarget;
	TWeakObjectPtr<UAbilitySystemComponent> TargetAbilitySystem;
	FTimerHandle PollTimer;
};
