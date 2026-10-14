#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Network/DSSessionTypes.h"
#include "Network/Matchmaking/DSMatchmakerTicketService.h"
#include "DSMatchmakingSubsystem.generated.h"

class UDSSessionSubsystem;

UENUM(BlueprintType)
enum class EDSMatchmakingState : uint8
{
	Idle,
	Searching,
	Joining,
	Hosting,
	/** Aguardando o serviço de tickets (Nakama/Steam/EOS). */
	WaitingForTicket,
	Matched,
	Failed
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FDSOnMatchmakingStateChanged, EDSMatchmakingState, NewState);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FDSOnMatchmakingFinished, bool, bWasSuccessful, bool, bIsHost);

/**
 * Fila por tamanho de party (1..5) com suporte a grupo pré-formado.
 * Implementação inicial (sem serviço de ticket): procura sessão compatível com vagas
 * suficientes para o grupo → entra; se não houver (ou o join falhar) → hospeda.
 * Solo não passa por matchmaking (StartSoloRun).
 */
UCLASS()
class DUNGEONSTANCE_API UDSMatchmakingSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	/** GroupSize = jogadores do grupo pré-formado (o líder chama; os demais seguem — ver README). */
	UFUNCTION(BlueprintCallable, Category = "DS|Matchmaking")
	void StartMatchmaking(EDSPartySize PartySize, int32 GroupSize = 1);

	/** Variante com dados completos do ticket (usada quando há serviço de ticket). */
	UFUNCTION(BlueprintCallable, Category = "DS|Matchmaking")
	void StartMatchmakingWithTicket(const FDSMatchmakingTicket& Ticket);

	UFUNCTION(BlueprintCallable, Category = "DS|Matchmaking")
	void CancelMatchmaking();

	UFUNCTION(BlueprintPure, Category = "DS|Matchmaking")
	EDSMatchmakingState GetState() const { return State; }

	UFUNCTION(BlueprintPure, Category = "DS|Matchmaking")
	bool IsMatchmaking() const;

	/** Registra o serviço de ticket (ex.: futuro UDSNakamaMatchmaker). nullptr volta ao fluxo de sessões. */
	void SetTicketService(UObject* ServiceObject);

	UPROPERTY(BlueprintAssignable, Category = "DS|Matchmaking")
	FDSOnMatchmakingStateChanged OnMatchmakingStateChanged;

	UPROPERTY(BlueprintAssignable, Category = "DS|Matchmaking")
	FDSOnMatchmakingFinished OnMatchmakingFinished;

private:
	UFUNCTION()
	void HandleSessionsFound(const TArray<FDSSessionInfo>& Sessions);

	UFUNCTION()
	void HandleSessionJoined(bool bWasSuccessful);

	UFUNCTION()
	void HandleSessionCreated(bool bWasSuccessful);

	void HandleTicketMatched(bool bSuccess, const FString& ConnectStringOrError);

	UDSSessionSubsystem* GetSessionSubsystem() const;
	int32 PickBestSession(const TArray<FDSSessionInfo>& Sessions) const;
	void HostFallback();
	void SetState(EDSMatchmakingState NewState);
	void Finish(bool bSuccess, bool bIsHost);

	UPROPERTY()
	TScriptInterface<IDSMatchmakerTicketService> TicketService;

	EDSMatchmakingState State = EDSMatchmakingState::Idle;
	FDSMatchmakingTicket ActiveTicket;
};
