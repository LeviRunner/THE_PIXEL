#include "UI/HUD/DSHUDLayoutWidget.h"

#include "Core/DSGameState.h"
#include "Core/DSPlayerState.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "UI/CardPick/DSCardPickWidget.h"
#include "UI/DSUIStateAccess.h"
#include "UI/DSUIStatics.h"
#include "UI/HUD/DSCastBarWidget.h"
#include "UI/HUD/DSPartyFrameWidget.h"
#include "UI/HUD/DSTargetIndicatorWidget.h"
#include "UI/HUD/DSWaveIndicatorWidget.h"
#include "UI/Menus/DSEndOfRunWidget.h"
#include "UI/Menus/DSLobbyWidget.h"

void UDSHUDLayoutWidget::NativeConstruct()
{
	Super::NativeConstruct();

	TryBindToMatch();
	if (!BoundGameState.IsValid() && GetWorld())
	{
		GetWorld()->GetTimerManager().SetTimer(BindRetryTimer, this, &ThisClass::TryBindToMatch, BindRetryInterval, true);
	}
}

void UDSHUDLayoutWidget::NativeDestruct()
{
	UnbindFromMatch();
	Super::NativeDestruct();
}

TOptional<FUIInputConfig> UDSHUDLayoutWidget::GetDesiredInputConfig() const
{
	// HUD de combate: input vai para o jogo. Telas filhas (cartas, lobby) pedem o próprio modo.
	return FUIInputConfig(ECommonInputMode::Game, EMouseCaptureMode::CapturePermanently);
}

void UDSHUDLayoutWidget::TryBindToMatch()
{
	UWorld* World = GetWorld();
	ADSGameState* GameState = World ? World->GetGameState<ADSGameState>() : nullptr;
	ADSPlayerState* PlayerState = GetOwningPlayerState<ADSPlayerState>();
	if (!GameState || !PlayerState || !DSUIState::GetAbilitySystem(PlayerState))
	{
		return; // Ainda replicando (ou mapa de menu sem ADSGameState): tenta de novo no timer.
	}

	if (World)
	{
		World->GetTimerManager().ClearTimer(BindRetryTimer);
	}
	BoundGameState = GameState;
	BoundPlayerState = PlayerState;

	InitializeChildren(*GameState, *PlayerState);
	GameState->OnMatchPhaseChanged.AddDynamic(this, &ThisClass::HandleMatchPhaseChanged);
	ApplyMatchPhase(DSUIState::GetMatchPhase(*GameState));
}

void UDSHUDLayoutWidget::InitializeChildren(ADSGameState& GameState, ADSPlayerState& PlayerState)
{
	if (PartyFrame)
	{
		PartyFrame->InitParty(&GameState, &PlayerState);
	}
	if (CastBar)
	{
		CastBar->InitWithAbilitySystem(DSUIState::GetAbilitySystem(&PlayerState));
	}
	if (WaveIndicator)
	{
		WaveIndicator->InitWithGameState(&GameState);
	}
	if (CardPick)
	{
		CardPick->InitWithPlayerState(&PlayerState, &GameState);
	}
	if (Lobby)
	{
		Lobby->InitWithPlayerState(&PlayerState);
	}
}

void UDSHUDLayoutWidget::UnbindFromMatch()
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(BindRetryTimer);
	}
	if (ADSGameState* GameState = BoundGameState.Get())
	{
		GameState->OnMatchPhaseChanged.RemoveDynamic(this, &ThisClass::HandleMatchPhaseChanged);
	}
	BoundGameState.Reset();
	BoundPlayerState.Reset();
}

void UDSHUDLayoutWidget::HandleMatchPhaseChanged(EDSMatchPhase NewPhase)
{
	ApplyMatchPhase(NewPhase);
}

void UDSHUDLayoutWidget::ApplyMatchPhase(EDSMatchPhase Phase)
{
	if (bHasAppliedPhase && Phase == LastAppliedPhase)
	{
		return;
	}
	bHasAppliedPhase = true;
	LastAppliedPhase = Phase;

	const bool bInLobby = Phase == EDSMatchPhase::WaitingForPlayers;
	const bool bRunOver = Phase == EDSMatchPhase::Victory || Phase == EDSMatchPhase::Defeat || Phase == EDSMatchPhase::Ended;

	SetCombatHUDVisible(!bInLobby && !bRunOver);

	if (Lobby)
	{
		if (bInLobby)
		{
			Lobby->ActivateWidget();
		}
		else
		{
			// DeactivateWidget não faz nada se nunca foi ativado: esconde explicitamente.
			Lobby->DeactivateWidget();
			Lobby->SetVisibility(ESlateVisibility::Collapsed);
		}
	}

	UpdateEndOfRun(Phase, bRunOver);
	BP_OnMatchPhaseChanged(Phase);
}

void UDSHUDLayoutWidget::UpdateEndOfRun(EDSMatchPhase Phase, bool bRunOver)
{
	if (!EndOfRun)
	{
		return;
	}
	if (!bRunOver)
	{
		EndOfRun->DeactivateWidget();
		return;
	}
	if (EndOfRun->IsActivated())
	{
		return; // Victory/Defeat -> Ended: mantém o resultado já mostrado.
	}

	ADSGameState* GameState = BoundGameState.Get();
	ADSPlayerState* PlayerState = BoundPlayerState.Get();
	if (GameState && PlayerState)
	{
		// Ended direto (sem Victory) conta como derrota, ex.: partida esvaziou.
		const bool bVictory = Phase == EDSMatchPhase::Victory;
		EndOfRun->ShowRunResult(DSUIState::BuildLocalRunResult(*GameState, *PlayerState, bVictory));
		EndOfRun->ActivateWidget();
	}
}

void UDSHUDLayoutWidget::SetCombatHUDVisible(bool bVisible)
{
	DSUI::SetWidgetShown(CastBar, bVisible);
	DSUI::SetWidgetShown(WaveIndicator, bVisible);
	// TargetIndicator se mostra/esconde sozinho conforme existe alvo travado.
	// A party continua visível no lobby (mostra quem já entrou); some só no fim da run.
	DSUI::SetWidgetShown(PartyFrame, bVisible || LastAppliedPhase == EDSMatchPhase::WaitingForPlayers);
}
