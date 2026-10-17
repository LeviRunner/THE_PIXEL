#pragma once

#include "CoreMinimal.h"
#include "CommonUserWidget.h"
#include "Engine/TimerHandle.h"
#include "DSCastBarWidget.generated.h"

class UAbilitySystemComponent;
class UDSCastSlotWidget;

/**
 * Barra de casts do jogador local: ataque, skill da classe, esquiva e Q/E/R/F.
 * Recargas mudam continuamente, então os slots são atualizados num timer leve
 * (RefreshInterval) em vez de NativeTick.
 * WBP: WBP_DSCastBar.
 */
UCLASS(Abstract)
class DUNGEONSTANCE_API UDSCastBarWidget : public UCommonUserWidget
{
	GENERATED_BODY()

public:
	void InitWithAbilitySystem(UAbilitySystemComponent* InAbilitySystem);

protected:
	virtual void NativeDestruct() override;

	UPROPERTY(BlueprintReadOnly, Category = "DS|UI", meta = (BindWidget))
	TObjectPtr<UDSCastSlotWidget> Slot_Attack;

	UPROPERTY(BlueprintReadOnly, Category = "DS|UI", meta = (BindWidget))
	TObjectPtr<UDSCastSlotWidget> Slot_ClassSkill;

	UPROPERTY(BlueprintReadOnly, Category = "DS|UI", meta = (BindWidget))
	TObjectPtr<UDSCastSlotWidget> Slot_Dodge;

	UPROPERTY(BlueprintReadOnly, Category = "DS|UI", meta = (BindWidgetOptional))
	TObjectPtr<UDSCastSlotWidget> Slot_Support1;

	UPROPERTY(BlueprintReadOnly, Category = "DS|UI", meta = (BindWidgetOptional))
	TObjectPtr<UDSCastSlotWidget> Slot_Support2;

	UPROPERTY(BlueprintReadOnly, Category = "DS|UI", meta = (BindWidgetOptional))
	TObjectPtr<UDSCastSlotWidget> Slot_Support3;

	UPROPERTY(BlueprintReadOnly, Category = "DS|UI", meta = (BindWidgetOptional))
	TObjectPtr<UDSCastSlotWidget> Slot_Support4;

	UPROPERTY(EditDefaultsOnly, Category = "DS|UI", meta = (ClampMin = "0.03"))
	float RefreshInterval = 0.1f;

private:
	TArray<UDSCastSlotWidget*> GetSlots() const;
	void AssignDefaultInputTags();
	void RefreshSlots();

	FTimerHandle RefreshTimer;
};
