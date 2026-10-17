#pragma once

#include "CoreMinimal.h"
#include "CommonActivatableWidget.h"
#include "Engine/TimerHandle.h"
#include "Core/DSTypes.h"
// FDSSessionInfo precisa estar completo para o parâmetro do UFUNCTION (código gerado).
#include "Network/DSSessionSubsystem.h"
#include "DSLobbyWidget.generated.h"

class ADSPlayerState;
class UCommonButtonBase;
class UImage;
class UTextBlock;

/**
 * Lobby / seleção: classe, stance, tamanho da party, pronto, iniciar solo / procurar / hospedar.
 * Fora de sessão grava a escolha no UDSGameInstance; dentro de sessão também envia
 * Server_SelectHero/Server_SetReady. Seletores são cíclicos (anterior/próximo) para gamepad.
 * WBP: WBP_DSLobby (menu: bShowSessionButtons = true; na partida: false).
 */
UCLASS(Abstract)
class DUNGEONSTANCE_API UDSLobbyWidget : public UCommonActivatableWidget
{
	GENERATED_BODY()

public:
	/** Chamado pelo layout do HUD quando já existe PlayerState replicado (lobby em partida). */
	void InitWithPlayerState(ADSPlayerState* InPlayerState);

protected:
	virtual void NativeOnInitialized() override;
	virtual void NativeOnActivated() override;
	virtual void NativeOnDeactivated() override;
	virtual void NativeDestruct() override;
	virtual UWidget* NativeGetDesiredFocusTarget() const override;
	virtual TOptional<FUIInputConfig> GetDesiredInputConfig() const override;

	UFUNCTION(BlueprintImplementableEvent, Category = "DS|UI")
	void BP_OnSelectionChanged(EDSHeroClass HeroClass, EDSStance Stance, EDSPartySize PartySize);

	/** Mostra os botões Solo/Procurar/Hospedar (só no menu, fora de sessão). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "DS|UI")
	bool bShowSessionButtons = true;

	// --- Classe ---
	UPROPERTY(BlueprintReadOnly, Category = "DS|UI", meta = (BindWidget))
	TObjectPtr<UCommonButtonBase> Btn_ClassPrev;

	UPROPERTY(BlueprintReadOnly, Category = "DS|UI", meta = (BindWidget))
	TObjectPtr<UCommonButtonBase> Btn_ClassNext;

	UPROPERTY(BlueprintReadOnly, Category = "DS|UI", meta = (BindWidget))
	TObjectPtr<UTextBlock> ClassNameText;

	UPROPERTY(BlueprintReadOnly, Category = "DS|UI", meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> ClassDescriptionText;

	UPROPERTY(BlueprintReadOnly, Category = "DS|UI", meta = (BindWidgetOptional))
	TObjectPtr<UImage> ClassIcon;

	// --- Stance ---
	UPROPERTY(BlueprintReadOnly, Category = "DS|UI", meta = (BindWidget))
	TObjectPtr<UCommonButtonBase> Btn_StancePrev;

	UPROPERTY(BlueprintReadOnly, Category = "DS|UI", meta = (BindWidget))
	TObjectPtr<UCommonButtonBase> Btn_StanceNext;

	UPROPERTY(BlueprintReadOnly, Category = "DS|UI", meta = (BindWidget))
	TObjectPtr<UTextBlock> StanceNameText;

	UPROPERTY(BlueprintReadOnly, Category = "DS|UI", meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> StanceDescriptionText;

	UPROPERTY(BlueprintReadOnly, Category = "DS|UI", meta = (BindWidgetOptional))
	TObjectPtr<UImage> StanceIcon;

	// --- Tamanho da party ---
	UPROPERTY(BlueprintReadOnly, Category = "DS|UI", meta = (BindWidgetOptional))
	TObjectPtr<UCommonButtonBase> Btn_PartySizePrev;

	UPROPERTY(BlueprintReadOnly, Category = "DS|UI", meta = (BindWidgetOptional))
	TObjectPtr<UCommonButtonBase> Btn_PartySizeNext;

	UPROPERTY(BlueprintReadOnly, Category = "DS|UI", meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> PartySizeText;

	// --- Ações ---
	UPROPERTY(BlueprintReadOnly, Category = "DS|UI", meta = (BindWidgetOptional))
	TObjectPtr<UCommonButtonBase> Btn_Ready;

	UPROPERTY(BlueprintReadOnly, Category = "DS|UI", meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> ReadyText;

	UPROPERTY(BlueprintReadOnly, Category = "DS|UI", meta = (BindWidgetOptional))
	TObjectPtr<UCommonButtonBase> Btn_StartSolo;

	UPROPERTY(BlueprintReadOnly, Category = "DS|UI", meta = (BindWidgetOptional))
	TObjectPtr<UCommonButtonBase> Btn_FindMatch;

	UPROPERTY(BlueprintReadOnly, Category = "DS|UI", meta = (BindWidgetOptional))
	TObjectPtr<UCommonButtonBase> Btn_Host;

	UPROPERTY(BlueprintReadOnly, Category = "DS|UI", meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> StatusText;

	/** Intervalo de leitura de bIsReady (o contrato não tem delegate de "pronto"). */
	UPROPERTY(EditDefaultsOnly, Category = "DS|UI", meta = (ClampMin = "0.1"))
	float ReadyPollInterval = 0.25f;

private:
	void BindButtons();
	void LoadInitialSelection();

	void CycleClass(int32 Direction);
	void CycleStance(int32 Direction);
	void CyclePartySize(int32 Direction);
	void HandleClassPrev() { CycleClass(-1); }
	void HandleClassNext() { CycleClass(1); }
	void HandleStancePrev() { CycleStance(-1); }
	void HandleStanceNext() { CycleStance(1); }
	void HandlePartySizePrev() { CyclePartySize(-1); }
	void HandlePartySizeNext() { CyclePartySize(1); }

	void HandleReadyClicked();
	void HandleStartSoloClicked();
	void HandleFindMatchClicked();
	void HandleHostClicked();

	void ApplySelection();
	void RefreshSelectionDisplay();
	void RefreshReadyDisplay();
	void RefreshActionButtons();
	void SetStatus(const FText& Message);

	void BindSessionDelegates();
	void UnbindSessionDelegates();

	UFUNCTION()
	void HandleSessionCreated(bool bWasSuccessful);

	UFUNCTION()
	void HandleSessionsFound(const TArray<FDSSessionInfo>& Results);

	UFUNCTION()
	void HandleSessionJoined(bool bWasSuccessful);

	TWeakObjectPtr<ADSPlayerState> PlayerState;
	EDSHeroClass SelectedClass = EDSHeroClass::None;
	EDSStance SelectedStance = EDSStance::None;
	EDSPartySize SelectedPartySize = EDSPartySize::Solo;
	FTimerHandle ReadyPollTimer;
	bool bSessionRequestInFlight = false;
};
