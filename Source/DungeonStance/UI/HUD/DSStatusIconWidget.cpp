#include "UI/HUD/DSStatusIconWidget.h"

#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "UI/DSUIDataAsset.h"
#include "UI/DSUIStatics.h"

void UDSStatusIconWidget::SetStatus(const FDSStatusUIInfo& Info, int32 Stacks)
{
	DSUI::SetImageFromSoftTexture(StatusIcon, Info.Icon);
	SetToolTipText(Info.DisplayName);

	if (StacksText)
	{
		// Só mostra o número quando acumula (1 acúmulo = só o ícone).
		StacksText->SetText(FText::AsNumber(Stacks));
		DSUI::SetWidgetShown(StacksText, Stacks > 1);
	}
	BP_OnStatusSet(Info.bIsDebuff, Stacks);
}
