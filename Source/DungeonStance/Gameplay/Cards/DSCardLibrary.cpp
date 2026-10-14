#include "Gameplay/Cards/DSCardLibrary.h"

#include "AbilitySystemComponent.h"
#include "Math/RandomStream.h"
#include "Core/DSPlayerState.h"
#include "Gameplay/Abilities/DSAbilitySystemComponent.h"
#include "Gameplay/Attributes/DSAttributeSet.h"
#include "Gameplay/Cards/DSCardStatBindings.h"
#include "Gameplay/Characters/DSPlayerStateBridge.h"
#include "Gameplay/Data/DSCardData.h"
#include "Gameplay/Data/DSGameplaySettings.h"
#include "Gameplay/DSGameplayTagsGameplay.h"
#include "Gameplay/Effects/DSGameplayEffects.h"
#include "DungeonStance.h"

namespace
{
	TWeakObjectPtr<const UDSCardRegistry> GRegistryOverride;
}

// ---------------------------------------------------------------------------
// Regras puras
// ---------------------------------------------------------------------------

int32 UDSCardLibrary::GetStacks(const TMap<FName, int32>& Stacks, FName CardId)
{
	const int32* Found = Stacks.Find(CardId);
	return Found ? *Found : 0;
}

bool UDSCardLibrary::ParentsOwned(const FDSCardDefinition& Card, const TMap<FName, int32>& Stacks)
{
	if (Card.Parents.Num() == 0)
	{
		return true;
	}
	return Card.Parents.ContainsByPredicate([&Stacks](const FName& ParentId)
	{
		return GetStacks(Stacks, ParentId) > 0;
	});
}

bool UDSCardLibrary::IsCardRollable(const FDSCardDefinition& Card, const TMap<FName, int32>& Stacks)
{
	return Card.Weight > 0.f && GetStacks(Stacks, Card.CardId) < Card.MaxStacks && ParentsOwned(Card, Stacks);
}

TArray<FName> UDSCardLibrary::RollFromPool(const TArray<const FDSCardDefinition*>& Pool, const TMap<FName, int32>& Stacks, int32 NumChoices, FRandomStream& Random)
{
	TArray<const FDSCardDefinition*> Candidates;
	for (const FDSCardDefinition* Card : Pool)
	{
		if (Card && IsCardRollable(*Card, Stacks))
		{
			Candidates.Add(Card);
		}
	}

	TArray<FName> Picked;
	while (Picked.Num() < NumChoices && Candidates.Num() > 0)
	{
		float Total = 0.f;
		for (const FDSCardDefinition* Card : Candidates)
		{
			Total += Card->Weight;
		}
		float Roll = Random.FRand() * Total;
		int32 Index = Candidates.Num() - 1;
		for (int32 i = 0; i < Candidates.Num(); ++i)
		{
			Roll -= Candidates[i]->Weight;
			if (Roll < 0.f)
			{
				Index = i;
				break;
			}
		}
		Picked.Add(Candidates[Index]->CardId);
		Candidates.RemoveAt(Index);
	}
	return Picked;
}

EDSCardPickResult UDSCardLibrary::ValidatePickRules(const FDSCardDefinition* Card, EDSStance Stance, const TMap<FName, int32>& Stacks, int32 PendingPicks)
{
	if (!Card)
	{
		return EDSCardPickResult::UnknownCard;
	}
	if (Card->Stance != Stance)
	{
		return EDSCardPickResult::WrongTree;
	}
	if (GetStacks(Stacks, Card->CardId) >= Card->MaxStacks)
	{
		return EDSCardPickResult::MaxStacks;
	}
	if (!ParentsOwned(*Card, Stacks))
	{
		return EDSCardPickResult::ParentNotOwned;
	}
	if (Card->Weight <= 0.f)
	{
		return EDSCardPickResult::NotAvailable;
	}
	if (PendingPicks <= 0)
	{
		return EDSCardPickResult::NoPickAvailable;
	}
	return EDSCardPickResult::Ok;
}

void UDSCardLibrary::GetCardStatParts(const UDSCardRegistry* Registry, const TMap<FName, int32>& Stacks, FName Stat, float& OutAdd, float& OutMult)
{
	OutAdd = 0.f;
	OutMult = 1.f;
	if (!Registry)
	{
		return;
	}
	for (const TPair<FName, int32>& Pair : Stacks)
	{
		const FDSCardDefinition* Card = Registry->FindCard(Pair.Key);
		if (!Card)
		{
			continue;
		}
		for (int32 n = 0; n < Pair.Value; ++n)
		{
			for (const FDSCardModifier& Mod : Card->Modifiers)
			{
				if (Mod.Stat != Stat)
				{
					continue;
				}
				if (Mod.Op == EDSModifierOp::Mult)
				{
					OutMult *= Mod.Value;
				}
				else
				{
					OutAdd += Mod.Value;
				}
			}
		}
	}
}

