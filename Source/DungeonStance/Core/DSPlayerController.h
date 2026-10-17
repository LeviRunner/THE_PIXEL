#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "Core/DSTypes.h"
#include "DSPlayerController.generated.h"

class ADSPlayerState;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FDSOnCardPickRejected, FName, CardId, const FString&, Reason);

/**
 * Controller do jogador. Pontos de entrada do cliente no servidor:
 * escolha de herói, "pronto" e escolha de carta. Toda RPC é validada no
 * servidor; pedido inválido é ignorado (com log) em vez de derrubar a conexão.
 *
 * O HUD (ADSHUD) é criado pelo fluxo padrão da engine: o GameMode define
 * HUDClass e o servidor chama ClientSetHUD para o controller local.
 */
UCLASS()
class DUNGEONSTANCE_API ADSPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	ADSPlayerController(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	UFUNCTION(BlueprintPure, Category = "DungeonStance|Player")
	ADSPlayerState* GetDSPlayerState() const;

	/** Escolhe classe e stance (só antes da run começar). */
	UFUNCTION(Server, Reliable, BlueprintCallable, Category = "DungeonStance|Player")
	void Server_SelectHero(EDSHeroClass InHeroClass, EDSStance InStance);

	/** Marca o jogador como pronto no lobby da partida. */
	UFUNCTION(Server, Reliable, BlueprintCallable, Category = "DungeonStance|Player")
	void Server_SetReady(bool bInReady);

	/** Escolhe uma das CurrentCardChoices. */
	UFUNCTION(Server, Reliable, BlueprintCallable, Category = "DungeonStance|Cards")
	void Server_PickCard(FName CardId);

	/** Aviso ao dono de que a carta foi recusada (para a UI destravar). */
	UFUNCTION(Client, Reliable)
	void Client_CardPickRejected(FName CardId, const FString& Reason);

	UPROPERTY(BlueprintAssignable, Category = "DungeonStance|Cards")
	FDSOnCardPickRejected OnCardPickRejected;

protected:
	virtual void BeginPlay() override;

	/**
	 * Se verdadeiro, o cliente local envia a classe/stance salvas no UDSGameInstance
	 * e marca "pronto" ao entrar no mapa (herói já escolhido no lobby/menu).
	 */
	UPROPERTY(EditDefaultsOnly, Category = "DungeonStance|Player")
	bool bAutoSubmitSavedHero = true;

	/** Só aceita mudanças de lobby enquanto a partida espera jogadores. */
	bool IsInLobbyPhase() const;

	void RejectCardPick(FName CardId, const FString& Reason);
};
