#include "Network/DSSessionSubsystem.h"

#include "DungeonStance.h"
#include "Network/DSNetworkSettings.h"
#include "OnlineSubsystem.h"
#include "OnlineSubsystemUtils.h"
#include "Engine/Engine.h"
#include "Engine/GameInstance.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"

namespace DSSessionPrivate
{
	/** Usuário local que hospeda/entra (jogo de um jogador por máquina). */
	constexpr int32 LocalUserNum = 0;

	EDSPartySize ToPartySize(int32 NumPlayers)
	{
		return static_cast<EDSPartySize>(FMath::Clamp(NumPlayers, DSConstants::MinPartySize, DSConstants::MaxPartySize));
	}

	FString BuildTravelOptions(int32 NumPlayers, bool bListen)
	{
		const FString PartyOption = FString::Printf(TEXT("%s=%d"), DSSessionKeys::PartySizeUrlOption(), NumPlayers);
		return bListen ? FString(TEXT("listen?")) + PartyOption : PartyOption;
	}
}

// --- Ciclo de vida ------------------------------------------------------

void UDSSessionSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	if (GEngine)
	{
		NetworkFailureHandle = GEngine->OnNetworkFailure().AddUObject(this, &ThisClass::HandleNetworkFailure);
	}
}

void UDSSessionSubsystem::Deinitialize()
{
	if (GEngine)
	{
		GEngine->OnNetworkFailure().Remove(NetworkFailureHandle);
	}
	if (const UGameInstance* GameInstance = GetGameInstance())
	{
		GameInstance->GetTimerManager().ClearTimer(FindTimeoutHandle);
	}
	ClearSessionDelegates();
	PendingOp = EPendingOp::None;

	Super::Deinitialize();
}

UWorld* UDSSessionSubsystem::GetGameWorld() const
{
	const UGameInstance* GameInstance = GetGameInstance();
	return GameInstance ? GameInstance->GetWorld() : nullptr;
}

IOnlineSessionPtr UDSSessionSubsystem::GetSessionInterface() const
{
	const IOnlineSubsystem* OnlineSubsystem = Online::GetSubsystem(GetGameWorld());
	if (!OnlineSubsystem)
	{
		UE_LOG(LogDungeonStance, Error, TEXT("[Session] Nenhum OnlineSubsystem ativo (confira [OnlineSubsystem] no DefaultEngine.ini)."));
		return nullptr;
	}

	IOnlineSessionPtr Sessions = OnlineSubsystem->GetSessionInterface();
	if (!Sessions.IsValid())
	{
		UE_LOG(LogDungeonStance, Error, TEXT("[Session] O subsistema %s não expõe IOnlineSession."), *OnlineSubsystem->GetSubsystemName().ToString());
	}
	return Sessions;
}

bool UDSSessionSubsystem::HasActiveSession() const
{
	const IOnlineSessionPtr Sessions = GetSessionInterface();
	return Sessions.IsValid() && Sessions->GetNamedSession(NAME_GameSession) != nullptr;
}

bool UDSSessionSubsystem::ShouldUseLAN() const
{
	const IOnlineSubsystem* OnlineSubsystem = Online::GetSubsystem(GetGameWorld());
	const bool bIsNull = OnlineSubsystem && OnlineSubsystem->GetSubsystemName() == NULL_SUBSYSTEM;
	return bIsNull && UDSNetworkSettings::Get()->bForceLANWithNullSubsystem;
}

void UDSSessionSubsystem::ClearSessionDelegates()
{
	const IOnlineSessionPtr Sessions = GetSessionInterface();
	if (!Sessions.IsValid())
	{
		return;
	}
	Sessions->ClearOnCreateSessionCompleteDelegate_Handle(CreateCompleteHandle);
	Sessions->ClearOnFindSessionsCompleteDelegate_Handle(FindCompleteHandle);
	Sessions->ClearOnJoinSessionCompleteDelegate_Handle(JoinCompleteHandle);
	Sessions->ClearOnDestroySessionCompleteDelegate_Handle(DestroyCompleteHandle);
}

