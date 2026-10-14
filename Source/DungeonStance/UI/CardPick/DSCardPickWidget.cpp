#include "UI/CardPick/DSCardPickWidget.h"

#include "Components/PanelWidget.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "Core/DSGameState.h"
#include "Core/DSPlayerController.h"
#include "Core/DSPlayerState.h"
#include "DungeonStance.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "UI/CardPick/DSCardOptionWidget.h"
#include "UI/DSUIDataAsset.h"
#include "UI/DSUIStateAccess.h"
#include "UI/DSUIStatics.h"

#define LOCTEXT_NAMESPACE "DungeonStanceUI"

void UDSCardPickWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();
	SetVisibility(ESlateVisibility::Collapsed);
}

void UDSCardPickWidget::InitWithPlayerState(ADSPlayerState* InPlayerState, ADSGameState* InGameState)
{
	UnbindFromPlayerState();
	PlayerState = InPlayerState;
	GameState = InGameState;
	if (!InPlayerState)
	{
		return;
	}
	InPlayerState->OnPendingPicksChanged.AddDynamic(this, &ThisClass::HandlePendingPicksChanged);
	InPlayerState->OnCardChoicesChanged.AddDynamic(this, &ThisClass::HandleCardChoicesChanged);
	EvaluateOpenState();
}

void UDSCardPickWidget::NativeDestruct()
{
	UnbindFromPlayerState();
	Super::NativeDestruct();
}

void UDSCardPickWidget::UnbindFromPlayerState()
{
	if (ADSPlayerState* PS = PlayerState.Get())
	{
		PS->OnPendingPicksChanged.RemoveDynamic(this, &ThisClass::HandlePendingPicksChanged);
		PS->OnCardChoicesChanged.RemoveDynamic(this, &ThisClass::HandleCardChoicesChanged);
	}
	PlayerState.Reset();
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(TimerHandle);
	}
}

void UDSCardPickWidget::NativeOnActivated()
{
	Super::NativeOnActivated();
	SetVisibility(ESlateVisibility::Visible);
	LocalWindowStartTime = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0;
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimer(TimerHandle, this, &ThisClass::RefreshTimer, TimerRefreshInterval, true);
	}
	RefreshTimer();
}

void UDSCardPickWidget::NativeOnDeactivated()
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(TimerHandle);
	}
	SetVisibility(ESlateVisibility::Collapsed);
	Super::NativeOnDeactivated();
}

UWidget* UDSCardPickWidget::NativeGetDesiredFocusTarget() const
{
	// Gamepad: foco começa na primeira carta.
	return Options.Num() > 0 ? Options[0].Get() : Super::NativeGetDesiredFocusTarget();
}

TOptional<FUIInputConfig> UDSCardPickWidget::GetDesiredInputConfig() const
{
	// A escolha acontece entre ondas, mas o jogador ainda pode se mover: jogo + menu.
	return FUIInputConfig(ECommonInputMode::All, EMouseCaptureMode::NoCapture);
}

void UDSCardPickWidget::HandlePendingPicksChanged(int32 PendingPicks)
{
	// Uma escolha foi consumida (ou chegou uma nova): libera os botões para o próximo sorteio.
	bAwaitingServer = false;
	ShownChoices.Reset();
	EvaluateOpenState();
}

void UDSCardPickWidget::HandleCardChoicesChanged()
{
	EvaluateOpenState();
}

void UDSCardPickWidget::EvaluateOpenState()
{
	const ADSPlayerState* PS = PlayerState.Get();
	const int32 Pending = PS ? DSUIState::GetPendingCardPicks(*PS) : 0;
	const TArray<FName> Choices = PS ? DSUIState::GetCurrentCardChoices(*PS) : TArray<FName>();

	if (Pending <= 0 || Choices.Num() == 0)
	{
		if (IsActivated())
		{
			DeactivateWidget();
		}
		return;
	}

	if (Choices != ShownChoices)
	{
		RebuildOptions(Choices);
	}
	if (PendingPicksText)
	{
		PendingPicksText->SetText(FText::Format(LOCTEXT("PendingPicksFormat", "Escolhas restantes: {0}"), FText::AsNumber(Pending)));
	}
	if (!IsActivated())
	{
		ActivateWidget();
	}
}

