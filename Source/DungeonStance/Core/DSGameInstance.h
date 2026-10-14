#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "GameFramework/SaveGame.h"
#include "Core/DSTypes.h"
#include "DSGameInstance.generated.h"

/** Perfil local salvo em disco (preferências; nada de progresso — progresso é do backend). */
UCLASS()
class DUNGEONSTANCE_API UDSLocalProfileSave : public USaveGame
{
	GENERATED_BODY()

public:
	UPROPERTY()
	FString LocalPlayerId;

	UPROPERTY()
	EDSHeroClass LastClass = EDSHeroClass::None;

	UPROPERTY()
	EDSStance LastStance = EDSStance::None;

	UPROPERTY()
	EDSPartySize DesiredPartySize = EDSPartySize::Solo;
};

/**
 * Game instance: perfil local do jogador e preferências de lobby.
 * Acesso ao backend (login, progresso, envio do resultado) fica no
 * UDSBackendSubsystem (Network), que é um subsistema desta instância.
 */
UCLASS()
class DUNGEONSTANCE_API UDSGameInstance : public UGameInstance
{
	GENERATED_BODY()

public:
	virtual void Init() override;

	/** Grava LocalPlayerId/LastClass/LastStance/DesiredPartySize no slot local. */
	UFUNCTION(BlueprintCallable, Category = "DungeonStance|Profile")
	bool SaveLocalProfile();

	/** Lê o slot local; se não existir, mantém os valores atuais. */
	UFUNCTION(BlueprintCallable, Category = "DungeonStance|Profile")
	bool LoadLocalProfile();

	UFUNCTION(BlueprintPure, Category = "DungeonStance|Profile")
	int32 GetDesiredPartySizeAsInt() const { return static_cast<int32>(DesiredPartySize); }

	/** Id do jogador no backend (preenchido após o login pela área Network). */
	UPROPERTY(BlueprintReadWrite, Category = "DungeonStance|Profile")
	FString LocalPlayerId;

	UPROPERTY(BlueprintReadWrite, Category = "DungeonStance|Profile")
	EDSHeroClass LastClass = EDSHeroClass::None;

	UPROPERTY(BlueprintReadWrite, Category = "DungeonStance|Profile")
	EDSStance LastStance = EDSStance::None;

	UPROPERTY(BlueprintReadWrite, Category = "DungeonStance|Profile")
	EDSPartySize DesiredPartySize = EDSPartySize::Solo;

protected:
	UPROPERTY(EditDefaultsOnly, Category = "DungeonStance|Profile")
	FString ProfileSlotName = TEXT("DSLocalProfile");

	UPROPERTY(EditDefaultsOnly, Category = "DungeonStance|Profile")
	int32 ProfileUserIndex = 0;
};
