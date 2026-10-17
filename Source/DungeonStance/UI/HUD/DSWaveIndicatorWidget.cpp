#include "UI/HUD/DSWaveIndicatorWidget.h"

#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "Core/DSGameState.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "UI/DSUIStateAccess.h"
#include "UI/DSUIStatics.h"

#define LOCTEXT_NAMESPACE "DungeonStanceUI"

void UDSWaveIndicatorWidget::InitWithGameState(ADSGameState* InGameState)
{
	UnbindFromGameState();
	GameState = InGameState;
	if (!InGameState)
	{
		return;
	}

	InGameState->OnWaveChanged.AddDynamic(this, &ThisClass::HandleWaveChanged);
	InGameState->OnTeamXPChanged.AddDynamic(this, &ThisClass::HandleTeamXPChanged);
	InGameState->OnEnemiesRemainingChanged.AddDynamic(this, &ThisClass::HandleEnemiesRemainingChanged);
	InGameState->OnMatchPhaseChanged.AddDynamic(this, &ThisClass::HandleMatchPhaseChanged);

	// Estado inicial (o cliente pode ter entrado no meio da run).
	RefreshWave(DSUIState::GetCurrentWave(*InGameState), DSUIState::IsBossWave(*InGameState));
	LastTeamLevel = DSUIState::GetTeamLevel(*InGameState);
	HandleTeamXPChanged(LastTeamLevel, DSUIState::GetTeamXP(*InGameState), DSUIState::GetTeamXPToNext(*InGameState));
	HandleEnemiesRemainingChanged(DSUIState::GetEnemiesRemaining(*InGameState));

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimer(PhaseClockTimer, this, &ThisClass::RefreshPhaseClock, PhaseClockInterval, true);
	}
	HandleMatchPhaseChanged(DSUIState::GetMatchPhase(*InGameState));
}

void UDSWaveIndicatorWidget::NativeDestruct()
{
	UnbindFromGameState();
	Super::NativeDestruct();
}

void UDSWaveIndicatorWidget::UnbindFromGameState()
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(PhaseClockTimer);
	}
	if (ADSGameState* GS = GameState.Get())
	{
		GS->OnWaveChanged.RemoveDynamic(this, &ThisClass::HandleWaveChanged);
		GS->OnTeamXPChanged.RemoveDynamic(this, &ThisClass::HandleTeamXPChanged);
		GS->OnEnemiesRemainingChanged.RemoveDynamic(this, &ThisClass::HandleEnemiesRemainingChanged);
		GS->OnMatchPhaseChanged.RemoveDynamic(this, &ThisClass::HandleMatchPhaseChanged);
	}
	GameState.Reset();
}

void UDSWaveIndicatorWidget::HandleWaveChanged(int32 Wave, bool bIsBoss)
{
	RefreshWave(Wave, bIsBoss);
}

void UDSWaveIndicatorWidget::RefreshWave(int32 Wave, bool bIsBoss)
{
	const ADSGameState* GS = GameState.Get();
	const int32 MaxWaves = GS ? DSUIState::GetMaxWaves(*GS) : DSConstants::MaxWaves;

	WaveText->SetText(FText::Format(LOCTEXT("WaveFormat", "Onda {0}/{1}"), FText::AsNumber(Wave), FText::AsNumber(MaxWaves)));
	DSUI::SetWidgetShown(BossWarning, bIsBoss);

	if (bIsBoss && Wave != LastWave)
	{
		BP_OnBossWaveStarted(Wave);
	}
	LastWave = Wave;
}

void UDSWaveIndicatorWidget::HandleTeamXPChanged(int32 Level, int32 XP, int32 XPToNext)
{
	if (TeamLevelText)
	{
		TeamLevelText->SetText(FText::Format(LOCTEXT("TeamLevelFormat", "Nível {0}"), FText::AsNumber(Level)));
	}
	if (TeamXPBar)
	{
		TeamXPBar->SetPercent(DSUI::SafeRatio(static_cast<float>(XP), static_cast<float>(XPToNext)));
	}
	if (TeamXPText)
	{
		TeamXPText->SetText(FText::Format(LOCTEXT("TeamXPFormat", "{0} / {1} XP"), FText::AsNumber(XP), FText::AsNumber(XPToNext)));
	}
	if (LastTeamLevel != INDEX_NONE && Level > LastTeamLevel)
	{
		BP_OnTeamLevelUp(Level);
	}
	LastTeamLevel = Level;
}

void UDSWaveIndicatorWidget::HandleEnemiesRemainingChanged(int32 Remaining)
{
	if (EnemiesRemainingText)
	{
		EnemiesRemainingText->SetText(FText::Format(LOCTEXT("EnemiesRemainingFormat", "Inimigos: {0}"), FText::AsNumber(Remaining)));
	}
}

void UDSWaveIndicatorWidget::HandleMatchPhaseChanged(EDSMatchPhase NewPhase)
{
	// Inimigos restantes só fazem sentido com a onda em andamento.
	DSUI::SetWidgetShown(EnemiesRemainingText, NewPhase == EDSMatchPhase::WaveInProgress);
	RefreshPhaseClock();
}

void UDSWaveIndicatorWidget::RefreshPhaseClock()
{
	if (!PhaseTimeText)
	{
		return;
	}
	const ADSGameState* GS = GameState.Get();
	const float Remaining = GS ? DSUIState::GetPhaseTimeRemaining(*GS) : 0.f;
	PhaseTimeText->SetText(DSUI::FormatClockSeconds(Remaining));
	DSUI::SetWidgetShown(PhaseTimeText, Remaining > 0.f);
}

#undef LOCTEXT_NAMESPACE
