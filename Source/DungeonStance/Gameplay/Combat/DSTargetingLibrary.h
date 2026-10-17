#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "DSTargetingLibrary.generated.h"

class ADSCharacterBase;

/** Que lado buscar a partir de quem pergunta. */
UENUM(BlueprintType)
enum class EDSTargetFilter : uint8
{
	Hostile,
	Ally,
	Any
};

/**
 * Consultas de área cientes do grupo (1 a 5 heróis): arco/raio/linha, vivos, por time.
 * Distâncias contam a cápsula do alvo (o protótipo somava o raio do inimigo).
 */
UCLASS()
class DUNGEONSTANCE_API UDSTargetingLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	/** Personagens vivos num raio (centro no plano XY, altura tolerante). */
	static void FindInRadius(const ADSCharacterBase* Querier, const FVector& Center, float Radius, EDSTargetFilter Filter,
		TArray<ADSCharacterBase*>& OutTargets, bool bIncludeDowned = false);

	/** Personagens vivos dentro de um arco à frente (ArcDegrees total; 360 = nova). Ordenados por distância. */
	static void FindInArc(const ADSCharacterBase* Querier, const FVector& Origin, const FVector& Forward, float Radius, float ArcDegrees,
		EDSTargetFilter Filter, TArray<ADSCharacterBase*>& OutTargets);

	/** Personagens vivos num retângulo à frente (tiros perfurantes). */
	static void FindInLine(const ADSCharacterBase* Querier, const FVector& Origin, const FVector& Forward, float Length, float HalfWidth,
		EDSTargetFilter Filter, TArray<ADSCharacterBase*>& OutTargets);

	/** Mais próximo do filtro dentro do raio (ou nulo). */
	static ADSCharacterBase* FindNearest(const ADSCharacterBase* Querier, const FVector& Center, float Radius, EDSTargetFilter Filter);

	static bool PassesFilter(const ADSCharacterBase* Querier, const ADSCharacterBase* Candidate, EDSTargetFilter Filter);
	static float GetCollisionRadius(const ADSCharacterBase* Character);
};
