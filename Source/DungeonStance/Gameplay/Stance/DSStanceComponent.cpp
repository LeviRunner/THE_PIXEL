#include "Gameplay/Stance/DSStanceComponent.h"

#include "Core/DSPlayerState.h"
#include "Gameplay/Abilities/DSAbilitySystemComponent.h"
#include "Gameplay/Cards/DSCardLibrary.h"
#include "Gameplay/Characters/DSPlayerStateBridge.h"

UDSStanceComponent::UDSStanceComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	SetIsReplicatedByDefault(false); // tudo vem replicado pelo PlayerState
}

void UDSStanceComponent::InitializeForPlayerState(ADSPlayerState* InPlayerState)
{
	if (PlayerState.Get() == InPlayerState)
	{
		return;
	}
	if (ADSPlayerState* Old = PlayerState.Get())
	{
		Old->OnCardsChanged.RemoveDynamic(this, &UDSStanceComponent::HandleCardsChanged);
	}
	PlayerState = InPlayerState;
	if (!InPlayerState)
	{
		CachedStacks.Reset();
		return;
	}
	InPlayerState->OnCardsChanged.AddUniqueDynamic(this, &UDSStanceComponent::HandleCardsChanged);
	RefreshCache();

	// Servidor: o ASC vive no PlayerState, mas um pawn novo garante que tudo está aplicado (idempotente).
	UDSAbilitySystemComponent* ASC = DSPlayerStateBridge::GetASC(InPlayerState);
	if (ASC && ASC->IsOwnerActorAuthoritative() && CachedStacks.Num() > 0)
	{
		UDSCardLibrary::ReapplyAllCards(ASC, UDSCardLibrary::GetCardRegistry(), CachedStacks);
	}
}

void UDSStanceComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (ADSPlayerState* Old = PlayerState.Get())
	{
		Old->OnCardsChanged.RemoveDynamic(this, &UDSStanceComponent::HandleCardsChanged);
	}
	Super::EndPlay(EndPlayReason);
}

void UDSStanceComponent::HandleCardsChanged()
{
	RefreshCache();
}

void UDSStanceComponent::RefreshCache()
{
	CachedStacks = DSPlayerStateBridge::GetCardStacks(PlayerState.Get());
	OnStanceCardsChanged.Broadcast();
}

EDSStance UDSStanceComponent::GetStance() const
{
	return DSPlayerStateBridge::GetStance(PlayerState.Get());
}

float UDSStanceComponent::GetCardStat(FName Stat, float Base) const
{
	return UDSCardLibrary::AggregateCardStat(UDSCardLibrary::GetCardRegistry(), CachedStacks, Stat, Base);
}

int32 UDSStanceComponent::GetCardStacks(FName CardId) const
{
	return UDSCardLibrary::GetStacks(CachedStacks, CardId);
}
