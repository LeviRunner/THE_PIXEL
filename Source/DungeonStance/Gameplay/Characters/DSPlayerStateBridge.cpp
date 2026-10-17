#include "Gameplay/Characters/DSPlayerStateBridge.h"

#include "Core/DSPlayerState.h"
#include "Gameplay/Abilities/DSAbilitySystemComponent.h"
#include "Gameplay/Attributes/DSAttributeSet.h"

namespace DSPlayerStateBridge
{
	ADSPlayerState* AsDSPlayerState(APlayerState* PlayerState)
	{
		return Cast<ADSPlayerState>(PlayerState);
	}

	UDSAbilitySystemComponent* GetASC(const ADSPlayerState* PlayerState)
	{
		return PlayerState ? Cast<UDSAbilitySystemComponent>(PlayerState->GetAbilitySystemComponent()) : nullptr;
	}

	UDSAttributeSet* GetAttributeSet(const ADSPlayerState* PlayerState)
	{
		return PlayerState ? PlayerState->GetAttributeSet() : nullptr;
	}

	EDSHeroClass GetHeroClass(const ADSPlayerState* PlayerState)
	{
		return PlayerState ? PlayerState->GetHeroClass() : EDSHeroClass::None;
	}

	EDSStance GetStance(const ADSPlayerState* PlayerState)
	{
		return PlayerState ? PlayerState->GetStance() : EDSStance::None;
	}

	int32 GetClassRank(const ADSPlayerState* PlayerState)
	{
		return PlayerState ? PlayerState->GetClassRank() : 1;
	}

	int32 GetPendingCardPicks(const ADSPlayerState* PlayerState)
	{
		return PlayerState ? PlayerState->GetPendingCardPicks() : 0;
	}

	TArray<FName> GetCurrentCardChoices(const ADSPlayerState* PlayerState)
	{
		return PlayerState ? TArray<FName>(PlayerState->GetCurrentCardChoices()) : TArray<FName>();
	}

	TMap<FName, int32> GetCardStacks(const ADSPlayerState* PlayerState)
	{
		return PlayerState ? ToStackMap(PlayerState->GetOwnedCards()) : TMap<FName, int32>();
	}

	APawn* GetPawn(const ADSPlayerState* PlayerState)
	{
		return PlayerState ? PlayerState->GetPawn() : nullptr;
	}

	TMap<FName, int32> ToStackMap(const TArray<FDSOwnedCard>& OwnedCards)
	{
		TMap<FName, int32> Stacks;
		for (const FDSOwnedCard& Card : OwnedCards)
		{
			if (!Card.CardId.IsNone() && Card.Stacks > 0)
			{
				Stacks.FindOrAdd(Card.CardId) += Card.Stacks;
			}
		}
		return Stacks;
	}
}
