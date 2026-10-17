#include "Network/DSNetworkSettings.h"

UDSNetworkSettings::UDSNetworkSettings()
{
	CategoryName = TEXT("Game");
	SectionName = TEXT("Dungeon Stance Network");
}

int32 UDSNetworkSettings::ClampPartySize(EDSPartySize PartySize) const
{
	const int32 UpperBound = FMath::Clamp(MaxPartySize, DSConstants::MinPartySize, DSConstants::MaxPartySize);
	return FMath::Clamp(static_cast<int32>(PartySize), DSConstants::MinPartySize, UpperBound);
}

FString UDSNetworkSettings::GetLobbyMapName() const
{
	return LobbyMap.IsNull() ? FString() : LobbyMap.ToSoftObjectPath().GetLongPackageName();
}

FString UDSNetworkSettings::GetDungeonMapName() const
{
	return DungeonMap.IsNull() ? FString() : DungeonMap.ToSoftObjectPath().GetLongPackageName();
}
