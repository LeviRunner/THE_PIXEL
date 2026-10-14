#pragma once

#include "CoreMinimal.h"
#include "CommonUserWidget.h"
#include "Engine/TimerHandle.h"
#include "Core/DSTypes.h"
#include "DSWaveIndicatorWidget.generated.h"

class ADSGameState;
class UProgressBar;
class UTextBlock;
class UWidget;

/**
 * Onda X/10, aviso de chefe, inimigos restantes, nível/XP do time e tempo restante da fase.
 * Tudo por evento do ADSGameState, exceto o relógio da fase (timer leve).
 * WBP: WBP_DSWaveIndicator.
 */
UCLASS(Abstract)
class DUNGEONSTANCE_API UDSWaveIndicatorWidget : public UCommonUserWidget
{
	GENERATED_BODY()

public:
	void InitWithGameState(ADSGameState* InGameState);

protected:
	virtual void NativeDestruct() override;

	/** Disparado quando uma onda de chefe começa (animação/som de alerta no WBP). */
	UFUNCTION(BlueprintImplementableEvent, Category = "DS|UI")
	void BP_OnBossWaveStarted(int32 Wave);

	UFUNCTION(BlueprintImplementableEvent, Category = "DS|UI")
	void BP_OnTeamLevelUp(int32 NewLevel);

	UPROPERTY(BlueprintReadOnly, Category = "DS|UI", meta = (BindWidget))
	TObjectPtr<UTextBlock> WaveText;

	UPROPERTY(BlueprintReadOnly, Category = "DS|UI", meta = (BindWidgetOptional))
	TObjectPtr<UWidget> BossWarning;

	UPROPERTY(BlueprintReadOnly, Category = "DS|UI", meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> EnemiesRemainingText;

	UPROPERTY(BlueprintReadOnly, Category = "DS|UI", meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> TeamLevelText;

	UPROPERTY(BlueprintReadOnly, Category = "DS|UI", meta = (BindWidgetOptional))
	TObjectPtr<UProgressBar> TeamXPBar;

	UPROPERTY(BlueprintReadOnly, Category = "DS|UI", meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> TeamXPText;

	UPROPERTY(BlueprintReadOnly, Category = "DS|UI", meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> PhaseTimeText;

	UPROPERTY(EditDefaultsOnly, Category = "DS|UI", meta = (ClampMin = "0.1"))
	float PhaseClockInterval = 0.25f;

private:
	UFUNCTION()
	void HandleWaveChanged(int32 Wave, bool bIsBoss);

	UFUNCTION()
	void HandleTeamXPChanged(int32 Level, int32 XP, int32 XPToNext);

	UFUNCTION()
	void HandleEnemiesRemainingChanged(int32 Remaining);

	UFUNCTION()
	void HandleMatchPhaseChanged(EDSMatchPhase NewPhase);

	void UnbindFromGameState();
	void RefreshWave(int32 Wave, bool bIsBoss);
	void RefreshPhaseClock();

	TWeakObjectPtr<ADSGameState> GameState;
	FTimerHandle PhaseClockTimer;
	int32 LastTeamLevel = INDEX_NONE;
	int32 LastWave = INDEX_NONE;
};
