#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Core/DSTypes.h"
#include "DSCardLibrary.generated.h"

class ADSPlayerState;
class UAbilitySystemComponent;
class UDSAbilitySystemComponent;
class UDSCardRegistry;
struct FDSCardDefinition;
struct FRandomStream;

/** Resultado da validação de escolha (cards.ts validateCardPick). */
UENUM(BlueprintType)
enum class EDSCardPickResult : uint8
{
	Ok,
	UnknownCard,
	WrongTree,
	MaxStacks,
	ParentNotOwned,
	NotAvailable,
	NoPickAvailable
};

/**
 * Regras das cartas (port de cards.ts) + aplicação no GAS.
 *
 * API usada pelo Core (Core/DSCardBridge.cpp) — usa o registro de UDSGameplaySettings:
 *   RollCardChoices(EDSStance, const TArray<FDSOwnedCard>&, int32 NumChoices, FRandomStream&) -> TArray<FName>
 *   ValidateCardPick(FName, EDSStance, const TArray<FDSOwnedCard>&, int32 PendingPicks, FString& OutReason) -> bool
 *   ApplyCardToPlayer(ADSPlayerState*, FName CardId) -> bool   (chamado ANTES de AddCardStack: conta +1 acúmulo)
 *
 * Overloads com UDSCardRegistry explícito existem para testes e ferramentas.
 */
UCLASS()
class DUNGEONSTANCE_API UDSCardLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	// --- Regras puras (cards.ts) ---
	/** Tier 1 sempre liberado; os demais precisam de um pai com acúmulo. */
	static bool ParentsOwned(const FDSCardDefinition& Card, const TMap<FName, int32>& Stacks);
	/** Peso > 0, abaixo do máximo de acúmulos e com pai possuído. */
	static bool IsCardRollable(const FDSCardDefinition& Card, const TMap<FName, int32>& Stacks);
	/** Sorteio ponderado sem reposição (CardSelectionUI.gd roll_cards). */
	static TArray<FName> RollFromPool(const TArray<const FDSCardDefinition*>& Pool, const TMap<FName, int32>& Stacks, int32 NumChoices, FRandomStream& Random);
	/** Card nulo = carta desconhecida. PendingPicks substitui "picksUsed < teamLevel − 1" do protótipo. */
	static EDSCardPickResult ValidatePickRules(const FDSCardDefinition* Card, EDSStance Stance, const TMap<FName, int32>& Stacks, int32 PendingPicks);
	/** (base + Σadd) × Πmult sobre as cartas possuídas (aggregateCardStat). */
	static float AggregateCardStat(const UDSCardRegistry* Registry, const TMap<FName, int32>& Stacks, FName Stat, float Base);
	/** Partes separadas (Σadd, Πmult) de um stat. */
	static void GetCardStatParts(const UDSCardRegistry* Registry, const TMap<FName, int32>& Stacks, FName Stat, float& OutAdd, float& OutMult);
	static FString PickResultToString(EDSCardPickResult Result);

	// --- Com registro explícito ---
	static TArray<FName> RollCardChoices(const UDSCardRegistry* Registry, EDSStance Stance, const TMap<FName, int32>& Stacks, int32 NumChoices, FRandomStream& Random);
	static EDSCardPickResult ValidateCardPick(const UDSCardRegistry* Registry, FName CardId, EDSStance Stance, const TMap<FName, int32>& Stacks, int32 PendingPicks);

	// --- API do Core (registro global) ---
	static TArray<FName> RollCardChoices(EDSStance Stance, const TArray<FDSOwnedCard>& OwnedCards, int32 NumChoices, FRandomStream& Random);
	static bool ValidateCardPick(FName CardId, EDSStance Stance, const TArray<FDSOwnedCard>& OwnedCards, int32 PendingPicks, FString& OutReason);
	static bool ApplyCardToPlayer(ADSPlayerState* PlayerState, FName CardId);

	// --- Aplicação no GAS (servidor) ---
	/** Remove o UDSGE_CardStats anterior e aplica um novo com os modificadores agregados. */
	static void ApplyCardStatsToASC(UAbilitySystemComponent* ASC, const UDSCardRegistry* Registry, const TMap<FName, int32>& Stacks);
	/** Concede a habilidade de apoio da carta no primeiro slot livre (Input.Support.1..4). */
	static bool GrantCardAbility(UDSAbilitySystemComponent* ASC, const UDSCardRegistry* Registry, const FDSCardDefinition& Card);
	/** Reaplica tudo a partir das cartas possuídas (novo pawn, reconexão). */
	static void ReapplyAllCards(UDSAbilitySystemComponent* ASC, const UDSCardRegistry* Registry, const TMap<FName, int32>& Stacks);

	// --- Registro ---
	/** Override (testes / GameMode). Nulo = volta a usar UDSGameplaySettings. */
	static void SetCardRegistryOverride(const UDSCardRegistry* Registry);
	static const UDSCardRegistry* GetCardRegistry();

	/** Acúmulos de uma carta no mapa (0 se ausente). */
	static int32 GetStacks(const TMap<FName, int32>& Stacks, FName CardId);
};
