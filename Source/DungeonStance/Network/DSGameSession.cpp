#include "Network/DSGameSession.h"

#include "DungeonStance.h"
#include "Core/DSTypes.h"
#include "Network/DSNetworkSettings.h"
#include "Network/DSSessionTypes.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "OnlineSessionSettings.h"
#include "OnlineSubsystemUtils.h"
#include "Kismet/GameplayStatics.h"

void ADSGameSession::InitOptions(const FString& Options)
{
	Super::InitOptions(Options);

	int32 PartySize = UGameplayStatics::GetIntOption(Options, DSSessionKeys::PartySizeUrlOption(), 0);
	if (PartySize <= 0)
	{
		PartySize = ReadPartySizeFromNamedSession();
	}
	if (PartySize <= 0)
	{
		PartySize = UDSNetworkSettings::Get()->MaxPartySize;
	}

	MaxPlayers = FMath::Clamp(PartySize, DSConstants::MinPartySize, DSConstants::MaxPartySize);
	MaxSpectators = 0;
	UE_LOG(LogDungeonStance, Log, TEXT("[GameSession] MaxPlayers = %d."), MaxPlayers);
}

int32 ADSGameSession::ReadPartySizeFromNamedSession() const
{
	const IOnlineSessionPtr Sessions = Online::GetSessionInterface(GetWorld());
	const FNamedOnlineSession* Session = Sessions.IsValid() ? Sessions->GetNamedSession(SessionName) : nullptr;
	return Session ? Session->SessionSettings.NumPublicConnections : 0;
}
