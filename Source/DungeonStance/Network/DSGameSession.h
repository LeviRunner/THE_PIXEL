#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameSession.h"
#include "DSGameSession.generated.h"

/**
 * GameSession do Dungeon Stance: fixa MaxPlayers no tamanho da party (1..5) para que
 * AGameSession::ApproveLogin recuse quem passar do limite.
 *
 * Ordem de prioridade: opção de URL "?PartySize=N" (posta pelo UDSSessionSubsystem) →
 * NumPublicConnections da sessão nomeada → UDSNetworkSettings::MaxPartySize.
 *
 * Uso: ADSGameMode deve definir GameSessionClass = ADSGameSession::StaticClass() (área Core).
 */
UCLASS()
class DUNGEONSTANCE_API ADSGameSession : public AGameSession
{
	GENERATED_BODY()

public:
	virtual void InitOptions(const FString& Options) override;

private:
	int32 ReadPartySizeFromNamedSession() const;
};
