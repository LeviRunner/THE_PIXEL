#include "Gameplay/Data/DSCardData.h"

#include "Abilities/GameplayAbility.h"
#include "DungeonStance.h"

FPrimaryAssetId UDSCardData::GetPrimaryAssetId() const
{
	return FPrimaryAssetId(TEXT("DSCard"), GetFName());
}

void UDSCardRegistry::BuildCacheIfNeeded() const
{
	if (bCacheBuilt)
	{
		return;
	}
	bCacheBuilt = true;
	Cache.Reset();

	if (CardTable)
	{
		static const FString Context(TEXT("UDSCardRegistry"));
		TArray<FDSCardDefinition*> Rows;
		CardTable->GetAllRows<FDSCardDefinition>(Context, Rows);
		for (const FDSCardDefinition* Row : Rows)
		{
			if (Row && !Row->CardId.IsNone())
			{
				Cache.Add(Row->CardId, *Row);
			}
		}
	}
	for (const UDSCardData* Asset : CardAssets)
	{
		if (Asset && !Asset->Definition.CardId.IsNone())
		{
			if (Cache.Contains(Asset->Definition.CardId))
			{
				UE_LOG(LogDungeonStance, Warning, TEXT("Carta duplicada no registro: %s (o asset sobrescreve a linha da tabela)."), *Asset->Definition.CardId.ToString());
			}
			Cache.Add(Asset->Definition.CardId, Asset->Definition);
		}
	}
}

const FDSCardDefinition* UDSCardRegistry::FindCard(FName CardId) const
{
	BuildCacheIfNeeded();
	return Cache.Find(CardId);
}

void UDSCardRegistry::GetCardsForStance(EDSStance Stance, TArray<const FDSCardDefinition*>& OutCards) const
{
	BuildCacheIfNeeded();
	OutCards.Reset();
	for (const TPair<FName, FDSCardDefinition>& Pair : Cache)
	{
		if (Pair.Value.Stance == Stance)
		{
			OutCards.Add(&Pair.Value);
		}
	}
	// Ordem estável (o TMap não garante ordem): necessário para sorteio determinístico por seed.
	OutCards.Sort([](const FDSCardDefinition& A, const FDSCardDefinition& B)
	{
		return A.Tier != B.Tier ? A.Tier < B.Tier : (A.Branch != B.Branch ? A.Branch < B.Branch : A.CardId.LexicalLess(B.CardId));
	});
}

int32 UDSCardRegistry::NumCards() const
{
	BuildCacheIfNeeded();
	return Cache.Num();
}

TSubclassOf<UGameplayAbility> UDSCardRegistry::FindSupportAbility(FName AbilityId) const
{
	const TSubclassOf<UGameplayAbility>* Found = SupportAbilities.Find(AbilityId);
	return Found ? *Found : nullptr;
}

void UDSCardRegistry::SetCardsForTesting(const TArray<FDSCardDefinition>& InCards)
{
	Cache.Reset();
	for (const FDSCardDefinition& Card : InCards)
	{
		Cache.Add(Card.CardId, Card);
	}
	bCacheBuilt = true;
}
