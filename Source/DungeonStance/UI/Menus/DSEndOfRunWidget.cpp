#include "UI/Menus/DSEndOfRunWidget.h"

#include "CommonButtonBase.h"
#include "Components/PanelWidget.h"
#include "Components/TextBlock.h"
#include "Network/DSSessionSubsystem.h"
#include "UI/DSUIDataAsset.h"
#include "UI/DSUIStateAccess.h"
#include "UI/DSUIStatics.h"

#define LOCTEXT_NAMESPACE "DungeonStanceUI"

void UDSEndOfRunWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();
	SetVisibility(ESlateVisibility::Collapsed);
	Btn_Continue->OnClicked().AddUObject(this, &ThisClass::HandleContinueClicked);
}

void UDSEndOfRunWidget::NativeOnActivated()
{
	Super::NativeOnActivated();
	SetVisibility(ESlateVisibility::Visible);
}

void UDSEndOfRunWidget::NativeOnDeactivated()
{
	SetVisibility(ESlateVisibility::Collapsed);
	Super::NativeOnDeactivated();
}

UWidget* UDSEndOfRunWidget::NativeGetDesiredFocusTarget() const
{
	return Btn_Continue.Get();
}

TOptional<FUIInputConfig> UDSEndOfRunWidget::GetDesiredInputConfig() const
{
	return FUIInputConfig(ECommonInputMode::Menu, EMouseCaptureMode::NoCapture);
}

void UDSEndOfRunWidget::ShowRunResult(const FDSRunResult& Result)
{
	ResultTitleText->SetText(Result.bVictory ? LOCTEXT("Victory", "VITÓRIA") : LOCTEXT("Defeat", "DERROTA"));

	if (HeroClassText)
	{
		const UDSUIDataAsset* UIData = DSUI::GetUIData(this);
		HeroClassText->SetText(UIData ? UIData->GetHeroClassName(Result.HeroClass)
			: StaticEnum<EDSHeroClass>()->GetDisplayNameTextByValue(static_cast<int64>(Result.HeroClass)));
	}
	if (WavesClearedText)
	{
		WavesClearedText->SetText(FText::Format(LOCTEXT("WavesCleared", "Ondas sobrevividas: {0}"), FText::AsNumber(Result.WavesCleared)));
	}
	if (KillsText)
	{
		KillsText->SetText(FText::Format(LOCTEXT("Kills", "Abates: {0}"), FText::AsNumber(Result.Kills)));
	}
	if (RankXPText)
	{
		RankXPText->SetText(FText::Format(LOCTEXT("RankXP", "+{0} XP de classe"), FText::AsNumber(Result.RankXPEarned)));
	}
	if (GoldText)
	{
		GoldText->SetText(FText::Format(LOCTEXT("Gold", "+{0} ouro"), FText::AsNumber(Result.GoldEarned)));
		DSUI::SetWidgetShown(GoldText, Result.GoldEarned > 0);
	}
	DSUI::SetWidgetShown(LootContainer, false); // TODO: preencher quando o Espólio vier replicado.

	BP_OnResultShown(Result);
}

void UDSEndOfRunWidget::HandleContinueClicked()
{
	if (UDSSessionSubsystem* Sessions = DSUIState::GetSessionSubsystem(this))
	{
		Sessions->DestroySession();
	}
	BP_OnContinueRequested();
}

#undef LOCTEXT_NAMESPACE
