#pragma once

#include "CoreMinimal.h"
#include "CommonUserWidget.h"
#include "Engine/TimerHandle.h"
#include "DSPartyFrameWidget.generated.h"

class ADSGameState;
class ADSPlayerState;
class UDSPartyMemberWidget;
class UPanelWidget;

/**
 * Lista de até 5 quadros de aliados (jogador local primeiro).
 * Os quadros são criados dentro de MemberContainer (Vertical/HorizontalBox), então
 * com 1 jogador sobra só um quadro e o layout não quebra.
 * WBP: WBP_DSPartyFrame.
 */
UCLASS(Abstract)
class DUNGEONSTANCE_API UDSPartyFrameWidget : public UCommonUserWidget
{
	GENERATED_BODY()

public:
	void InitParty(ADSGameState* InGameState, ADSPlayerState* InLocalPlayerState);

protected:
	virtual void NativeDestruct() override;

	UPROPERTY(BlueprintReadOnly, Category = "DS|UI", meta = (BindWidget))
	TObjectPtr<UPanelWidget> MemberContainer;

	UPROPERTY(EditDefaultsOnly, Category = "DS|UI")
	TSubclassOf<UDSPartyMemberWidget> MemberWidgetClass;

	/** Falso = mostra só os aliados (quando o jogador local tem um quadro próprio no WBP). */
	UPROPERTY(EditDefaultsOnly, Category = "DS|UI")
	bool bShowLocalPlayer = true;

	/**
	 * AGameStateBase não tem evento de entrada/saída no PlayerArray; comparamos a lista
	 * neste intervalo (barato: no máximo 5 ponteiros).
	 * TODO(Core): trocar por delegates OnPlayerAdded/OnPlayerRemoved no ADSGameState.
	 */
	UPROPERTY(EditDefaultsOnly, Category = "DS|UI", meta = (ClampMin = "0.1"))
	float RosterPollInterval = 0.5f;

private:
	void RefreshRoster();
	TArray<ADSPlayerState*> CollectPartyPlayers() const;
	bool IsSameRoster(const TArray<ADSPlayerState*>& Players) const;
	UDSPartyMemberWidget* GetOrCreateMemberWidget(int32 Index);

	TWeakObjectPtr<ADSGameState> GameState;
	TWeakObjectPtr<ADSPlayerState> LocalPlayerState;
	TArray<TWeakObjectPtr<ADSPlayerState>> ShownPlayers;
	FTimerHandle RosterTimer;

	UPROPERTY(Transient)
	TArray<TObjectPtr<UDSPartyMemberWidget>> MemberWidgets;
};
