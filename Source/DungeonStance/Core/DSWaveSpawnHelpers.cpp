#include "Core/DSWaveSpawnHelpers.h"

#include "Engine/World.h"
#include "Math/RandomStream.h"
#include "NavigationSystem.h"

namespace DSWaveSpawnHelpers
{
	namespace
	{
		bool IsEntryAvailable(const FDSEnemySpawnEntry& Entry, int32 Wave)
		{
			return Entry.EnemyData != nullptr && Entry.Weight > 0.f && Entry.MinWave <= Wave;
		}

		bool IsClearOfHeroes(const FVector& Point, const TArray<FVector>& HeroLocations, float ClearanceCm)
		{
			const float ClearanceSq = FMath::Square(ClearanceCm);
			for (const FVector& Hero : HeroLocations)
			{
				if (FVector::DistSquared2D(Point, Hero) < ClearanceSq)
				{
					return false;
				}
			}
			return true;
		}

		FVector RandomRingPoint(const FVector& Center, const FDSEnemySpawnRing& Ring, FRandomStream& Random)
		{
			const float Angle = Random.FRandRange(0.f, UE_TWO_PI);
			const float MinRadius = FMath::Min(Ring.MinRadiusCm, Ring.MaxRadiusCm);
			const float Radius = Random.FRandRange(MinRadius, FMath::Max(Ring.MinRadiusCm, Ring.MaxRadiusCm));
			return Center + FVector(FMath::Cos(Angle) * Radius, FMath::Sin(Angle) * Radius, 0.f);
		}
	}

	const FDSEnemySpawnEntry* PickRegularEntry(const TArray<FDSEnemySpawnEntry>& Entries, int32 Wave, FRandomStream& Random)
	{
		float TotalWeight = 0.f;
		for (const FDSEnemySpawnEntry& Entry : Entries)
		{
			TotalWeight += IsEntryAvailable(Entry, Wave) ? Entry.Weight : 0.f;
		}
		if (TotalWeight <= 0.f)
		{
			return nullptr;
		}

		float Roll = Random.FRandRange(0.f, TotalWeight);
		const FDSEnemySpawnEntry* Last = nullptr;
		for (const FDSEnemySpawnEntry& Entry : Entries)
		{
			if (!IsEntryAvailable(Entry, Wave))
			{
				continue;
			}
			Last = &Entry;
			Roll -= Entry.Weight;
			if (Roll <= 0.f)
			{
				return &Entry;
			}
		}
		return Last; // Arredondamento de float: fica com a última disponível.
	}

	const FDSBossSpawnEntry* FindBossEntry(const TArray<FDSBossSpawnEntry>& Bosses, int32 Wave)
	{
		const FDSBossSpawnEntry* Best = nullptr;
		for (const FDSBossSpawnEntry& Boss : Bosses)
		{
			if (!Boss.EnemyData)
			{
				continue;
			}
			if (Boss.Wave == Wave)
			{
				return &Boss;
			}
			if (Boss.Wave < Wave && (!Best || Boss.Wave > Best->Wave))
			{
				Best = &Boss;
			}
		}
		if (Best)
		{
			return Best;
		}
		const FDSBossSpawnEntry* First = Bosses.FindByPredicate([](const FDSBossSpawnEntry& Boss) { return Boss.EnemyData != nullptr; });
		return First;
	}

	FVector Centroid(const TArray<FVector>& Points, const FVector& Fallback)
	{
		if (Points.Num() == 0)
		{
			return Fallback;
		}
		FVector Sum = FVector::ZeroVector;
		for (const FVector& Point : Points)
		{
			Sum += Point;
		}
		return Sum / static_cast<double>(Points.Num());
	}

	FVector FindEnemySpawnLocation(UWorld* World, const FVector& Center, const TArray<FVector>& HeroLocations,
		const FDSEnemySpawnRing& Ring, FRandomStream& Random)
	{
		UNavigationSystemV1* NavSystem = FNavigationSystem::GetCurrent<UNavigationSystemV1>(World);
		FVector Candidate = Center;
		for (int32 Attempt = 0; Attempt < FMath::Max(1, Ring.MaxAttempts); ++Attempt)
		{
			Candidate = RandomRingPoint(Center, Ring, Random);
			if (NavSystem)
			{
				FNavLocation Projected;
				if (!NavSystem->ProjectPointToNavigation(Candidate, Projected, Ring.NavProjectionExtent))
				{
					continue; // Fora do navmesh (parede, água, penhasco).
				}
				Candidate = Projected.Location;
			}
			if (IsClearOfHeroes(Candidate, HeroLocations, Ring.PlayerClearanceCm))
			{
				return Candidate;
			}
		}
		return Candidate;
	}
}