// --- Host ---------------------------------------------------------------

void UDSSessionSubsystem::HostSession(EDSPartySize PartySize, bool bIsLAN)
{
	if (IsBusy())
	{
		UE_LOG(LogDungeonStance, Warning, TEXT("[Session] HostSession ignorado: outra operação em andamento."));
		OnSessionCreated.Broadcast(false);
		return;
	}

	const IOnlineSessionPtr Sessions = GetSessionInterface();
	if (!Sessions.IsValid())
	{
		OnSessionCreated.Broadcast(false);
		return;
	}

	if (Sessions->GetNamedSession(NAME_GameSession))
	{
		FAfterDestroy Next;
		Next.Op = EPendingOp::Creating;
		Next.PartySize = PartySize;
		Next.bIsLAN = bIsLAN;
		DestroyThen(Next);
		return;
	}

	HostedPlayerCount = UDSNetworkSettings::Get()->ClampPartySize(PartySize);
	const FOnlineSessionSettings Settings = BuildSessionSettings(PartySize, bIsLAN);

	CreateCompleteHandle = Sessions->AddOnCreateSessionCompleteDelegate_Handle(
		FOnCreateSessionCompleteDelegate::CreateUObject(this, &ThisClass::HandleCreateSessionComplete));
	PendingOp = EPendingOp::Creating;

	UE_LOG(LogDungeonStance, Log, TEXT("[Session] Criando sessão: %d jogadores, LAN=%d."), HostedPlayerCount, bIsLAN ? 1 : 0);

	// Alguns subsistemas disparam o delegate antes de retornar false: só falha aqui se ainda pendente.
	if (!Sessions->CreateSession(DSSessionPrivate::LocalUserNum, NAME_GameSession, Settings) && PendingOp == EPendingOp::Creating)
	{
		HandleCreateSessionComplete(NAME_GameSession, false);
	}
}

FOnlineSessionSettings UDSSessionSubsystem::BuildSessionSettings(EDSPartySize PartySize, bool bIsLAN) const
{
	const UDSNetworkSettings* NetSettings = UDSNetworkSettings::Get();
	const int32 NumPlayers = NetSettings->ClampPartySize(PartySize);
	const bool bIsDedicated = IsRunningDedicatedServer();

	FOnlineSessionSettings Settings;
	Settings.NumPublicConnections = NumPlayers;
	Settings.NumPrivateConnections = 0;
	Settings.bIsLANMatch = bIsLAN;
	Settings.bIsDedicated = bIsDedicated;
	Settings.bShouldAdvertise = true;
	Settings.bAllowJoinInProgress = true;
	Settings.bAllowInvites = true;
	Settings.bUsesPresence = !bIsDedicated && NetSettings->bUsePresence;
	Settings.bAllowJoinViaPresence = Settings.bUsesPresence;
	Settings.bUseLobbiesIfAvailable = Settings.bUsesPresence;

	const EOnlineDataAdvertisementType::Type Advertise = EOnlineDataAdvertisementType::ViaOnlineServiceAndPing;
	Settings.Set(DSSessionKeys::PartySize(), NumPlayers, Advertise);
	Settings.Set(DSSessionKeys::GameMode(), NetSettings->GameModeTag, Advertise);
	Settings.Set(SETTING_MAPNAME, NetSettings->GetLobbyMapName(), Advertise);
	return Settings;
}

void UDSSessionSubsystem::HandleCreateSessionComplete(FName SessionName, bool bWasSuccessful)
{
	if (const IOnlineSessionPtr Sessions = GetSessionInterface())
	{
		Sessions->ClearOnCreateSessionCompleteDelegate_Handle(CreateCompleteHandle);
	}
	PendingOp = EPendingOp::None;

	UE_LOG(LogDungeonStance, Log, TEXT("[Session] CreateSession(%s) concluído: %s."), *SessionName.ToString(), bWasSuccessful ? TEXT("ok") : TEXT("FALHOU"));
	OnSessionCreated.Broadcast(bWasSuccessful);

	// Servidor dedicado já está no mapa certo; o host listen abre o lobby.
	if (bWasSuccessful && !IsRunningDedicatedServer())
	{
		TravelToLobbyAsHost(HostedPlayerCount);
	}
}

