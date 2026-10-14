#include "Gameplay/Data/DSClassData.h"

#include "AbilitySystemComponent.h"
#include "Gameplay/Attributes/DSAttributeSet.h"

FPrimaryAssetId UDSClassData::GetPrimaryAssetId() const
{
	return FPrimaryAssetId(TEXT("DSClass"), GetFName());
}

FDSBaseStats UDSClassData::GetStatsForRank(int32 Rank) const
{
	FDSBaseStats Stats = BaseStats;
	const int32 RanksAboveOne = FMath::Clamp(Rank, 1, DSConstants::MaxClassRank) - 1;
	Stats.MaxHealth *= 1.f + HealthBonusPerRank * RanksAboveOne;
	Stats.Damage *= 1.f + DamageBonusPerRank * RanksAboveOne;
	return Stats;
}

void UDSClassData::ApplyBaseStats(UAbilitySystemComponent* ASC, int32 Rank) const
{
	if (!ASC || !ASC->IsOwnerActorAuthoritative())
	{
		return;
	}
	const FDSBaseStats Stats = GetStatsForRank(Rank);
	auto Set = [ASC](const FGameplayAttribute& Attribute, float Value)
	{
		ASC->SetNumericAttributeBase(Attribute, Value);
	};

	// Máximos antes dos atuais (o clamp usa o máximo).
	Set(UDSAttributeSet::GetMaxHealthAttribute(), Stats.MaxHealth);
	Set(UDSAttributeSet::GetMaxResourceAttribute(), Stats.MaxResource);
	Set(UDSAttributeSet::GetMaxStaminaAttribute(), Stats.MaxStamina);
	Set(UDSAttributeSet::GetHealthAttribute(), Stats.MaxHealth);
	Set(UDSAttributeSet::GetResourceAttribute(), Resource.bStartsEmpty ? 0.f : Stats.MaxResource);
	Set(UDSAttributeSet::GetStaminaAttribute(), Stats.MaxStamina);
	Set(UDSAttributeSet::GetShieldAttribute(), 0.f);

	Set(UDSAttributeSet::GetDamageAttribute(), Stats.Damage);
	Set(UDSAttributeSet::GetAttackSpeedAttribute(), Stats.AttackSpeed);
	Set(UDSAttributeSet::GetAttackRangeAttribute(), Stats.AttackRange);
	Set(UDSAttributeSet::GetMoveSpeedAttribute(), Stats.MoveSpeed);
	Set(UDSAttributeSet::GetCooldownReductionAttribute(), Stats.CooldownReduction);
	Set(UDSAttributeSet::GetCritChanceAttribute(), Stats.CritChance);
	Set(UDSAttributeSet::GetCritDamageAttribute(), Stats.CritDamage);
	Set(UDSAttributeSet::GetArmorAttribute(), Stats.Armor);
	Set(UDSAttributeSet::GetLifestealAttribute(), Stats.Lifesteal);
	Set(UDSAttributeSet::GetDodgeChanceAttribute(), Stats.DodgeChance);
	Set(UDSAttributeSet::GetHealthRegenAttribute(), Stats.HealthRegen);
	Set(UDSAttributeSet::GetResourceRegenAttribute(), Resource.RegenPerSecond);
	Set(UDSAttributeSet::GetDamageReductionAttribute(), 0.f);
}

float UDSClassData::GetMechanic(FName Key, float Default) const
{
	const float* Found = Mechanics.Find(Key);
	return Found ? *Found : Default;
}
