#pragma once

#include "CoreMinimal.h"
#include "CommonUserWidget.h"
#include "DSStatusIconWidget.generated.h"

class UImage;
class UTextBlock;
struct FDSStatusUIInfo;

/** Ícone de um status ativo (sangramento, atordoamento...) com acúmulos. WBP: WBP_DSStatusIcon. */
UCLASS(Abstract)
class DUNGEONSTANCE_API UDSStatusIconWidget : public UCommonUserWidget
{
	GENERATED_BODY()

public:
	void SetStatus(const FDSStatusUIInfo& Info, int32 Stacks);

protected:
	UFUNCTION(BlueprintImplementableEvent, Category = "DS|UI")
	void BP_OnStatusSet(bool bIsDebuff, int32 Stacks);

	UPROPERTY(BlueprintReadOnly, Category = "DS|UI", meta = (BindWidget))
	TObjectPtr<UImage> StatusIcon;

	UPROPERTY(BlueprintReadOnly, Category = "DS|UI", meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> StacksText;
};
