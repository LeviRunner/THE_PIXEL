#include "UI/HUD/DSPartyFrameWidget.h"

#include "Components/PanelWidget.h"
#include "Core/DSGameState.h"
#include "Core/DSPlayerState.h"
#include "Core/DSTypes.h"
#include "DungeonStance.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "UI/HUD/DSPartyMemberWidget.h"

void UDSPartyFrameWidget::InitParty(ADSGameState* InGameState, ADSPlayerState* InLocalPlayerState)
{
	GameState = InGameState;
	LocalPlayerState = InLocalPlayerState;

	if (!MemberWidgetClass)
	{
		UE_LOG(LogDungeonStance, Warning, TEXT("UDSPartyFrameWidget '%s' sem MemberWidgetClass."), *GetName());
		return;
	}

	RefreshRoster();
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimer(RosterTimer, this, &ThisClass::RefreshRoster, RosterPollInterval, true);
	}
}

void UDSPartyFrameWidget::NativeDestruct()
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(RosterTimer);
	}
	Super::NativeDestruct();
}

TArray<ADSPlayerState*> UDSPartyFrameWidget::CollectPartyPlayers() const
{
	TArray<ADSPlayerState*> Players;
	const ADSGameState* GS = GameState.Get();
	if (!GS)
	{
		return Players;
	}

	ADSPlayerState* LocalPS = LocalPlayerState.Get();
	if (bShowLocalPlayer && LocalPS)
	{
		Players.Add(LocalPS);
	}
	for (APlayerState* Entry : GS->PlayerArray)
	{
		ADSPlayerState* PS = Cast<ADSPlayerState>(Entry);
		if (!PS || PS == LocalPS || PS->IsOnlyASpectator())
		{
			continue;
		}
		Players.Add(PS);
	}
	if (Players.Num() > DSConstants::MaxPartySize)
	{
		Players.SetNum(DSConstants::MaxPartySize);
	}
	return Players;
}

bool UDSPartyFrameWidget::IsSameRoster(const TArray<ADSPlayerState*>& Players) const
{
	if (Players.Num() != ShownPlayers.Num())
	{
		return false;
	}
	for (int32 Index = 0; Index < Players.Num(); ++Index)
	{
		if (ShownPlayers[Index].Get() != Players[Index])
		{
			return false;
		}
	}
	return true;
}

UDSPartyMemberWidget* UDSPartyFrameWidget::GetOrCreateMemberWidget(int32 Index)
{
	if (MemberWidgets.IsValidIndex(Index))
	{
		return MemberWidgets[Index];
	}
	UDSPartyMemberWidget* Member = CreateWidget<UDSPartyMemberWidget>(this, MemberWidgetClass);
	if (Member)
	{
		MemberContainer->AddChild(Member);
		MemberWidgets.Add(Member);
	}
	return Member;
}

void UDSPartyFrameWidget::RefreshRoster()
{
	const TArray<ADSPlayerState*> Players = CollectPartyPlayers();
	if (IsSameRoster(Players))
	{
		return;
	}

	// Reaproveita os quadros por índice; só cria/remove a diferença.
	for (int32 Index = 0; Index < Players.Num(); ++Index)
	{
		if (UDSPartyMemberWidget* Member = GetOrCreateMemberWidget(Index))
		{
			Member->SetPlayerState(Players[Index]);
		}
	}
	while (MemberWidgets.Num() > Players.Num())
	{
		UDSPartyMemberWidget* Extra = MemberWidgets.Pop();
		if (Extra)
		{
			Extra->SetPlayerState(nullptr);
			Extra->RemoveFromParent();
		}
	}

	ShownPlayers.Reset(Players.Num());
	for (ADSPlayerState* PS : Players)
	{
		ShownPlayers.Add(PS);
	}
}