void UDSSessionSubsystem::TravelToLobbyAsHost(int32 NumPlayers)
{
	const FString MapName = UDSNetworkSettings::Get()->GetLobbyMapName();
	if (MapName.IsEmpty())
	{
		UE_LOG(LogDungeonStance, Error, TEXT("[Session] LobbyMap não configurado em Project Settings > Dungeon Stance Network."));
		return;
	}
	UGameplayStatics::OpenLevel(GetGameWorld(), FName(*MapName), true, DSSessionPrivate::BuildTravelOptions(NumPlayers, true));
}

// --- Busca --------------------------------------------------------------

void UDSSessionSubsystem::FindSessions(EDSPartySize PartySize)
{
	const IOnlineSessionPtr Sessions = GetSessionInterface();
	if (IsBusy() || !Sessions.IsValid())
	{
		UE_LOG(LogDungeonStance, Warning, TEXT("[Session] FindSessions ignorado (ocupado ou sem IOnlineSession)."));
		OnSessionsFound.Broadcast(TArray<FDSSessionInfo>());
		return;
	}

	const UDSNetworkSettings* NetSettings = UDSNetworkSettings::Get();
	SearchPlayerCount = NetSettings->ClampPartySize(PartySize);

	SessionSearch = MakeShared<FOnlineSessionSearch>();
	SessionSearch->MaxSearchResults = NetSettings->MaxSearchResults;
	SessionSearch->bIsLanQuery = ShouldUseLAN();
	if (NetSettings->bUsePresence && !SessionSearch->bIsLanQuery)
	{
#if defined(SEARCH_LOBBIES)
		SessionSearch->QuerySettings.Set(SEARCH_LOBBIES, true, EOnlineComparisonOp::Equals);
#else
		SessionSearch->QuerySettings.Set(SEARCH_PRESENCE, true, EOnlineComparisonOp::Equals);
#endif
	}
	// Filtro no serviço (Steam/EOS). O Null ignora; refiltramos em BuildSessionInfos.
	SessionSearch->QuerySettings.Set(DSSessionKeys::PartySize(), SearchPlayerCount, EOnlineComparisonOp::Equals);

	FindCompleteHandle = Sessions->AddOnFindSessionsCompleteDelegate_Handle(
		FOnFindSessionsCompleteDelegate::CreateUObject(this, &ThisClass::HandleFindSessionsComplete));
	PendingOp = EPendingOp::Finding;

	GetGameInstance()->GetTimerManager().SetTimer(FindTimeoutHandle, FTimerDelegate::CreateUObject(this, &ThisClass::HandleFindTimeout),
		NetSettings->FindSessionsTimeoutSeconds, false);

	UE_LOG(LogDungeonStance, Log, TEXT("[Session] Buscando sessões de %d jogadores (LAN=%d)."), SearchPlayerCount, SessionSearch->bIsLanQuery ? 1 : 0);
	if (!Sessions->FindSessions(DSSessionPrivate::LocalUserNum, SessionSearch.ToSharedRef()) && PendingOp == EPendingOp::Finding)
	{
		FinishFind(false);
	}
}

void UDSSessionSubsystem::HandleFindSessionsComplete(bool bWasSuccessful)
{
	if (PendingOp == EPendingOp::Finding)
	{
		FinishFind(bWasSuccessful);
	}
}

void UDSSessionSubsystem::HandleFindTimeout()
{
	if (PendingOp != EPendingOp::Finding)
	{
		return;
	}
	UE_LOG(LogDungeonStance, Warning, TEXT("[Session] Busca expirou após %.1fs; tratando como vazia."), UDSNetworkSettings::Get()->FindSessionsTimeoutSeconds);
	if (const IOnlineSessionPtr Sessions = GetSessionInterface())
	{
		Sessions->CancelFindSessions();
	}
	FinishFind(false);
}

