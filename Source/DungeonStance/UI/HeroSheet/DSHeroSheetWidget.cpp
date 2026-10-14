#include "UI/HeroSheet/DSHeroSheetWidget.h"

#include "AbilitySystemComponent.h"
#include "CommonButtonBase.h"
#include "Components/TextBlock.h"
#include "Components/WidgetSwitcher.h"
#include "Gameplay/Attributes/DSAttributeSet.h"
#include "GameFramework/PlayerState.h"
#include "UI/DSUIStateAccess.h"

namespace
{
	void SetHeroSheetStat(UTextBlock* Text, const UAbilitySystemComponent& ASC, const FGameplayAttribute& Attribute, int32 FractionDigits = 0)
	{
		if (!Text)
		{
			return;
		}
		FNumberFormattingOptions Options;
		Options.MinimumFractionalDigits = FractionDigits;
		Options.MaximumFractionalDigits = FractionDigits;
		Text->SetText(FText::AsNumber(ASC.GetNumericAttribute(Attribute), &Options));
	}
}

void UDSHeroSheetWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	auto BindTab = [this](UCommonButtonBase* Button, EDSHeroSheetTab Tab)
	{
		if (Button)
		{
			Button->OnClicked().AddUObject(this, &ThisClass::SetActiveTab, Tab);
		}
	};
	BindTab(Btn_TabSheet, EDSHeroSheetTab::Sheet);
	BindTab(Btn_TabVault, EDSHeroSheetTab::Vault);
	BindTab(Btn_TabBackpack, EDSHeroSheetTab::Backpack);
	BindTab(Btn_TabBelt, EDSHeroSheetTab::Belt);
}

void UDSHeroSheetWidget::NativeOnActivated()
{
	Super::NativeOnActivated();
	SetActiveTab(EDSHeroSheetTab::Sheet);
	RefreshStats();
}

UWidget* UDSHeroSheetWidget::NativeGetDesiredFocusTarget() const
{
	return Btn_TabSheet ? Btn_TabSheet.Get() : Super::NativeGetDesiredFocusTarget();
}

TOptional<FUIInputConfig> UDSHeroSheetWidget::GetDesiredInputConfig() const
{
	return FUIInputConfig(ECommonInputMode::Menu, EMouseCaptureMode::NoCapture);
}

void UDSHeroSheetWidget::SetActiveTab(EDSHeroSheetTab Tab)
{
	const int32 Index = static_cast<int32>(Tab);
	if (TabSwitcher && Index < TabSwitcher->GetNumWidgets())
	{
		TabSwitcher->SetActiveWidgetIndex(Index);
	}
	// TODO(Network): ao abrir Cofre/Mochila/Cinto, pedir o inventário ao backend e preencher a grade.
}

void UDSHeroSheetWidget::RefreshStats()
{
	const UAbilitySystemComponent* ASC = DSUIState::GetAbilitySystem(GetOwningPlayerState());
	if (!ASC)
	{
		return;
	}
	SetHeroSheetStat(StatHealthText, *ASC, UDSAttributeSet::GetMaxHealthAttribute());
	SetHeroSheetStat(StatDamageText, *ASC, UDSAttributeSet::GetDamageAttribute());
	SetHeroSheetStat(StatAttackSpeedText, *ASC, UDSAttributeSet::GetAttackSpeedAttribute(), 2);
	SetHeroSheetStat(StatCritText, *ASC, UDSAttributeSet::GetCritChanceAttribute(), 2);
	SetHeroSheetStat(StatArmorText, *ASC, UDSAttributeSet::GetArmorAttribute());
	SetHeroSheetStat(StatMoveSpeedText, *ASC, UDSAttributeSet::GetMoveSpeedAttribute());
	// TODO(UI): cartas possuídas (OwnedCards) e rank da classe nesta aba.
}
