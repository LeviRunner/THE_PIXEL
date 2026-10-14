#include "Gameplay/Abilities/DSSupportAbilityData.h"

FPrimaryAssetId UDSSupportAbilityData::GetPrimaryAssetId() const
{
	return FPrimaryAssetId(TEXT("DSSupportAbility"), GetFName());
}
