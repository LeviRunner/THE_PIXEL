#include "Network/DSMatchmakingSubsystem.h"

#include "DungeonStance.h"
#include "Network/DSNetworkSettings.h"
#include "Network/DSSessionSubsystem.h"
#include "Engine/GameInstance.h"
#include "GameFramework/PlayerController.h"

void UDSMatchmakingSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	UDSSessionSubsystem* Sessions = Collection.InitializeDependency<UDSSessionSubsystem>();
	if (!Sessions)
	{
		UE_LOG(LogDungeonStance, Error, TEXT("[Matchmaking] UDSSessionSubsystem indisponível."));
		return;
	}
	Sessions->OnSessionsFound.AddUniqueDynamic(this, &ThisClass::HandleSessionsFound);
	Sessions->OnSessionJoined.AddUniqueDynamic(this, &ThisClass::HandleSessionJoined);
	Sessions->OnSessionCreated.AddUniqueDynamic(this, &ThisClass::HandleSessionCreated);
}

void UDSMatchmakingSubsystem::Deinitialize()
{
	if (UDSSessionSubsystem* Sessions = GetSessionSubsystem())
	{
		Sessions->OnSessionsFound.RemoveDynamic(this, &ThisClass::HandleSessionsFound);
		Sessions->OnSessionJoined.RemoveDynamic(this, &ThisClass::HandleSessionJoined);
		Sessions->OnSessionCreated.RemoveDynamic(this, &ThisClass::HandleSessionCreated);
	}
	CancelMatchmaking();
	Super::Deinitialize();
}

UDSSessionSubsystem* UDSMatchmakingSubsystem::GetSessionSubsystem() const
{
	const UGameInstance* GameInstance = GetGameInstance();
	return GameInstance ? GameInstance->GetSubsystem<UDSSessionSubsystem>() : nullptr;
}

bool UDSMatchmakingSubsystem::IsMatchmaking() const
{
	return State == EDSMatchmakingState::Searching || State == EDSMatchmakingState::Joining
		|| State == EDSMatchmakingState::Hosting || State == EDSMatchmakingState::WaitingForTicket;
}

void UDSMatchmakingSubsystem::SetTicketService(UObject* ServiceObject)
{
	if (ServiceObject && !ServiceObject->Implements<UDSMatchmakerTicketService>())
	{
		UE_LOG(LogDungeonStance, Error, TEXT("[Matchmaking] %s não implementa IDSMatchmakerTicketService."), *GetNameSafe(ServiceObject));
		return;
	}
	TicketService = ServiceObject ? TScriptInterface<IDSMatchmakerTicketService>(ServiceObject) : TScriptInterface<IDSMatchmakerTicketService>();
}

// --- Entrada ------------------------------------------------------------

void UDSMatchmakingSubsystem::StartMatchmaking(EDSPartySize PartySize, int32 GroupSize)
{
	FDSMatchmakingTicket Ticket;
	Ticket.PartySize = PartySize;
	Ticket.GroupSize = GroupSize;
	StartMatchmakingWithTicket(Ticket);
}

void UDSMatchmakingSubsystem::StartMatchmakingWithTicket(const FDSMatchmakingTicket& Ticket)
{
	if (IsMatchmaking())
	{
		UE_LOG(LogDungeonStance, Warning, TEXT("[Matchmaking] Já está na fila."));
		return;
	}
	UDSSessionSubsystem* Sessions = GetSessionSubsystem();
	if (!Sessions)
	{
		Finish(false, false);
		return;
	}

	const int32 PartyPlayers = UDSNetworkSettings::Get()->ClampPartySize(Ticket.PartySize);
	if (Ticket.GroupSize < 1 || Ticket.GroupSize > PartyPlayers)
	{
		UE_LOG(LogDungeonStance, Error, TEXT("[Matchmaking] Grupo de %d não cabe numa party de %d."), Ticket.GroupSize, PartyPlayers);
		Finish(false, false);
		return;
	}

	ActiveTicket = Ticket;

	// Solo, ou grupo que já completa a party: não há com quem casar.
	if (PartyPlayers == DSConstants::MinPartySize)
	{
		SetState(EDSMatchmakingState::Matched);
		Sessions->StartSoloRun();
		Finish(true, true);
		return;
	}
	if (Ticket.GroupSize == PartyPlayers)
	{
		HostFallback();
		return;
	}

	if (TicketService.GetInterface() != nullptr)
	{
		SetState(EDSMatchmakingState::WaitingForTicket);
		const bool bQueued = TicketService->SubmitTicket(Ticket, FDSOnTicketMatched::CreateUObject(this, &ThisClass::HandleTicketMatched));
		if (!bQueued)
		{
			UE_LOG(LogDungeonStance, Warning, TEXT("[Matchmaking] Serviço de ticket recusou o pedido; usando busca de sessões."));
		}
		else
		{
			return;
		}
	}

	SetState(EDSMatchmakingState::Searching);
	Sessions->FindSessions(Ticket.PartySize);
}

