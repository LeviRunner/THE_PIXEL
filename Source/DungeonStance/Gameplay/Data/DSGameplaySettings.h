#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "UObject/SoftObjectPtr.h"
#include "DSGameplaySettings.generated.h"

class UDSCardRegistry;

/**
 * Configuração global da área Gameplay (DefaultGame.ini, seção [/Script/DungeonStance.DSGameplaySettings]).
 * Usado quando a API do Core não passa o registro de cartas explicitamente (DSCardBridge).
 */
UCLASS(Config = Game, DefaultConfig)
class DUNGEONSTANCE_API UDSGameplaySettings : public UObject
{
	GENERATED_BODY()

public:
	/** Catálogo de cartas e habilidades de apoio da run. */
	UPROPERTY(Config, EditAnywhere, Category = "Cards")
	TSoftObjectPtr<UDSCardRegistry> CardRegistry = TSoftObjectPtr<UDSCardRegistry>(FSoftObjectPath(TEXT("/Game/DungeonStance/Data/Cards/DA_CardRegistry.DA_CardRegistry")));

	/** Nº de cartas oferecidas por escolha. */
	UPROPERTY(Config, EditAnywhere, Category = "Cards", meta = (ClampMin = 1))
	int32 CardChoicesPerPick = 3;
};
