#include "UI/DSUIDataAsset.h"

namespace
{
	template <typename TEnum>
	FText GetEnumDisplayName(TEnum Value)
	{
		return StaticEnum<TEnum>()->GetDisplayNameTextByValue(static_cast<int64>(Value));
	}
}

const FDSClassUIInfo* UDSUIDataAsset::FindClassInfo(EDSHeroClass HeroClass) const
{
	return Classes.Find(HeroClass);
}

const FDSStanceUIInfo* UDSUIDataAsset::FindStanceInfo(EDSStance Stance) const
{
	return Stances.Find(Stance);
}

const FDSStatusUIInfo* UDSUIDataAsset::FindStatusInfo(const FGameplayTag& StatusTag) const
{
	return Statuses.FindByPredicate([&StatusTag](const FDSStatusUIInfo& Info)
	{
		return Info.StatusTag == StatusTag;
	});
}

const FDSDisplayUIInfo* UDSUIDataAsset::FindAbilityInfo(const FGameplayTagContainer& AbilityAssetTags) const
{
	for (const FGameplayTag& Tag : AbilityAssetTags)
	{
		if (const FDSDisplayUIInfo* Info = Abilities.Find(Tag))
		{
			return Info;
		}
	}
	return nullptr;
}

const FDSDisplayUIInfo* UDSUIDataAsset::FindCardInfo(FName CardId) const
{
	return Cards.Find(CardId);
}

FText UDSUIDataAsset::GetHeroClassName(EDSHeroClass HeroClass) const
{
	const FDSClassUIInfo* Info = FindClassInfo(HeroClass);
	return (Info && !Info->DisplayName.IsEmpty()) ? Info->DisplayName : GetEnumDisplayName(HeroClass);
}

FText UDSUIDataAsset::GetStanceName(EDSStance Stance) const
{
	const FDSStanceUIInfo* Info = FindStanceInfo(Stance);
	return (Info && !Info->DisplayName.IsEmpty()) ? Info->DisplayName : GetEnumDisplayName(Stance);
}

FText UDSUIDataAsset::GetPartySizeName(EDSPartySize PartySize) const
{
	const FText* Name = PartySizeNames.Find(PartySize);
	return (Name && !Name->IsEmpty()) ? *Name : GetEnumDisplayName(PartySize);
}