void UDSMatchmakingSubsystem::CancelMatchmaking()
{
	if (!IsMatchmaking())
	{
		return;
	}
	if (State == EDSMatchmakingState::WaitingForTicket && TicketService.GetInterface() != nullptr)
	{
		TicketService->CancelTicket();
	}
	// Busca/join/host em andamento terminam no subsistema de sessão; os callbacks são ignorados em Idle.
	UE_LOG(LogDungeonStance, Log, TEXT("[Matchmaking] Cancelado."));
	SetState(EDSMatchmakingState::Idle);
}

// --- Fluxo de sessões ---------------------------------------------------

int32 UDSMatchmakingSubsystem::PickBestSession(const TArray<FDSSessionInfo>& Sessions) const
{
	// Prefere a sessão mais cheia (fecha parties mais rápido) e, no empate, a de menor ping.
	const FDSSessionInfo* Best = nullptr;
	for (const FDSSessionInfo& Info : Sessions)
	{
		if (Info.GetOpenSlots() < ActiveTicket.GroupSize)
		{
			continue;
		}
		const bool bBetter = !Best || Info.CurrentPlayers > Best->CurrentPlayers
			|| (Info.CurrentPlayers == Best->CurrentPlayers && Info.PingMs < Best->PingMs);
		if (bBetter)
		{
			Best = &Info;
		}
	}
	return Best ? Best->ResultIndex : INDEX_NONE;
}

void UDSMatchmakingSubsystem::HandleSessionsFound(const TArray<FDSSessionInfo>& Sessions)
{
	if (State != EDSMatchmakingState::Searching)
	{
		return; // Busca disparada pela UI (lista de partidas), não pelo matchmaking.
	}

	const int32 ResultIndex = PickBestSession(Sessions);
	UDSSessionSubsystem* SessionSubsystem = GetSessionSubsystem();
	if (ResultIndex == INDEX_NONE || !SessionSubsystem)
	{
		HostFallback();
		return;
	}

	SetState(EDSMatchmakingState::Joining);
	SessionSubsystem->JoinSession(ResultIndex);
}

void UDSMatchmakingSubsystem::HandleSessionJoined(bool bWasSuccessful)
{
	if (State != EDSMatchmakingState::Joining)
	{
		return;
	}
	if (!bWasSuccessful)
	{
		UE_LOG(LogDungeonStance, Warning, TEXT("[Matchmaking] Join falhou (sessão cheia/fechou); hospedando."));
		HostFallback();
		return;
	}
	SetState(EDSMatchmakingState::Matched);
	Finish(true, false);
}

void UDSMatchmakingSubsystem::HostFallback()
{
	UDSSessionSubsystem* Sessions = GetSessionSubsystem();
	if (!Sessions)
	{
		Finish(false, false);
		return;
	}
	SetState(EDSMatchmakingState::Hosting);
	Sessions->HostSession(ActiveTicket.PartySize, Sessions->ShouldUseLAN());
}

void UDSMatchmakingSubsystem::HandleSessionCreated(bool bWasSuccessful)
{
	if (State != EDSMatchmakingState::Hosting)
	{
		return;
	}
	SetState(bWasSuccessful ? EDSMatchmakingState::Matched : EDSMatchmakingState::Failed);
	Finish(bWasSuccessful, true);
}

// --- Fluxo de tickets ---------------------------------------------------

void UDSMatchmakingSubsystem::HandleTicketMatched(bool bSuccess, const FString& ConnectStringOrError)
{
	if (State != EDSMatchmakingState::WaitingForTicket)
	{
		return;
	}

	const UGameInstance* GameInstance = GetGameInstance();
	APlayerController* PlayerController = GameInstance ? GameInstance->GetFirstLocalPlayerController() : nullptr;
	if (!bSuccess || ConnectStringOrError.IsEmpty() || !PlayerController)
	{
		UE_LOG(LogDungeonStance, Warning, TEXT("[Matchmaking] Ticket falhou: %s"), *ConnectStringOrError.Left(256));
		SetState(EDSMatchmakingState::Failed);
		Finish(false, false);
		return;
	}

	SetState(EDSMatchmakingState::Matched);
	PlayerController->ClientTravel(ConnectStringOrError, TRAVEL_Absolute);
	Finish(true, false);
}

// --- Estado -------------------------------------------------------------

void UDSMatchmakingSubsystem::SetState(EDSMatchmakingState NewState)
{
	if (State == NewState)
	{
		return;
	}
	State = NewState;
	OnMatchmakingStateChanged.Broadcast(NewState);
}

void UDSMatchmakingSubsystem::Finish(bool bSuccess, bool bIsHost)
{
	UE_LOG(LogDungeonStance, Log, TEXT("[Matchmaking] Fim: sucesso=%d, host=%d."), bSuccess ? 1 : 0, bIsHost ? 1 : 0);
	if (!bSuccess)
	{
		SetState(EDSMatchmakingState::Failed);
	}
	OnMatchmakingFinished.Broadcast(bSuccess, bIsHost);
}
