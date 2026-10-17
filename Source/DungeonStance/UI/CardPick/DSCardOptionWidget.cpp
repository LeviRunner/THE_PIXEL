#include "UI/CardPick/DSCardOptionWidget.h"

#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "UI/DSUIDataAsset.h"
#include "UI/DSUIStatics.h"

#define LOCTEXT_NAMESPACE "DungeonStanceUI"

void UDSCardOptionWidget::SetCard(FName InCardId, const FDSDisplayUIInfo* Info, int32 CurrentStacks)
{
	CardId = InCardId;

	const bool bHasName = Info && !Info->DisplayName.IsEmpty();
	CardNameText->SetText(bHasName ? Info->DisplayName : FText::FromName(InCardId));

	if (CardDescriptionText)
	{
		CardDescriptionText->SetText(Info ? Info->Description : FText::GetEmpty());
	}
	if (CardIcon)
	{
		DSUI::SetImageFromSoftTexture(CardIcon, Info ? Info->Icon : TSoftObjectPtr<UTexture2D>());
	}
	if (StacksText)
	{
		// TODO(Gameplay): mostrar "x/máx." quando UDSCardData expuser o máximo de acúmulos.
		StacksText->SetText(FText::Format(LOCTEXT("CardStacksFormat", "Acúmulos: {0}"), FText::AsNumber(CurrentStacks)));
		DSUI::SetWidgetShown(StacksText, CurrentStacks > 0);
	}
	BP_OnCardSet(InCardId, CurrentStacks);
}

#undef LOCTEXT_NAMESPACE
