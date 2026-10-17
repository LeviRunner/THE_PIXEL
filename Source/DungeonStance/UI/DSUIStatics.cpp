#include "UI/DSUIStatics.h"

#include "Blueprint/UserWidget.h"
#include "Components/Image.h"
#include "Engine/Texture2D.h"
#include "GameFramework/PlayerController.h"
#include "UI/DSHUD.h"

#define LOCTEXT_NAMESPACE "DungeonStanceUI"

namespace DSUI
{
	UDSUIDataAsset* GetUIData(const UUserWidget* Widget)
	{
		const APlayerController* PlayerController = Widget ? Widget->GetOwningPlayer() : nullptr;
		const ADSHUD* HUD = PlayerController ? Cast<ADSHUD>(PlayerController->GetHUD()) : nullptr;
		return HUD ? HUD->GetUIData() : nullptr;
	}

	void SetImageFromSoftTexture(UImage* Image, const TSoftObjectPtr<UTexture2D>& Texture)
	{
		if (!Image)
		{
			return;
		}
		if (Texture.IsNull())
		{
			Image->SetVisibility(ESlateVisibility::Collapsed);
			return;
		}
		Image->SetBrushFromSoftTexture(Texture, false);
		Image->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
	}

	void SetWidgetShown(UWidget* Widget, bool bShown, ESlateVisibility ShownVisibility)
	{
		if (Widget)
		{
			Widget->SetVisibility(bShown ? ShownVisibility : ESlateVisibility::Collapsed);
		}
	}

	float SafeRatio(float Value, float MaxValue)
	{
		return MaxValue > KINDA_SMALL_NUMBER ? FMath::Clamp(Value / MaxValue, 0.f, 1.f) : 0.f;
	}

	FText FormatCooldownSeconds(float Seconds)
	{
		constexpr float DecimalThreshold = 10.f;
		FNumberFormattingOptions Options;
		Options.MinimumFractionalDigits = Seconds < DecimalThreshold ? 1 : 0;
		Options.MaximumFractionalDigits = Options.MinimumFractionalDigits;
		return FText::AsNumber(FMath::Max(0.f, Seconds), &Options);
	}

	FText FormatClockSeconds(float Seconds)
	{
		const int32 TotalSeconds = FMath::Max(0, FMath::CeilToInt(Seconds));
		FNumberFormattingOptions TwoDigits;
		TwoDigits.MinimumIntegralDigits = 2;
		return FText::Format(LOCTEXT("ClockFormat", "{0}:{1}"),
			FText::AsNumber(TotalSeconds / 60),
			FText::AsNumber(TotalSeconds % 60, &TwoDigits));
	}
}

#undef LOCTEXT_NAMESPACE