void UDSSessionSubsystem::FinishFind(bool bWasSuccessful)
{
	if (const IOnlineSessionPtr Sessions = GetSessionInterface())
	{
		Sessions->ClearOnFindSessionsCompleteDelegate_Handle(FindCompleteHandle);
	}
	if (const UGameInstance* GameInstance = GetGameInstance())
	{
		GameInstance->GetTimerManager().ClearTimer(FindTimeoutHandle);
	}
	PendingOp = EPendingOp::None;

	LastResults = bWasSuccessful ? BuildSessionInfos() : TArray<FDSSessionInfo>();
	UE_LOG(LogDungeonStance, Log, TEXT("[Session] Busca concluída (%s): %d sessões compatíveis."),
		bWasSuccessful ? TEXT("ok") : TEXT("falha"), LastResults.Num());
	OnSessionsFound.Broadcast(LastResults);
}

TArray<FDSSessionInfo> UDSSessionSubsystem::BuildSessionInfos() const
{
	TArray<FDSSessionInfo> Infos;
	if (!SessionSearch.IsValid())
	{
		return Infos;
	}

	for (int32 Index = 0; Index < SessionSearch->SearchResults.Num(); ++Index)
	{
		const FOnlineSessionSearchResult& Result = SessionSearch->SearchResults[Index];
		int32 AdvertisedPlayers = 0;
		if (!Result.IsValid() || !Result.Session.SessionSettings.Get(DSSessionKeys::PartySize(), AdvertisedPlayers))
		{
			continue; // Sessão de outro jogo/versão ou sem o setting: descarta.
		}
		if (AdvertisedPlayers != SearchPlayerCount)
		{
			continue;
		}

		FDSSessionInfo Info;
		Info.ResultIndex = Index;
		Info.SessionId = Result.GetSessionIdStr();
		Info.HostName = Result.Session.OwningUserName;
		Info.PartySize = DSSessionPrivate::ToPartySize(AdvertisedPlayers);
		Info.MaxPlayers = Result.Session.SessionSettings.NumPublicConnections;
		Info.CurrentPlayers = FMath::Clamp(Info.MaxPlayers - Result.Session.NumOpenPublicConnections, 0, Info.MaxPlayers);
		Info.PingMs = Result.PingInMs;
		Info.bIsLAN = Result.Session.SessionSettings.bIsLANMatch;
		if (Info.GetOpenSlots() > 0)
		{
			Infos.Add(Info);
		}
	}
	return Infos;
}

// --- Join ---------------------------------------------------------------

void UDSSessionSubsystem::JoinSession(int32 ResultIndex)
{
	const IOnlineSessionPtr Sessions = GetSessionInterface();
	const bool bValidIndex = SessionSearch.IsValid() && SessionSearch->SearchResults.IsValidIndex(ResultIndex)
		&& SessionSearch->SearchResults[ResultIndex].IsValid();
	if (IsBusy() || !Sessions.IsValid() || !bValidIndex)
	{
		UE_LOG(LogDungeonStance, Warning, TEXT("[Session] JoinSession(%d) inválido (ocupado, sem busca ou índice fora do intervalo)."), ResultIndex);
		OnSessionJoined.Broadcast(false);
		return;
	}

	if (Sessions->GetNamedSession(NAME_GameSession))
	{
		FAfterDestroy Next;
		Next.Op = EPendingOp::Joining;
		Next.ResultIndex = ResultIndex;
		DestroyThen(Next);
		return;
	}

	JoinCompleteHandle = Sessions->AddOnJoinSessionCompleteDelegate_Handle(
		FOnJoinSessionCompleteDelegate::CreateUObject(this, &ThisClass::HandleJoinSessionComplete));
	PendingOp = EPendingOp::Joining;

	if (!Sessions->JoinSession(DSSessionPrivate::LocalUserNum, NAME_GameSession, SessionSearch->SearchResults[ResultIndex])
		&& PendingOp == EPendingOp::Joining)
	{
		HandleJoinSessionComplete(NAME_GameSession, EOnJoinSessionCompleteResult::UnknownError);
	}
}

