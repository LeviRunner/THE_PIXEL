#include "Core/DSPlayerState.h"

#include "Gameplay/Abilities/DSAbilitySystemComponent.h"
#include "Gameplay/Attributes/DSAttributeSet.h"
#include "Net/UnrealNetwork.h"

ADSPlayerState::ADSPlayerState(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	AbilitySystemComponent = CreateDefaultSubobject<UDSAbilitySystemComponent>(TEXT("AbilitySystemComponent"));
	AbilitySystemComponent->SetIsReplicated(true);
	// Mixed: GEs completos só para o dono; os outros recebem tags/cues mínimos.
	AbilitySystemComponent->SetReplicationMode(EGameplayEffectReplicationMode::Mixed);

	// Subobjeto do mesmo ator: o ASC encontra o AttributeSet automaticamente.
	AttributeSet = CreateDefaultSubobject<UDSAttributeSet>(TEXT("AttributeSet"));

	SetNetUpdateFrequency(PlayerStateNetUpdateFrequency);
}

void ADSPlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ADSPlayerState, HeroClass);
	DOREPLIFETIME(ADSPlayerState, Stance);
	DOREPLIFETIME(ADSPlayerState, ClassRank);
	DOREPLIFETIME(ADSPlayerState, bIsReady);
	DOREPLIFETIME(ADSPlayerState, Kills);
	DOREPLIFETIME(ADSPlayerState, RunGold);
	DOREPLIFETIME(ADSPlayerState, PendingCardPicks);
	DOREPLIFETIME(ADSPlayerState, OwnedCards);
	DOREPLIFETIME_CONDITION(ADSPlayerState, CurrentCardChoices, COND_OwnerOnly);
	DOREPLIFETIME(ADSPlayerState, RunResult);
	DOREPLIFETIME(ADSPlayerState, bHasRunResult);
}

void ADSPlayerState::CopyProperties(APlayerState* PlayerState)
{
	Super::CopyProperties(PlayerState);

	// Viagem/reconexão: mantém a escolha do herói e o rank verificado.
	if (ADSPlayerState* Target = Cast<ADSPlayerState>(PlayerState))
	{
		Target->HeroClass = HeroClass;
		Target->Stance = Stance;
		Target->ClassRank = ClassRank;
		Target->BackendPlayerId = BackendPlayerId;
	}
}

UAbilitySystemComponent* ADSPlayerState::GetAbilitySystemComponent() const
{
	return AbilitySystemComponent;
}

int32 ADSPlayerState::GetCardStacks(FName CardId) const
{
	const FDSOwnedCard* Found = OwnedCards.FindByPredicate([CardId](const FDSOwnedCard& Card) { return Card.CardId == CardId; });
	return Found ? Found->Stacks : 0;
}

FString ADSPlayerState::GetBackendPlayerId() const
{
	if (!BackendPlayerId.IsEmpty())
	{
		return BackendPlayerId;
	}
	const FUniqueNetIdRepl& NetId = GetUniqueId();
	return NetId.IsValid() ? NetId.ToString() : GetPlayerName();
}

// ---------------------------------------------------------------------------
// Setters do servidor
// ---------------------------------------------------------------------------

void ADSPlayerState::SetHeroSelection(EDSHeroClass InHeroClass, EDSStance InStance)
{
	check(HasAuthority());
	HeroClass = InHeroClass;
	Stance = InStance;
	OnRep_HeroSelection();
}

void ADSPlayerState::SetReady(bool bInReady)
{
	check(HasAuthority());
	bIsReady = bInReady;
	OnRep_HeroSelection();
}

void ADSPlayerState::SetClassRank(int32 InRank)
{
	check(HasAuthority());
	ClassRank = FMath::Clamp(InRank, 1, DSConstants::MaxClassRank);
	OnRep_HeroSelection();
}

void ADSPlayerState::SetBackendPlayerId(const FString& InPlayerId)
{
	check(HasAuthority());
	BackendPlayerId = InPlayerId;
}

void ADSPlayerState::AddKill()
{
	check(HasAuthority());
	++Kills;
}

void ADSPlayerState::AddPendingCardPicks(int32 Amount)
{
	check(HasAuthority());
	if (Amount <= 0)
	{
		return;
	}
	PendingCardPicks += Amount;
	OnRep_PendingCardPicks();
}

void ADSPlayerState::ConsumePendingCardPick()
{
	check(HasAuthority());
	PendingCardPicks = FMath::Max(0, PendingCardPicks - 1);
	OnRep_PendingCardPicks();
}

void ADSPlayerState::ClearPendingCardPicks()
{
	check(HasAuthority());
	PendingCardPicks = 0;
	OnRep_PendingCardPicks();
}

void ADSPlayerState::AddCardStack(FName CardId)
{
	check(HasAuthority());
	FDSOwnedCard* Found = OwnedCards.FindByPredicate([CardId](const FDSOwnedCard& Card) { return Card.CardId == CardId; });
	if (Found)
	{
		++Found->Stacks;
	}
	else
	{
		FDSOwnedCard NewCard;
		NewCard.CardId = CardId;
		NewCard.Stacks = 1;
		OwnedCards.Add(NewCard);
	}
	OnRep_OwnedCards();
}

void ADSPlayerState::SetCurrentCardChoices(const TArray<FName>& InChoices)
{
	check(HasAuthority());
	CurrentCardChoices = InChoices;
	OnRep_CurrentCardChoices();
}

void ADSPlayerState::AddRunGold(int32 Amount)
{
	check(HasAuthority());
	RunGold += FMath::Max(0, Amount);
}

void ADSPlayerState::SetRunResult(const FDSRunResult& InResult)
{
	check(HasAuthority());
	RunResult = InResult;
	bHasRunResult = true;
	OnRep_RunResult();
}

void ADSPlayerState::ResetRunData()
{
	check(HasAuthority());
	RunResult = FDSRunResult();
	bHasRunResult = false;
	Kills = 0;
	RunGold = 0;
	OwnedCards.Reset();
	OnRep_OwnedCards();
	ClearPendingCardPicks();
	SetCurrentCardChoices(TArray<FName>());
}

// ---------------------------------------------------------------------------
// OnRep → delegates
// ---------------------------------------------------------------------------

void ADSPlayerState::OnRep_HeroSelection()
{
	OnHeroSelectionChanged.Broadcast();
}

void ADSPlayerState::OnRep_PendingCardPicks()
{
	OnPendingPicksChanged.Broadcast(PendingCardPicks);
}

void ADSPlayerState::OnRep_OwnedCards()
{
	OnCardsChanged.Broadcast();
}

void ADSPlayerState::OnRep_CurrentCardChoices()
{
	OnCardChoicesChanged.Broadcast();
}

void ADSPlayerState::OnRep_RunResult()
{
	if (bHasRunResult)
	{
		OnRunResultChanged.Broadcast();
	}
}
