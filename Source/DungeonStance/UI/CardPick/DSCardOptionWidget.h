#pragma once

#include "CoreMinimal.h"
#include "CommonButtonBase.h"
#include "DSCardOptionWidget.generated.h"

class UImage;
class UTextBlock;
struct FDSDisplayUIInfo;

/**
 * Uma carta sorteada na escolha de cartas. É um botão CommonUI (foco/gamepad nativos).
 * O clique é tratado pela UDSCardPickWidget via OnClicked().
 * WBP: WBP_DSCardOption.
 */
UCLASS(Abstract)
class DUNGEONSTANCE_API UDSCardOptionWidget : public UCommonButtonBase
{
	GENERATED_BODY()

public:
	/** Info pode ser nula (sem entrada no UIData): mostra o CardId como nome. */
	void SetCard(FName InCardId, const FDSDisplayUIInfo* Info, int32 CurrentStacks);

	UFUNCTION(BlueprintPure, Category = "DS|UI")
	FName GetCardId() const { return CardId; }

protected:
	/** Para o WBP completar com dados extras da carta (ramo, stance, raridade...). */
	UFUNCTION(BlueprintImplementableEvent, Category = "DS|UI")
	void BP_OnCardSet(FName NewCardId, int32 CurrentStacks);

	UPROPERTY(BlueprintReadOnly, Category = "DS|UI", meta = (BindWidget))
	TObjectPtr<UTextBlock> CardNameText;

	UPROPERTY(BlueprintReadOnly, Category = "DS|UI", meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> CardDescriptionText;

	UPROPERTY(BlueprintReadOnly, Category = "DS|UI", meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> StacksText;

	UPROPERTY(BlueprintReadOnly, Category = "DS|UI", meta = (BindWidgetOptional))
	TObjectPtr<UImage> CardIcon;

private:
	FName CardId;
};
