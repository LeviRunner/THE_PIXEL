#include "Gameplay/Data/DSEnemyData.h"

#include "Gameplay/Combat/DSEnemyRules.h"

FPrimaryAssetId UDSEnemyData::GetPrimaryAssetId() const
{
	return FPrimaryAssetId(TEXT("DSEnemy"), GetFName());
}

FDSEnemyStats UDSEnemyData::ComputeStats(int32 Level, bool bElite) const
{
	return DSEnemyRules::ComputeStats(ArchetypeStats, Level, bElite, EliteHealthMultiplier, EliteDpsMultiplier,
		EliteRadiusBonus, ProjectileDpsFactor, SpeedMultiplier);
}

#if WITH_EDITOR
void UDSEnemyData::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);
	if (PropertyChangedEvent.GetPropertyName() == GET_MEMBER_NAME_CHECKED(UDSEnemyData, Archetype))
	{
		ArchetypeStats = DSEnemyRules::GetArchetypeDefaults(Archetype);
		bIsBoss = Archetype == EDSEnemyArchetype::Boss;
	}
}
#endif
