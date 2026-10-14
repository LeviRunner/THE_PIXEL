#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Core/DSTypes.h"
#include "DSStanceComponent.generated.h"

class ADSPlayerState;
class UDSCardRegistry;

DECLARE_MULTICAST_DELEGATE(FDSOnStanceCardsChanged);

/**
 * Stance (árvore de cartas escolhida antes da run) e cartas aplicadas no herói.
 * - Servidor: reaplica UDSGE_CardStats e habilidades de apoio a partir do ADSPlayerState
 *   (novo pawn / reconexão). A escolha em si passa por UDSCardLibrary::ApplyCardToPlayer.
 * - Todas as máquinas: GetCardStat lê stats de gatilho (crit_heal_pct, support_power...)
 *   das cartas replicadas no PlayerState, como PlayerPerks.gd/aggregateCardStat.
 */
UCLASS(ClassGroup = (DungeonStance), meta = (BlueprintSpawnableComponent))
class DUNGEONSTANCE_API UDSStanceComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UDSStanceComponent();

	/** Chamado pelo herói quando o PlayerState fica disponível (PossessedBy / OnRep_PlayerState). */
	void InitializeForPlayerState(ADSPlayerState* InPlayerState);

	UFUNCTION(BlueprintPure, Category = "DS|Stance")
	EDSStance GetStance() const;

	/** (Base + Σadd) × Πmult do stat sobre as cartas possuídas. */
	UFUNCTION(BlueprintPure, Category = "DS|Stance")
	float GetCardStat(FName Stat, float Base = 0.f) const;

	UFUNCTION(BlueprintPure, Category = "DS|Stance")
	int32 GetCardStacks(FName CardId) const;

	const TMap<FName, int32>& GetAllCardStacks() const { return CachedStacks; }

	FDSOnStanceCardsChanged OnStanceCardsChanged;

protected:
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	UFUNCTION()
	void HandleCardsChanged();

private:
	void RefreshCache();

	TWeakObjectPtr<ADSPlayerState> PlayerState;
	TMap<FName, int32> CachedStacks;
};