float UDSCardLibrary::AggregateCardStat(const UDSCardRegistry* Registry, const TMap<FName, int32>& Stacks, FName Stat, float Base)
{
	float Add = 0.f;
	float Mult = 1.f;
	GetCardStatParts(Registry, Stacks, Stat, Add, Mult);
	return (Base + Add) * Mult;
}

FString UDSCardLibrary::PickResultToString(EDSCardPickResult Result)
{
	// Mesmas mensagens do protótipo (cards.ts) para facilitar logs/telemetria.
	switch (Result)
	{
	case EDSCardPickResult::Ok:              return TEXT("ok");
	case EDSCardPickResult::UnknownCard:     return TEXT("unknown card");
	case EDSCardPickResult::WrongTree:       return TEXT("card not in the selected tree");
	case EDSCardPickResult::MaxStacks:       return TEXT("card at max stacks");
	case EDSCardPickResult::ParentNotOwned:  return TEXT("parent not owned");
	case EDSCardPickResult::NotAvailable:    return TEXT("card not available");
	case EDSCardPickResult::NoPickAvailable: return TEXT("no level-up pick available");
	default:                                 return TEXT("invalid");
	}
}

// ---------------------------------------------------------------------------
// Com registro explícito
// ---------------------------------------------------------------------------

TArray<FName> UDSCardLibrary::RollCardChoices(const UDSCardRegistry* Registry, EDSStance Stance, const TMap<FName, int32>& Stacks, int32 NumChoices, FRandomStream& Random)
{
	if (!Registry || NumChoices <= 0)
	{
		return TArray<FName>();
	}
	TArray<const FDSCardDefinition*> Pool;
	Registry->GetCardsForStance(Stance, Pool);
	return RollFromPool(Pool, Stacks, NumChoices, Random);
}

EDSCardPickResult UDSCardLibrary::ValidateCardPick(const UDSCardRegistry* Registry, FName CardId, EDSStance Stance, const TMap<FName, int32>& Stacks, int32 PendingPicks)
{
	return ValidatePickRules(Registry ? Registry->FindCard(CardId) : nullptr, Stance, Stacks, PendingPicks);
}

// ---------------------------------------------------------------------------
// API do Core
// ---------------------------------------------------------------------------

TArray<FName> UDSCardLibrary::RollCardChoices(EDSStance Stance, const TArray<FDSOwnedCard>& OwnedCards, int32 NumChoices, FRandomStream& Random)
{
	return RollCardChoices(GetCardRegistry(), Stance, DSPlayerStateBridge::ToStackMap(OwnedCards), NumChoices, Random);
}

bool UDSCardLibrary::ValidateCardPick(FName CardId, EDSStance Stance, const TArray<FDSOwnedCard>& OwnedCards, int32 PendingPicks, FString& OutReason)
{
	const EDSCardPickResult Result = ValidateCardPick(GetCardRegistry(), CardId, Stance, DSPlayerStateBridge::ToStackMap(OwnedCards), PendingPicks);
	OutReason = PickResultToString(Result);
	return Result == EDSCardPickResult::Ok;
}

bool UDSCardLibrary::ApplyCardToPlayer(ADSPlayerState* PlayerState, FName CardId)
{
	const UDSCardRegistry* Registry = GetCardRegistry();
	UDSAbilitySystemComponent* ASC = DSPlayerStateBridge::GetASC(PlayerState);
	const FDSCardDefinition* Card = Registry ? Registry->FindCard(CardId) : nullptr;
	if (!Card || !ASC || !ASC->IsOwnerActorAuthoritative())
	{
		return false;
	}
	// O Core chama isto antes de AddCardStack: conta o acúmulo novo.
	TMap<FName, int32> Stacks = DSPlayerStateBridge::GetCardStacks(PlayerState);
	Stacks.FindOrAdd(CardId) += 1;

	ApplyCardStatsToASC(ASC, Registry, Stacks);
	GrantCardAbility(ASC, Registry, *Card);
	return true;
}

// ---------------------------------------------------------------------------
// GAS
// ---------------------------------------------------------------------------

