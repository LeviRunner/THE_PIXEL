#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Engine/EngineBaseTypes.h"
#include "Engine/TimerHandle.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "OnlineSessionSettings.h"
#include "Network/DSSessionTypes.h"
#include "DSSessionSubsystem.generated.h"

class UNetDriver;

/**
 * Sessões via IOnlineSubsystem/IOnlineSession: funciona com OnlineSubsystemNull (LAN/PIE)
 * e, sem mudar a API, com Steam/EOS quando o DefaultPlatformService mudar.
 *
 * Só uma operação assíncrona por vez (IsBusy). Se já existir uma sessão, Host/Join a
 * destroem antes e então prosseguem.
 */
UCLASS()
class DUNGEONSTANCE_API UDSSessionSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	/** Cria a sessão (MaxPlayers = tamanho da party) e abre o lobby com ?listen. */
	UFUNCTION(BlueprintCallable, Category = "DS|Session")
	void HostSession(EDSPartySize PartySize, bool bIsLAN);

	/** Busca sessões com o mesmo tamanho de party e vaga livre. Resultado em OnSessionsFound. */
	UFUNCTION(BlueprintCallable, Category = "DS|Session")
	void FindSessions(EDSPartySize PartySize);

	/** Entra na sessão de índice FDSSessionInfo::ResultIndex da última busca e faz ClientTravel. */
	UFUNCTION(BlueprintCallable, Category = "DS|Session")
	void JoinSession(int32 ResultIndex);

	UFUNCTION(BlueprintCallable, Category = "DS|Session")
	void DestroySession();

	/** Abre a dungeon localmente (sem matchmaking). ?listen se bSoloUsesListenServer. */
	UFUNCTION(BlueprintCallable, Category = "DS|Session")
	void StartSoloRun();

	/** Só servidor: ServerTravel do lobby para a dungeon e marca a sessão como iniciada. */
	UFUNCTION(BlueprintCallable, Category = "DS|Session")
	bool TravelToDungeon();

	UFUNCTION(BlueprintPure, Category = "DS|Session")
	bool HasActiveSession() const;

	UFUNCTION(BlueprintPure, Category = "DS|Session")
	bool IsBusy() const { return PendingOp != EPendingOp::None; }

	/** LAN quando o subsistema ativo é o Null (e bForceLANWithNullSubsystem). */
	UFUNCTION(BlueprintPure, Category = "DS|Session")
	bool ShouldUseLAN() const;

	UFUNCTION(BlueprintPure, Category = "DS|Session")
	const TArray<FDSSessionInfo>& GetLastSearchResults() const { return LastResults; }

	UPROPERTY(BlueprintAssignable, Category = "DS|Session")
	FDSOnSessionCreated OnSessionCreated;

	UPROPERTY(BlueprintAssignable, Category = "DS|Session")
	FDSOnSessionsFound OnSessionsFound;

	UPROPERTY(BlueprintAssignable, Category = "DS|Session")
	FDSOnSessionJoined OnSessionJoined;

	UPROPERTY(BlueprintAssignable, Category = "DS|Session")
	FDSOnSessionDestroyed OnSessionDestroyed;

private:
	enum class EPendingOp : uint8
	{
		None,
		Creating,
		Finding,
		Joining,
		Destroying
	};

	/** O que fazer quando o DestroySession terminar. */
	struct FAfterDestroy
	{
		EPendingOp Op = EPendingOp::None;
		EDSPartySize PartySize = EDSPartySize::Solo;
		bool bIsLAN = false;
		int32 ResultIndex = INDEX_NONE;
		bool bStartSolo = false;
	};

	UWorld* GetGameWorld() const;
	IOnlineSessionPtr GetSessionInterface() const;
	FOnlineSessionSettings BuildSessionSettings(EDSPartySize PartySize, bool bIsLAN) const;
	TArray<FDSSessionInfo> BuildSessionInfos() const;

	void DestroyThen(const FAfterDestroy& Next);
	void RunAfterDestroy(const FAfterDestroy& Next, bool bDestroyed);
	void OpenSoloLevel();
	void TravelToLobbyAsHost(int32 NumPlayers);
	void FinishFind(bool bWasSuccessful);
	void HandleFindTimeout();
	void ClearSessionDelegates();

	void HandleCreateSessionComplete(FName SessionName, bool bWasSuccessful);
	void HandleFindSessionsComplete(bool bWasSuccessful);
	void HandleJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result);
	void HandleDestroySessionComplete(FName SessionName, bool bWasSuccessful);
	void HandleNetworkFailure(UWorld* World, UNetDriver* NetDriver, ENetworkFailure::Type FailureType, const FString& ErrorString);

	EPendingOp PendingOp = EPendingOp::None;
	FAfterDestroy AfterDestroy;

	TSharedPtr<FOnlineSessionSearch> SessionSearch;
	TArray<FDSSessionInfo> LastResults;
	int32 SearchPlayerCount = 0;
	int32 HostedPlayerCount = 0;

	FDelegateHandle CreateCompleteHandle;
	FDelegateHandle FindCompleteHandle;
	FDelegateHandle JoinCompleteHandle;
	FDelegateHandle DestroyCompleteHandle;
	FDelegateHandle NetworkFailureHandle;
	FTimerHandle FindTimeoutHandle;
};
