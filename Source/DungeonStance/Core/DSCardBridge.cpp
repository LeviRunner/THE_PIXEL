#include "Core/DSCardBridge.h"

#include "Core/DSPlayerState.h"
#include "Gameplay/Cards/DSCardLibrary.h"
#include "Math/RandomStream.h"

namespace DSCardBridge
{
	TArray<FName> RollChoices(const ADSPlayerState& Player, int32 NumChoices, FRandomStream& Random)
	{
		return UDSCardLibrary::RollCardChoices(Player.GetStance(), Player.GetOwnedCards(), NumChoices, Random);
	}

	bool ValidatePick(const ADSPlayerState& Player, FName CardId, FString& OutReason)
	{
		return UDSCardLibrary::ValidateCardPick(CardId, Player.GetStance(), Player.GetOwnedCards(), Player.GetPendingCardPicks(), OutReason);
	}

	bool ApplyToPlayer(ADSPlayerState& Player, FName CardId)
	{
		return UDSCardLibrary::ApplyCardToPlayer(&Player, CardId);
	}
}