void UDSCardLibrary::ApplyCardStatsToASC(UAbilitySystemComponent* ASC, const UDSCardRegistry* Registry, const TMap<FName, int32>& Stacks)
{
	if (!ASC || !ASC->IsOwnerActorAuthoritative())
	{
		return;
	}
	const FGameplayEffectSpecHandle Spec = ASC->MakeOutgoingSpec(UDSGE_CardStats::StaticClass(), 1.f, ASC->MakeEffectContext());
	if (!Spec.IsValid())
	{
		return;
	}
	// Remover e reaplicar mexe na vida máxima duas vezes; guarda a vida para não ganhar/perder no processo.
	const float OldHealth = ASC->GetNumericAttribute(UDSAttributeSet::GetHealthAttribute());
	const float OldMaxHealth = ASC->GetNumericAttribute(UDSAttributeSet::GetMaxHealthAttribute());
	ASC->RemoveActiveGameplayEffectBySourceEffect(UDSGE_CardStats::StaticClass(), nullptr);

	for (const FDSCardStatBinding& Binding : DSCardStats::GetBindings())
	{
		float Add = 0.f;
		float Mult = 1.f;
		GetCardStatParts(Registry, Stacks, Binding.CardStat, Add, Mult);
		Spec.Data->SetSetByCallerMagnitude(DSGameplayTags::GetCardSetByCallerTag(Binding.AttributeName, false), Add);
		Spec.Data->SetSetByCallerMagnitude(DSGameplayTags::GetCardSetByCallerTag(Binding.AttributeName, true), Mult);
	}
	ASC->ApplyGameplayEffectSpecToSelf(*Spec.Data.Get());

	if (OldHealth > 0.f)
	{
		const float NewMaxHealth = ASC->GetNumericAttribute(UDSAttributeSet::GetMaxHealthAttribute());
		const float NewHealth = FMath::Clamp(OldHealth + FMath::Max(0.f, NewMaxHealth - OldMaxHealth), 1.f, NewMaxHealth);
		ASC->SetNumericAttributeBase(UDSAttributeSet::GetHealthAttribute(), NewHealth);
	}
}

bool UDSCardLibrary::GrantCardAbility(UDSAbilitySystemComponent* ASC, const UDSCardRegistry* Registry, const FDSCardDefinition& Card)
{
	if (!ASC || !Registry || Card.GrantedAbilityId.IsNone() || !ASC->IsOwnerActorAuthoritative())
	{
		return false;
	}
	const TSubclassOf<UGameplayAbility> AbilityClass = Registry->FindSupportAbility(Card.GrantedAbilityId);
	if (!AbilityClass)
	{
		UE_LOG(LogDungeonStance, Warning, TEXT("Carta %s: habilidade %s sem classe no UDSCardRegistry."), *Card.CardId.ToString(), *Card.GrantedAbilityId.ToString());
		return false;
	}
	if (ASC->FindAbilitySpecFromClass(AbilityClass))
	{
		return true; // já concedida
	}
	const FGameplayTag Slot = ASC->FindFreeSupportInputTag();
	if (!Slot.IsValid())
	{
		// TODO(Gameplay+UI): 5ª habilidade substitui uma à escolha do jogador (CardSelectionUI.gd).
		UE_LOG(LogDungeonStance, Warning, TEXT("Slots de apoio cheios; %s não concedida."), *Card.GrantedAbilityId.ToString());
		return false;
	}
	return ASC->GrantAbilityWithInputTag(AbilityClass, Slot, 1, nullptr).IsValid();
}

void UDSCardLibrary::ReapplyAllCards(UDSAbilitySystemComponent* ASC, const UDSCardRegistry* Registry, const TMap<FName, int32>& Stacks)
{
	if (!ASC || !Registry)
	{
		return;
	}
	ApplyCardStatsToASC(ASC, Registry, Stacks);
	for (const TPair<FName, int32>& Pair : Stacks)
	{
		const FDSCardDefinition* Card = Registry->FindCard(Pair.Key);
		if (Card && Pair.Value > 0)
		{
			GrantCardAbility(ASC, Registry, *Card);
		}
	}
}

// ---------------------------------------------------------------------------
// Registro
// ---------------------------------------------------------------------------

void UDSCardLibrary::SetCardRegistryOverride(const UDSCardRegistry* Registry)
{
	GRegistryOverride = Registry;
}

const UDSCardRegistry* UDSCardLibrary::GetCardRegistry()
{
	if (const UDSCardRegistry* Override = GRegistryOverride.Get())
	{
		return Override;
	}
	const UDSGameplaySettings* Settings = GetDefault<UDSGameplaySettings>();
	const UDSCardRegistry* Registry = Settings ? Settings->CardRegistry.LoadSynchronous() : nullptr;
	if (!Registry)
	{
		UE_LOG(LogDungeonStance, Error, TEXT("UDSCardRegistry não configurado (UDSGameplaySettings::CardRegistry)."));
	}
	return Registry;
}
