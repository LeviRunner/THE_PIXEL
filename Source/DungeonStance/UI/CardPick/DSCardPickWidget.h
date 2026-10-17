#pragma once

#include "CoreMinimal.h"
#include "CommonActivatableWidget.h"
#include "Engine/TimerHandle.h"
#include "DSCardPickWidget.generated.h"

class ADSGameState;
class ADSPlayerState;
class UDSCardOptionWidget;
class UPanelWidget;
class UProgressBar;
class UTextBlock;

/**
 * Escolha de cartas: abre quando PendingCardPicks > 0 e há CurrentCardChoices.
 * Cada clique chama ADSPlayerController::Server_PickCard; quem valida e sorteia é o servidor.
 * Ao acabar o tempo a UI só fecha/espera: a resolução das escolhas restantes é do servidor.
 * WBP: WBP_DSCardPick.
 */
UCLASS(Abstract)
class DUNGEONSTANCE_API UDSCardPickWidget : public UCommonActivatableWidget
{
	GENERATED_BODY()

public:
	void InitWithPlayerState(ADSPlayerState* InPlayerState, ADSGameState* InGameState);

protected:
	virtual void NativeOnInitialized() override;
	virtual void NativeDestruct() override;
	virtual void NativeOnActivated() override;
	virtual void NativeOnDeactivated() override;
	virtual UWidget* NativeGetDesiredFocusTarget() const override;
	virtual TOptional<FUIInputConfig> GetDesiredInputConfig() const override;

	UPROPERTY(BlueprintReadOnly, Category = "DS|UI", meta = (BindWidget))
	TObjectPtr<UPanelWidget> OptionContainer;

	UPROPERTY(BlueprintReadOnly, Category = "DS|UI", meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> TimerText;

	UPROPERTY(BlueprintReadOnly, Category = "DS|UI", meta = (BindWidgetOptional))
	TObjectPtr<UProgressBar> TimerBar;

	/** "Escolhas restantes: N". */
	UPROPERTY(BlueprintReadOnly, Category = "DS|UI", meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> PendingPicksText;

	UPROPERTY(EditDefaultsOnly, Category = "DS|UI")
	TSubclassOf<UDSCardOptionWidget> OptionWidgetClass;

	/**
	 * Duração exibida da janela (CARD_PICK_SEC do protótipo). Só usada quando o
	 * GameState não está na fase CardPick com PhaseEndServerTime válido.
	 */
	UPROPERTY(EditDefaultsOnly, Category = "DS|UI", meta = (ClampMin = "1.0"))
	float FallbackPickWindowSeconds = 20.f;

	UPROPERTY(EditDefaultsOnly, Category = "DS|UI", meta = (ClampMin = "0.05"))
	float TimerRefreshInterval = 0.1f;

private:
	UFUNCTION()
	void HandlePendingPicksChanged(int32 PendingPicks);

	UFUNCTION()
	void HandleCardChoicesChanged();

	void UnbindFromPlayerState();
	void EvaluateOpenState();
	void RebuildOptions(const TArray<FName>& Choices);
	void SetOptionsEnabled(bool bEnabled);
	void HandleOptionClicked(FName CardId);
	void RefreshTimer();
	float GetTimeRemaining(float& OutWindowSeconds) const;

	TWeakObjectPtr<ADSPlayerState> PlayerState;
	TWeakObjectPtr<ADSGameState> GameState;

	UPROPERTY(Transient)
	TArray<TObjectPtr<UDSCardOptionWidget>> Options;

	TArray<FName> ShownChoices;
	FTimerHandle TimerHandle;
	double LocalWindowStartTime = 0.0;
	bool bAwaitingServer = false;
};
