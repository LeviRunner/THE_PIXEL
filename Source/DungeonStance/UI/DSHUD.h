#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "DSHUD.generated.h"

class UDSHUDLayoutWidget;
class UDSUIDataAsset;

/**
 * HUD do jogador local. Só cria o layout principal (UDSHUDLayoutWidget);
 * toda a lógica de exibição fica nos widgets.
 * Blueprint esperado: BP_DSHUD (LayoutClass = WBP_DSHUDLayout, UIData = DA_DSUIData).
 * No mapa de menu use outro BP (ex.: BP_DSMenuHUD) com um layout que só tenha o lobby.
 */
UCLASS(Blueprintable)
class DUNGEONSTANCE_API ADSHUD : public AHUD
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintPure, Category = "DS|UI")
	UDSHUDLayoutWidget* GetLayout() const { return Layout; }

	UFUNCTION(BlueprintPure, Category = "DS|UI")
	UDSUIDataAsset* GetUIData() const { return UIData; }

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	UPROPERTY(EditDefaultsOnly, Category = "DS|UI")
	TSubclassOf<UDSHUDLayoutWidget> LayoutClass;

	UPROPERTY(EditDefaultsOnly, Category = "DS|UI")
	TObjectPtr<UDSUIDataAsset> UIData;

	UPROPERTY(EditDefaultsOnly, Category = "DS|UI")
	int32 LayoutZOrder = 0;

private:
	void CreateLayout();

	UPROPERTY(Transient)
	TObjectPtr<UDSHUDLayoutWidget> Layout;
};
