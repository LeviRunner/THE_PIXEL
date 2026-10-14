#include "Network/DSBackendTypes.h"

namespace DSBackendIdsPrivate
{
	struct FClassIdPair
	{
		EDSHeroClass HeroClass;
		const TCHAR* BackendId;
	};

	// Mesma ordem/ids de Nakama_Legacy_Reference/src/classes.ts (Wizard = "mage" no protótipo).
	static const FClassIdPair ClassIds[] =
	{
		{ EDSHeroClass::Barbarian,		TEXT("barbarian") },
		{ EDSHeroClass::Crusader,		TEXT("crusader") },
		{ EDSHeroClass::DemonHunter,	TEXT("demon_hunter") },
		{ EDSHeroClass::Monk,			TEXT("monk") },
		{ EDSHeroClass::Necromancer,	TEXT("necromancer") },
		{ EDSHeroClass::Arcanist,		TEXT("arcanist") },
		{ EDSHeroClass::BloodKnight,	TEXT("blood_knight") },
		{ EDSHeroClass::Wizard,			TEXT("mage") },
		{ EDSHeroClass::Archer,			TEXT("archer") },
	};
}

const FDSClassProgress* FDSPlayerProfile::FindClass(EDSHeroClass HeroClass) const
{
	return Classes.FindByPredicate([HeroClass](const FDSClassProgress& Entry)
	{
		return Entry.HeroClass == HeroClass;
	});
}

int32 FDSPlayerProfile::GetClassRank(EDSHeroClass HeroClass) const
{
	const FDSClassProgress* Entry = FindClass(HeroClass);
	return Entry ? Entry->Rank : 1;
}

FString DSBackendIds::HeroClassToBackendId(EDSHeroClass HeroClass)
{
	for (const DSBackendIdsPrivate::FClassIdPair& Pair : DSBackendIdsPrivate::ClassIds)
	{
		if (Pair.HeroClass == HeroClass)
		{
			return Pair.BackendId;
		}
	}
	return FString();
}

EDSHeroClass DSBackendIds::HeroClassFromBackendId(const FString& BackendId)
{
	for (const DSBackendIdsPrivate::FClassIdPair& Pair : DSBackendIdsPrivate::ClassIds)
	{
		if (BackendId.Equals(Pair.BackendId, ESearchCase::CaseSensitive))
		{
			return Pair.HeroClass;
		}
	}
	return EDSHeroClass::None;
}

bool DSBackendIds::IsSafeIdentifier(const FString& Value, int32 MinLength, int32 MaxLength)
{
	if (Value.Len() < MinLength || Value.Len() > MaxLength)
	{
		return false;
	}
	for (const TCHAR Char : Value)
	{
		const bool bAllowed = FChar::IsAlnum(Char) || Char == TEXT('-') || Char == TEXT('_');
		if (!bAllowed)
		{
			return false;
		}
	}
	return true;
}

bool DSBackendIds::ValidateRunResult(const FDSRunResult& Result, FString& OutReason)
{
	if (!IsSafeIdentifier(Result.PlayerId))
	{
		OutReason = TEXT("PlayerId vazio ou inválido");
		return false;
	}
	if (HeroClassToBackendId(Result.HeroClass).IsEmpty())
	{
		OutReason = TEXT("HeroClass inválida");
		return false;
	}
	if (Result.WavesCleared < 0 || Result.WavesCleared > DSConstants::MaxWaves)
	{
		OutReason = FString::Printf(TEXT("WavesCleared fora do intervalo: %d"), Result.WavesCleared);
		return false;
	}
	if (Result.Kills < 0 || Result.RankXPEarned < 0 || Result.GoldEarned < 0)
	{
		OutReason = TEXT("Kills/RankXPEarned/GoldEarned negativos");
		return false;
	}
	return true;
}
