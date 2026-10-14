#pragma once

#include "CoreMinimal.h"
#include "CommonActivatableWidget.h"
#include "Engine/TimerHandle.h"
#include "Core/DSTypes.h"
#include "DSHUDLayoutWidget.generated.h"

class ADSGameState;
class ADSPlayerState;
class UDSCardPickWidget;
class UDSCastBarWidget;
class UDSEndOfRunWidget;
class UDSLobbyWidget;
class UDSPartyFrameWidget;
class UDSTargetIndicatorWidget;
class UDSWaveIndicatorWidget;

/**
 * Raiz do HUD em partida. Espera GameState/PlayerState/ASC replicarem e então
 * inicializa os filhos. Alterna lobby / HUD de combate / fim de run pela fase da partida.
 * Todos os slots são opcionais para o mesmo C++ servir o HUD de jogo e o de menu.
 */
UCLASS(Abstract)
class DUNGEONSTANCE_API UDSHUDLayoutWidget : public UCommonActivatableWidget
{
	GENERATED_BODY()

protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;
	virtual TOptional<FUIInputConfig> GetDesiredInputConfig() const override;

	/** Chamado depois de cada mudança de fase (animações de transição no WBP). */
	UFUNCTION(BlueprintImplementableEvent, Category = "DS|UI")
	void BP_OnMatchPhaseChanged(EDSMatchPhase NewPhase);

	UPROPERTY(BlueprintReadOnly, Category = "DS|UI", meta = (BindWidgetOptional))
	TObjectPtr<UDSPartyFrameWidget> PartyFrame;

	UPROPERTY(BlueprintReadOnly, Category = "DS|UI", meta = (BindWidgetOptional))
	TObjectPtr<UDSCastBarWidget> CastBar;

	UPROPERTY(BlueprintReadOnly, Category = "DS|UI", meta = (BindWidgetOptional))
	TObjectPtr<UDSWaveIndicatorWidget> WaveIndicator;

	UPROPERTY(BlueprintReadOnly, Category = "DS|UI", meta = (BindWidgetOptional))
	TObjectPtr<UDSCardPickWidget> CardPick;

	UPROPERTY(BlueprintReadOnly, Category = "DS|UI", meta = (BindWidgetOptional))
	TObjectPtr<UDSTargetIndicatorWidget> TargetIndicator;

	UPROPERTY(BlueprintReadOnly, Category = "DS|UI", meta = (BindWidgetOptional))
	TObjectPtr<UDSLobbyWidget> Lobby;

	UPROPERTY(BlueprintReadOnly, Category = "DS|UI", meta = (BindWidgetOptional))
	TObjectPtr<UDSEndOfRunWidget> EndOfRun;

	/** Intervalo entre tentativas de achar GameState/PlayerState/ASC no cliente. */
	UPROPERTY(EditDefaultsOnly, Category = "DS|UI", meta = (ClampMin = "0.05"))
	float BindRetryInterval = 0.2f;

private:
	void TryBindToMatch();
	void InitializeChildren(ADSGameState& GameState, ADSPlayerState& PlayerState);
	void UnbindFromMatch();

	UFUNCTION()
	void HandleMatchPhaseChanged(EDSMatchPhase NewPhase);

	void ApplyMatchPhase(EDSMatchPhase Phase);
	void UpdateEndOfRun(EDSMatchPhase Phase, bool bRunOver);
	void SetCombatHUDVisible(bool bVisible);

	TWeakObjectPtr<ADSGameState> BoundGameState;
	TWeakObjectPtr<ADSPlayerState> BoundPlayerState;
	FTimerHandle BindRetryTimer;
	EDSMatchPhase LastAppliedPhase = EDSMatchPhase::WaitingForPlayers;
	bool bHasAppliedPhase = false;
};