void UDSSessionSubsystem::HandleJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result)
{
	const IOnlineSessionPtr Sessions = GetSessionInterface();
	if (Sessions.IsValid())
	{
		Sessions->ClearOnJoinSessionCompleteDelegate_Handle(JoinCompleteHandle);
	}
	PendingOp = EPendingOp::None;

	bool bTraveled = false;
	FString ConnectString;
	if (Result == EOnJoinSessionCompleteResult::Success && Sessions.IsValid() && Sessions->GetResolvedConnectString(SessionName, ConnectString))
	{
		const UGameInstance* GameInstance = GetGameInstance();
		APlayerController* PlayerController = GameInstance ? GameInstance->GetFirstLocalPlayerController() : nullptr;
		if (PlayerController)
		{
			PlayerController->ClientTravel(ConnectString, TRAVEL_Absolute);
			bTraveled = true;
		}
		else
		{
			UE_LOG(LogDungeonStance, Error, TEXT("[Session] Join ok mas sem PlayerController local para ClientTravel."));
		}
	}

	UE_LOG(LogDungeonStance, Log, TEXT("[Session] JoinSession concluído: resultado=%d, viajando=%d."), static_cast<int32>(Result), bTraveled ? 1 : 0);
	OnSessionJoined.Broadcast(bTraveled);
}

// --- Destroy ------------------------------------------------------------

void UDSSessionSubsystem::DestroySession()
{
	if (PendingOp == EPendingOp::Destroying)
	{
		return;
	}
	if (IsBusy())
	{
		UE_LOG(LogDungeonStance, Warning, TEXT("[Session] DestroySession com operação pendente; a operação anterior será descartada."));
		ClearSessionDelegates();
		PendingOp = EPendingOp::None;
	}
	if (!HasActiveSession())
	{
		OnSessionDestroyed.Broadcast(true);
		return;
	}
	DestroyThen(FAfterDestroy());
}

void UDSSessionSubsystem::DestroyThen(const FAfterDestroy& Next)
{
	const IOnlineSessionPtr Sessions = GetSessionInterface();
	if (!Sessions.IsValid())
	{
		RunAfterDestroy(Next, false);
		return;
	}

	AfterDestroy = Next;
	DestroyCompleteHandle = Sessions->AddOnDestroySessionCompleteDelegate_Handle(
		FOnDestroySessionCompleteDelegate::CreateUObject(this, &ThisClass::HandleDestroySessionComplete));
	PendingOp = EPendingOp::Destroying;

	if (!Sessions->DestroySession(NAME_GameSession) && PendingOp == EPendingOp::Destroying)
	{
		HandleDestroySessionComplete(NAME_GameSession, false);
	}
}

void UDSSessionSubsystem::HandleDestroySessionComplete(FName SessionName, bool bWasSuccessful)
{
	if (const IOnlineSessionPtr Sessions = GetSessionInterface())
	{
		Sessions->ClearOnDestroySessionCompleteDelegate_Handle(DestroyCompleteHandle);
	}
	PendingOp = EPendingOp::None;

	UE_LOG(LogDungeonStance, Log, TEXT("[Session] DestroySession(%s): %s."), *SessionName.ToString(), bWasSuccessful ? TEXT("ok") : TEXT("FALHOU"));
	OnSessionDestroyed.Broadcast(bWasSuccessful);

	const FAfterDestroy Next = AfterDestroy;
	AfterDestroy = FAfterDestroy();
	RunAfterDestroy(Next, bWasSuccessful);
}

