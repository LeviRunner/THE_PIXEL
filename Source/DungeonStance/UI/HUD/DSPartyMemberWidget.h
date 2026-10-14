#pragma once

#include "CoreMinimal.h"
#include "CommonUserWidget.h"
#include "AttributeSet.h"
#include "GameplayTagContainer.h"
#include "DSPartyMemberWidget.generated.h"

class ADSPlayerState;
class UAbilitySystemComponent;
class UDSStatusIconWidget;
class UImage;
class UPanelWidget;
class UProgressBar;
class UTextBlock;
class UWidget;
struct FOnAttributeChangeData;

/**
 * Quadro de um aliado: vida, escudo, recurso, status ativos, "caído", classe e nome.
 * Atualiza só por evento (delegates de atributo/tag do ASC e do PlayerState).
 * WBP: WBP_DSPartyMember.
 */
UCLASS(Abstract)
class DUNGEONSTANCE_API UDSPartyMemberWidget : public UCommonUserWidget
{
	GENERATED_BODY()

public:
	/** Liga o quadro a um jogador (nulo = desliga). */
	void SetPlayerState(ADSPlayerState* InPlayerState);
	ADSPlayerState* GetPlayerState() const { return PlayerState.Get(); }

protected:
	virtual void NativeDestruct() override;

	UFUNCTION(BlueprintImplementableEvent, Category = "DS|UI")
	void BP_OnDownedChanged(bool bIsDowned);

	UFUNCTION(BlueprintImplementableEvent, Category = "DS|UI")
	void BP_OnPlayerChanged(bool bIsLocalPlayer);

	UPROPERTY(BlueprintReadOnly, Category = "DS|UI", meta = (BindWidget))
	TObjectPtr<UProgressBar> HealthBar;

	UPROPERTY(BlueprintReadOnly, Category = "DS|UI", meta = (BindWidgetOptional))
	TObjectPtr<UProgressBar> ShieldBar;

	UPROPERTY(BlueprintReadOnly, Category = "DS|UI", meta = (BindWidgetOptional))
	TObjectPtr<UProgressBar> ResourceBar;

	UPROPERTY(BlueprintReadOnly, Category = "DS|UI", meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> HealthText;

	UPROPERTY(BlueprintReadOnly, Category = "DS|UI", meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> PlayerNameText;

	UPROPERTY(BlueprintReadOnly, Category = "DS|UI", meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> ClassNameText;

	UPROPERTY(BlueprintReadOnly, Category = "DS|UI", meta = (BindWidgetOptional))
	TObjectPtr<UImage> ClassIcon;

	/** Sobreposição "CAÍDO" (visível quando State.Downed ou State.Dead). */
	UPROPERTY(BlueprintReadOnly, Category = "DS|UI", meta = (BindWidgetOptional))
	TObjectPtr<UWidget> DownedOverlay;

	UPROPERTY(BlueprintReadOnly, Category = "DS|UI", meta = (BindWidgetOptional))
	TObjectPtr<UPanelWidget> StatusContainer;

	UPROPERTY(EditDefaultsOnly, Category = "DS|UI")
	TSubclassOf<UDSStatusIconWidget> StatusIconClass;

private:
	void BindToPlayer();
	void UnbindFromPlayer();

	void HandleAttributeChanged(const FOnAttributeChangeData& Data);
	void HandleStateTagChanged(const FGameplayTag Tag, int32 NewCount);
	void HandleStatusTagChanged(const FGameplayTag Tag, int32 NewCount);

	UFUNCTION()
	void HandleHeroSelectionChanged();

	void RefreshVitals();
	void RefreshIdentity();
	void RefreshDowned();
	void RefreshStatuses();

	static TArray<FGameplayAttribute> GetWatchedAttributes();
	FGameplayTagContainer GetWatchedStatusTags() const;
	static FGameplayTagContainer GetDownedTags();

	TWeakObjectPtr<ADSPlayerState> PlayerState;
	TWeakObjectPtr<UAbilitySystemComponent> AbilitySystem;
	FGameplayTagContainer BoundStatusTags;
	bool bWasDowned = false;
};
