#pragma once

#include "CoreMinimal.h"
#include "CommonActivatableWidget.h"
#include "DSHeroSheetWidget.generated.h"

class UCommonButtonBase;
class UTextBlock;
class UWidgetSwitcher;

/** Abas da ficha do herói. A ordem deve bater com os filhos do TabSwitcher no WBP. */
UENUM(BlueprintType)
enum class EDSHeroSheetTab : uint8
{
	Sheet		UMETA(DisplayName = "Ficha"),
	Vault		UMETA(DisplayName = "Cofre"),
	Backpack	UMETA(DisplayName = "Mochila"),
	Belt		UMETA(DisplayName = "Cinto de Batalha")
};

/**
 * ESQUELETO — ficha do herói e inventário (Cofre da Conta, Mochila, Cinto de Batalha).
 * Hoje só mostra atributos atuais do ASC local e troca de abas.
 * TODO(Network): inventário vem do backend (UDSBackendSubsystem); falta API no CONTRATOS.md.
 * TODO(UI): UDSInventoryGridWidget / UDSInventorySlotWidget (120 / 28 / 4 slots, ver ESPEC §9).
 * WBP: WBP_DSHeroSheet.
 */
UCLASS(Abstract)
class DUNGEONSTANCE_API UDSHeroSheetWidget : public UCommonActivatableWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "DS|UI")
	void SetActiveTab(EDSHeroSheetTab Tab);

protected:
	virtual void NativeOnInitialized() override;
	virtual void NativeOnActivated() override;
	virtual UWidget* NativeGetDesiredFocusTarget() const override;
	virtual TOptional<FUIInputConfig> GetDesiredInputConfig() const override;

	UPROPERTY(BlueprintReadOnly, Category = "DS|UI", meta = (BindWidget))
	TObjectPtr<UWidgetSwitcher> TabSwitcher;

	UPROPERTY(BlueprintReadOnly, Category = "DS|UI", meta = (BindWidgetOptional))
	TObjectPtr<UCommonButtonBase> Btn_TabSheet;

	UPROPERTY(BlueprintReadOnly, Category = "DS|UI", meta = (BindWidgetOptional))
	TObjectPtr<UCommonButtonBase> Btn_TabVault;

	UPROPERTY(BlueprintReadOnly, Category = "DS|UI", meta = (BindWidgetOptional))
	TObjectPtr<UCommonButtonBase> Btn_TabBackpack;

	UPROPERTY(BlueprintReadOnly, Category = "DS|UI", meta = (BindWidgetOptional))
	TObjectPtr<UCommonButtonBase> Btn_TabBelt;

	// Atributos (aba Ficha). Todos opcionais.
	UPROPERTY(BlueprintReadOnly, Category = "DS|UI", meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> StatHealthText;

	UPROPERTY(BlueprintReadOnly, Category = "DS|UI", meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> StatDamageText;

	UPROPERTY(BlueprintReadOnly, Category = "DS|UI", meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> StatAttackSpeedText;

	UPROPERTY(BlueprintReadOnly, Category = "DS|UI", meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> StatCritText;

	UPROPERTY(BlueprintReadOnly, Category = "DS|UI", meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> StatArmorText;

	UPROPERTY(BlueprintReadOnly, Category = "DS|UI", meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> StatMoveSpeedText;

private:
	void RefreshStats();
};