void UDSCardPickWidget::RebuildOptions(const TArray<FName>& Choices)
{
	ShownChoices = Choices;
	bAwaitingServer = false;
	OptionContainer->ClearChildren();
	Options.Reset();

	if (!OptionWidgetClass)
	{
		UE_LOG(LogDungeonStance, Warning, TEXT("UDSCardPickWidget '%s' sem OptionWidgetClass."), *GetName());
		return;
	}

	const UDSUIDataAsset* UIData = DSUI::GetUIData(this);
	const ADSPlayerState* PS = PlayerState.Get();
	for (const FName& CardId : Choices)
	{
		UDSCardOptionWidget* Option = CreateWidget<UDSCardOptionWidget>(this, OptionWidgetClass);
		if (!Option)
		{
			continue;
		}
		const int32 Stacks = PS ? DSUIState::GetOwnedCardStacks(*PS, CardId) : 0;
		Option->SetCard(CardId, UIData ? UIData->FindCardInfo(CardId) : nullptr, Stacks);
		Option->OnClicked().AddUObject(this, &ThisClass::HandleOptionClicked, CardId);
		OptionContainer->AddChild(Option);
		Options.Add(Option);
	}

	// Recoloca o foco do gamepad na primeira carta do novo sorteio.
	if (IsActivated() && Options.Num() > 0)
	{
		Options[0]->SetFocus();
	}
}

void UDSCardPickWidget::SetOptionsEnabled(bool bEnabled)
{
	for (UDSCardOptionWidget* Option : Options)
	{
		if (Option)
		{
			Option->SetIsEnabled(bEnabled);
		}
	}
}

void UDSCardPickWidget::HandleOptionClicked(FName CardId)
{
	if (bAwaitingServer)
	{
		return; // Evita escolher duas vezes antes do servidor responder.
	}
	ADSPlayerController* PC = DSUIState::GetDSPlayerController(GetOwningPlayer());
	if (!PC)
	{
		return;
	}
	bAwaitingServer = true;
	SetOptionsEnabled(false);
	PC->Server_PickCard(CardId);
}

float UDSCardPickWidget::GetTimeRemaining(float& OutWindowSeconds) const
{
	OutWindowSeconds = FallbackPickWindowSeconds;
	const ADSGameState* GS = GameState.Get();
	if (GS && DSUIState::GetMatchPhase(*GS) == EDSMatchPhase::CardPick)
	{
		const float ServerRemaining = DSUIState::GetPhaseTimeRemaining(*GS);
		if (ServerRemaining > 0.f)
		{
			OutWindowSeconds = FMath::Max(FallbackPickWindowSeconds, ServerRemaining);
			return ServerRemaining;
		}
	}
	// Sem relógio do servidor: contagem local só para exibição.
	const double Now = GetWorld() ? GetWorld()->GetTimeSeconds() : LocalWindowStartTime;
	return FMath::Max(0.f, FallbackPickWindowSeconds - static_cast<float>(Now - LocalWindowStartTime));
}

void UDSCardPickWidget::RefreshTimer()
{
	float WindowSeconds = FallbackPickWindowSeconds;
	const float Remaining = GetTimeRemaining(WindowSeconds);
	if (TimerText)
	{
		TimerText->SetText(FText::AsNumber(FMath::CeilToInt(Remaining)));
	}
	if (TimerBar)
	{
		TimerBar->SetPercent(DSUI::SafeRatio(Remaining, WindowSeconds));
	}
}

#undef LOCTEXT_NAMESPACE
