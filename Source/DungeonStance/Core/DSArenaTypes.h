#pragma once

#include "CoreMinimal.h"
#include "Core/DSRunRules.h"
#include "DSArenaTypes.generated.h"

class UDSEnemyData;

/** Inimigo regular que pode aparecer nas ondas de uma arena (configurado no BP do GameMode). */
USTRUCT(BlueprintType)
struct FDSEnemySpawnEntry
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Arena")
	TObjectPtr<UDSEnemyData> EnemyData = nullptr;

	/** Peso relativo no sorteio entre as entradas liberadas na onda. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Arena", meta = (ClampMin = "0.0"))
	float Weight = 1.f;

	/** Primeira onda em que a entrada pode aparecer (ex.: atiradores só a partir da 2). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Arena", meta = (ClampMin = "1"))
	int32 MinWave = 1;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Arena")
	bool bElite = false;
};

/** Chefe de uma onda de chefe (ex.: onda 5 = elite, onda 10 = chefe final). */
USTRUCT(BlueprintType)
struct FDSBossSpawnEntry
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Arena", meta = (ClampMin = "1"))
	int32 Wave = 5;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Arena")
	TObjectPtr<UDSEnemyData> EnemyData = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Arena")
	bool bElite = false;
};

/** Anel em volta da party onde os inimigos nascem. */
USTRUCT(BlueprintType)
struct FDSEnemySpawnRing
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Arena", meta = (ClampMin = "0.0", Units = "cm"))
	float MinRadiusCm = DSRunRules::EnemySpawnRingMinCm;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Arena", meta = (ClampMin = "0.0", Units = "cm"))
	float MaxRadiusCm = DSRunRules::EnemySpawnRingMaxCm;

	/** Distância mínima até qualquer herói. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Arena", meta = (ClampMin = "0.0", Units = "cm"))
	float PlayerClearanceCm = DSRunRules::EnemySpawnPlayerClearanceCm;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Arena", meta = (ClampMin = "1"))
	int32 MaxAttempts = DSRunRules::EnemySpawnAttempts;

	/** Meia-extensão da busca no navmesh ao projetar o ponto sorteado. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Arena")
	FVector NavProjectionExtent = FVector(200.f, 200.f, 1000.f);
};
