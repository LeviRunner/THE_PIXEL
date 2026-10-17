#pragma once

#include "CoreMinimal.h"
#include "CommonActivatableWidget.h"
#include "Core/DSTypes.h"
#include "DSEndOfRunWidget.generated.h"

class UCommonButtonBase;
class UPanelWidget;
class UTextBlock;

/**
 * Tela de vitória/derrota: ondas, kills, XP de rank e espólio (dados de FDSRunResult).
 * WBP: WBP_DSEndOfRun.
 */
UCLASS(Abstract)
class DUNGEONSTANCE_API UDSEndOfRunWidget : public UCommonActivatableWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "DS|UI")
	void ShowRunResult(const FDSRunResult& Result);

protected:
	virtual void NativeOnInitialized() override;
	virtual void NativeOnActivated() override;
	virtual void NativeOnDeactivated() override;
	virtual UWidget* NativeGetDesiredFocusTarget() const override;
	virtual TOptional<FUIInputConfig> GetDesiredInputConfig() const override;

	UFUNCTION(BlueprintImplementableEvent, Category = "DS|UI")
	void BP_OnResultShown(const FDSRunResult& Result);

	/** O WBP decide o destino (ex.: abrir o mapa do menu). A sessão já foi encerrada. */
	UFUNCTION(BlueprintImplementableEvent, Category = "DS|UI")
	void BP_OnContinueRequested();

	UPROPERTY(BlueprintReadOnly, Category = "DS|UI", meta = (BindWidget))
	TObjectPtr<UTextBlock> ResultTitleText;

	UPROPERTY(BlueprintReadOnly, Category = "DS|UI", meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> HeroClassText;

	UPROPERTY(BlueprintReadOnly, Category = "DS|UI", meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> WavesClearedText;

	UPROPERTY(BlueprintReadOnly, Category = "DS|UI", meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> KillsText;

	UPROPERTY(BlueprintReadOnly, Category = "DS|UI", meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> RankXPText;

	UPROPERTY(BlueprintReadOnly, Category = "DS|UI", meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> GoldText;

	/** TODO(Network/Core): itens do Espólio (FDSRunResult ainda não traz itens). */
	UPROPERTY(BlueprintReadOnly, Category = "DS|UI", meta = (BindWidgetOptional))
	TObjectPtr<UPanelWidget> LootContainer;

	UPROPERTY(BlueprintReadOnly, Category = "DS|UI", meta = (BindWidget))
	TObjectPtr<UCommonButtonBase> Btn_Continue;

private:
	void HandleContinueClicked();
};
