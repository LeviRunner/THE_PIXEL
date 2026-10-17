#pragma once

#include "CoreMinimal.h"
#include "Core/DSArenaTypes.h"

class UWorld;
struct FRandomStream;

/** Funções auxiliares (sem estado) do spawn de ondas usadas pelo ADSGameMode. */
namespace DSWaveSpawnHelpers
{
	/** Sorteio ponderado entre as entradas liberadas na onda (MinWave ≤ onda, peso > 0, dado válido). */
	const FDSEnemySpawnEntry* PickRegularEntry(const TArray<FDSEnemySpawnEntry>& Entries, int32 Wave, FRandomStream& Random);

	/** Chefe configurado para a onda; sem entrada exata, usa o da maior onda ≤ Wave (ou o primeiro). */
	const FDSBossSpawnEntry* FindBossEntry(const TArray<FDSBossSpawnEntry>& Bosses, int32 Wave);

	/** Média das posições (Fallback se a lista estiver vazia). */
	FVector Centroid(const TArray<FVector>& Points, const FVector& Fallback);

	/**
	 * Ponto no anel [Min, Max] em volta de Center, projetado no navmesh e longe dos heróis.
	 * Sem navmesh/sem ponto válido, devolve o último candidato sorteado.
	 */
	FVector FindEnemySpawnLocation(UWorld* World, const FVector& Center, const TArray<FVector>& HeroLocations,
		const FDSEnemySpawnRing& Ring, FRandomStream& Random);
}
