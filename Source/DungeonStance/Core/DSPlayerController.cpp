#include "Core/DSPlayerController.h"

#include "Core/DSCardBridge.h"
#include "Core/DSGameInstance.h"
#include "Core/DSGameMode.h"
#include "Core/DSGameState.h"
#include "Core/DSPlayerState.h"
#include "DungeonStance.h"

ADSPlayerController::ADSPlayerController(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void ADSPlayerController::BeginPlay()
{
	Super::BeginPlay();

	if (!bAutoSubmitSavedHero || !IsLocalController())
	{
		return;
	}
	const UDSGameInstance* GameInstance = GetGameInstance<UDSGameInstance>();
	if (GameInstance && GameInstance->LastClass != EDSHeroClass::None && GameInstance->LastStance != EDSStance::None)
	{
		Server_SelectHero(GameInstance->LastClass, GameInstance->LastStance);
		Server_SetReady(true);
	}
}

ADSPlayerState* ADSPlayerController::GetDSPlayerState() const
{
	return GetPlayerState<ADSPlayerState>();
}

bool ADSPlayerController::IsInLobbyPhase() const
{
	const ADSGameState* GameState = GetWorld() ? GetWorld()->GetGameState<ADSGameState>() : nullptr;
	return GameState && GameState->GetMatchPhase() == EDSMatchPhase::WaitingForPlayers;
}

void ADSPlayerController::Server_SelectHero_Implementation(EDSHeroClass InHeroClass, EDSStance InStance)
{
	ADSPlayerState* DSPlayerState = GetDSPlayerState();
	if (!DSPlayerState || !IsInLobbyPhase())
	{
		UE_LOG(LogDungeonStance, Warning, TEXT("Server_SelectHero recusado: fora do lobby (%s)."), *GetNameSafe(this));
		return;
	}
	// Enum vindo da rede pode ter qualquer byte: valida o intervalo.
	const bool bValidClass = InHeroClass > EDSHeroClass::None && InHeroClass <= EDSHeroClass::Archer;
	const bool bValidStance = InStance > EDSStance::None && InStance <= EDSStance::Support;
	if (!bValidClass || !bValidStance)
	{
		UE_LOG(LogDungeonStance, Warning, TEXT("Server_SelectHero recusado: classe/stance inválida (%d/%d)."),
			static_cast<int32>(InHeroClass), static_cast<int32>(InStance));
		return;
	}
	DSPlayerState->SetHeroSelection(InHeroClass, InStance);
}

void ADSPlayerController::Server_SetReady_Implementation(bool bInReady)
{
	ADSPlayerState* DSPlayerState = GetDSPlayerState();
	if (!DSPlayerState || !IsInLobbyPhase())
	{
		return;
	}
	DSPlayerState->SetReady(bInReady);
	if (ADSGameMode* GameMode = GetWorld()->GetAuthGameMode<ADSGameMode>())
	{
		GameMode->NotifyPlayerReadyChanged(this);
	}
}

void ADSPlayerController::Server_PickCard_Implementation(FName CardId)
{
	ADSPlayerState* DSPlayerState = GetDSPlayerState();
	ADSGameMode* GameMode = GetWorld()->GetAuthGameMode<ADSGameMode>();
	if (!DSPlayerState || !GameMode)
	{
		return;
	}
	if (DSPlayerState->GetPendingCardPicks() <= 0)
	{
		RejectCardPick(CardId, TEXT("no_pending_picks"));
		return;
	}
	// Só aceita cartas que o servidor sorteou para este jogador.
	if (!DSPlayerState->GetCurrentCardChoices().Contains(CardId))
	{
		RejectCardPick(CardId, TEXT("not_offered"));
		return;
	}
	FString Reason;
	if (!DSCardBridge::ValidatePick(*DSPlayerState, CardId, Reason))
	{
		RejectCardPick(CardId, Reason);
		return;
	}
	if (!DSCardBridge::ApplyToPlayer(*DSPlayerState, CardId))
	{
		RejectCardPick(CardId, TEXT("apply_failed"));
		return;
	}
	DSPlayerState->AddCardStack(CardId);
	DSPlayerState->ConsumePendingCardPick();
	GameMode->NotifyCardPicked(this);
}

void ADSPlayerController::RejectCardPick(FName CardId, const FString& Reason)
{
	UE_LOG(LogDungeonStance, Log, TEXT("Carta %s recusada para %s: %s"), *CardId.ToString(), *GetNameSafe(this), *Reason);
	Client_CardPickRejected(CardId, Reason);
}

void ADSPlayerController::Client_CardPickRejected_Implementation(FName CardId, const FString& Reason)
{
	OnCardPickRejected.Broadcast(CardId, Reason);
}