void UDSSessionSubsystem::RunAfterDestroy(const FAfterDestroy& Next, bool bDestroyed)
{
	// Sem destruir a sessão antiga, repetir Host/Join entraria em loop: falha explícita.
	if (!bDestroyed && Next.Op != EPendingOp::None)
	{
		UE_LOG(LogDungeonStance, Error, TEXT("[Session] Não foi possível destruir a sessão anterior; operação seguinte cancelada."));
		if (Next.Op == EPendingOp::Creating)
		{
			OnSessionCreated.Broadcast(false);
		}
		else if (Next.Op == EPendingOp::Joining)
		{
			OnSessionJoined.Broadcast(false);
		}
		return;
	}

	if (Next.bStartSolo)
	{
		OpenSoloLevel();
	}
	else if (Next.Op == EPendingOp::Creating)
	{
		HostSession(Next.PartySize, Next.bIsLAN);
	}
	else if (Next.Op == EPendingOp::Joining)
	{
		JoinSession(Next.ResultIndex);
	}
}

// --- Solo / viagem ------------------------------------------------------

void UDSSessionSubsystem::StartSoloRun()
{
	if (IsBusy())
	{
		UE_LOG(LogDungeonStance, Warning, TEXT("[Session] StartSoloRun ignorado: outra operação em andamento."));
		return;
	}
	if (HasActiveSession())
	{
		FAfterDestroy Next;
		Next.bStartSolo = true;
		DestroyThen(Next);
		return;
	}
	OpenSoloLevel();
}

void UDSSessionSubsystem::OpenSoloLevel()
{
	const UDSNetworkSettings* NetSettings = UDSNetworkSettings::Get();
	const FString MapName = NetSettings->GetDungeonMapName();
	if (MapName.IsEmpty())
	{
		UE_LOG(LogDungeonStance, Error, TEXT("[Session] DungeonMap não configurado em Project Settings > Dungeon Stance Network."));
		return;
	}

	const FString Options = DSSessionPrivate::BuildTravelOptions(DSConstants::MinPartySize, NetSettings->bSoloUsesListenServer);
	UE_LOG(LogDungeonStance, Log, TEXT("[Session] Iniciando run solo em %s?%s."), *MapName, *Options);
	UGameplayStatics::OpenLevel(GetGameWorld(), FName(*MapName), true, Options);
}

bool UDSSessionSubsystem::TravelToDungeon()
{
	UWorld* World = GetGameWorld();
	if (!World || World->GetNetMode() == NM_Client)
	{
		UE_LOG(LogDungeonStance, Warning, TEXT("[Session] TravelToDungeon só pode ser chamado no servidor."));
		return false;
	}

	const FString MapName = UDSNetworkSettings::Get()->GetDungeonMapName();
	if (MapName.IsEmpty())
	{
		UE_LOG(LogDungeonStance, Error, TEXT("[Session] DungeonMap não configurado."));
		return false;
	}

	int32 NumPlayers = HostedPlayerCount > 0 ? HostedPlayerCount : DSConstants::MinPartySize;
	const IOnlineSessionPtr Sessions = GetSessionInterface();
	if (const FNamedOnlineSession* Session = Sessions.IsValid() ? Sessions->GetNamedSession(NAME_GameSession) : nullptr)
	{
		NumPlayers = Session->SessionSettings.NumPublicConnections;
		Sessions->StartSession(NAME_GameSession);
	}

	const bool bListen = World->GetNetMode() == NM_ListenServer;
	const FString Url = MapName + TEXT("?") + DSSessionPrivate::BuildTravelOptions(NumPlayers, bListen);
	UE_LOG(LogDungeonStance, Log, TEXT("[Session] ServerTravel para %s."), *Url);
	return World->ServerTravel(Url);
}

void UDSSessionSubsystem::HandleNetworkFailure(UWorld* World, UNetDriver* NetDriver, ENetworkFailure::Type FailureType, const FString& ErrorString)
{
	UE_LOG(LogDungeonStance, Warning, TEXT("[Session] Falha de rede (%s): %s"), ENetworkFailure::ToString(FailureType), *ErrorString);

	// Cliente que perdeu o servidor não deve ficar preso a uma sessão morta.
	if (World && World->GetNetMode() == NM_Client)
	{
		DestroySession();
	}
}
