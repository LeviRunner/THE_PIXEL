#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "AbilitySystemInterface.h"
#include "Core/DSTypes.h"
#include "DSPlayerState.generated.h"

class UAbilitySystemComponent;
class UDSAbilitySystemComponent;
class UDSAttributeSet;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FDSOnPendingPicksChanged, int32, PendingPicks);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FDSOnPlayerStateChanged);

/**
 * Estado replicado de um jogador na run (classe, stance, rank, cartas, kills)
 * e dono do ASC do herói (o herói pega o ASC daqui em PossessedBy/OnRep_PlayerState,
 * então o ASC sobrevive à morte/troca de pawn).
 *
 * Todos os setters (Set..., Add...) são só servidor; eles chamam o OnRep
 * manualmente para o host de listen server também receber os delegates.
 */
UCLASS()
class DUNGEONSTANCE_API ADSPlayerState : public APlayerState, public IAbilitySystemInterface
{
	GENERATED_BODY()

public:
	ADSPlayerState(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void CopyProperties(APlayerState* PlayerState) override;

	// ----- IAbilitySystemInterface -----
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;

	UFUNCTION(BlueprintPure, Category = "DungeonStance|Player")
	UDSAbilitySystemComponent* GetDSAbilitySystemComponent() const { return AbilitySystemComponent; }

	UFUNCTION(BlueprintPure, Category = "DungeonStance|Player")
	UDSAttributeSet* GetAttributeSet() const { return AttributeSet; }

	// ----- Getters -----
	UFUNCTION(BlueprintPure, Category = "DungeonStance|Player")
	EDSHeroClass GetHeroClass() const { return HeroClass; }

	UFUNCTION(BlueprintPure, Category = "DungeonStance|Player")
	EDSStance GetStance() const { return Stance; }

	UFUNCTION(BlueprintPure, Category = "DungeonStance|Player")
	int32 GetClassRank() const { return ClassRank; }

	UFUNCTION(BlueprintPure, Category = "DungeonStance|Player")
	int32 GetKills() const { return Kills; }

	UFUNCTION(BlueprintPure, Category = "DungeonStance|Player")
	int32 GetPendingCardPicks() const { return PendingCardPicks; }

	const TArray<FDSOwnedCard>& GetOwnedCards() const { return OwnedCards; }

	UFUNCTION(BlueprintPure, Category = "DungeonStance|Player", meta = (DisplayName = "Get Owned Cards"))
	TArray<FDSOwnedCard> K2_GetOwnedCards() const { return OwnedCards; }

	UFUNCTION(BlueprintPure, Category = "DungeonStance|Player")
	int32 GetCardStacks(FName CardId) const;

	/** Só é replicado para o dono; nos outros clientes fica vazio. */
	const TArray<FName>& GetCurrentCardChoices() const { return CurrentCardChoices; }

	UFUNCTION(BlueprintPure, Category = "DungeonStance|Player", meta = (DisplayName = "Get Current Card Choices"))
	TArray<FName> K2_GetCurrentCardChoices() const { return CurrentCardChoices; }

	UFUNCTION(BlueprintPure, Category = "DungeonStance|Player")
	bool IsReady() const { return bIsReady; }

	UFUNCTION(BlueprintPure, Category = "DungeonStance|Player")
	int32 GetRunGold() const { return RunGold; }

	/** Resultado final da run calculado pelo servidor (válido quando HasRunResult()). */
	UFUNCTION(BlueprintPure, Category = "DungeonStance|Player")
	FDSRunResult GetRunResult() const { return RunResult; }

	UFUNCTION(BlueprintPure, Category = "DungeonStance|Player")
	bool HasRunResult() const { return bHasRunResult; }

	/** Id do jogador no backend (para FDSRunResult). Cai no UniqueNetId / nome se não houver. */
	UFUNCTION(BlueprintPure, Category = "DungeonStance|Player")
	FString GetBackendPlayerId() const;

	// ----- Só servidor -----
	void SetHeroSelection(EDSHeroClass InHeroClass, EDSStance InStance);
	void SetReady(bool bInReady);
	void SetClassRank(int32 InRank);
	void SetBackendPlayerId(const FString& InPlayerId);
	void AddKill();
	void AddPendingCardPicks(int32 Amount);
	void ConsumePendingCardPick();
	void ClearPendingCardPicks();
	void AddCardStack(FName CardId);
	void SetCurrentCardChoices(const TArray<FName>& InChoices);
	void AddRunGold(int32 Amount);
	void SetRunResult(const FDSRunResult& InResult);
	/** Limpa os dados da run (cartas, kills, ouro); mantém classe, stance e rank. */
	void ResetRunData();

	// ----- Delegates -----
	UPROPERTY(BlueprintAssignable, Category = "DungeonStance|Player")
	FDSOnPendingPicksChanged OnPendingPicksChanged;

	UPROPERTY(BlueprintAssignable, Category = "DungeonStance|Player")
	FDSOnPlayerStateChanged OnCardsChanged;

	UPROPERTY(BlueprintAssignable, Category = "DungeonStance|Player")
	FDSOnPlayerStateChanged OnCardChoicesChanged;

	/** O servidor publicou o FDSRunResult deste jogador. */
	UPROPERTY(BlueprintAssignable, Category = "DungeonStance|Player")
	FDSOnPlayerStateChanged OnRunResultChanged;

	/** Classe, stance, rank ou "pronto" mudaram. */
	UPROPERTY(BlueprintAssignable, Category = "DungeonStance|Player")
	FDSOnPlayerStateChanged OnHeroSelectionChanged;

protected:
	UFUNCTION()
	void OnRep_HeroSelection();

	UFUNCTION()
	void OnRep_PendingCardPicks();

	UFUNCTION()
	void OnRep_OwnedCards();

	UFUNCTION()
	void OnRep_CurrentCardChoices();

	UFUNCTION()
	void OnRep_RunResult();

	/** Frequência alta: o ASC (atributos/GEs) replica por aqui. */
	static constexpr float PlayerStateNetUpdateFrequency = 100.f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "DungeonStance|Abilities")
	TObjectPtr<UDSAbilitySystemComponent> AbilitySystemComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "DungeonStance|Abilities")
	TObjectPtr<UDSAttributeSet> AttributeSet;

	UPROPERTY(ReplicatedUsing = OnRep_HeroSelection)
	EDSHeroClass HeroClass = EDSHeroClass::None;

	UPROPERTY(ReplicatedUsing = OnRep_HeroSelection)
	EDSStance Stance = EDSStance::None;

	UPROPERTY(ReplicatedUsing = OnRep_HeroSelection)
	int32 ClassRank = 1;

	UPROPERTY(ReplicatedUsing = OnRep_HeroSelection)
	bool bIsReady = false;

	UPROPERTY(Replicated)
	int32 Kills = 0;

	UPROPERTY(Replicated)
	int32 RunGold = 0;

	UPROPERTY(ReplicatedUsing = OnRep_PendingCardPicks)
	int32 PendingCardPicks = 0;

	UPROPERTY(ReplicatedUsing = OnRep_OwnedCards)
	TArray<FDSOwnedCard> OwnedCards;

	UPROPERTY(ReplicatedUsing = OnRep_CurrentCardChoices)
	TArray<FName> CurrentCardChoices;

	UPROPERTY(ReplicatedUsing = OnRep_RunResult)
	FDSRunResult RunResult;

	UPROPERTY(ReplicatedUsing = OnRep_RunResult)
	bool bHasRunResult = false;

	/** Só servidor (não replicado). */
	FString BackendPlayerId;
};
