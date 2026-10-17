#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Engine/DataTable.h"
#include "Core/DSTypes.h"
#include "DSCardData.generated.h"

class UGameplayAbility;
class UTexture2D;

UENUM(BlueprintType)
enum class EDSCardRarity : uint8
{
	Common,
	Rare,
	Epic
};

UENUM(BlueprintType)
enum class EDSModifierOp : uint8
{
	Add,
	Mult
};

/** Modificador de carta (cards_node.ts CardModifier). Stat = nome do protótipo ("crit_chance"). */
USTRUCT(BlueprintType)
struct FDSCardModifier
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly) FName Stat;
	UPROPERTY(EditAnywhere, BlueprintReadOnly) EDSModifierOp Op = EDSModifierOp::Add;
	UPROPERTY(EditAnywhere, BlueprintReadOnly) float Value = 0.f;
};

/**
 * Nó da árvore de cartas (cards_node.ts CardDefinition). Também é linha de DataTable,
 * para importar as 192 cartas de JSON/CSV (ver Gameplay/README.md).
 */
USTRUCT(BlueprintType)
struct FDSCardDefinition : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly) FName CardId;
	UPROPERTY(EditAnywhere, BlueprintReadOnly) FText DisplayName;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (MultiLine = true)) FText Description;
	/** Árvore (TreeId do protótipo: critical/magic/defense/support). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly) EDSStance Stance = EDSStance::None;
	UPROPERTY(EditAnywhere, BlueprintReadOnly) EDSCardRarity Rarity = EDSCardRarity::Common;
	/** Peso no sorteio (0 = nunca sorteada). Padrão: comum 10, rara 5, épica 2, capstone 1. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly) float Weight = 10.f;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (ClampMin = 1)) int32 MaxStacks = 1;
	/** Coluna (ramo 0–3) e linha (tier 1–12). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly) int32 Branch = 0;
	UPROPERTY(EditAnywhere, BlueprintReadOnly) int32 Tier = 1;
	/** Só é sorteável quando um dos pais tem acúmulo (tier 1 = sem pais). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly) TArray<FName> Parents;
	UPROPERTY(EditAnywhere, BlueprintReadOnly) TArray<FDSCardModifier> Modifiers;
	/** Habilidade de apoio liberada (AbilityId do protótipo: lost_warrior_aura...). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly) FName GrantedAbilityId;
	UPROPERTY(EditAnywhere, BlueprintReadOnly) TSoftObjectPtr<UTexture2D> Icon;
};

/** Uma carta como asset individual (alternativa à DataTable). */
UCLASS(BlueprintType, Const)
class DUNGEONSTANCE_API UDSCardData : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	virtual FPrimaryAssetId GetPrimaryAssetId() const override;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Card", meta = (ShowOnlyInnerProperties))
	FDSCardDefinition Definition;
};

/**
 * Catálogo de cartas e habilidades de apoio da run. O GameMode (Core) mantém uma referência
 * e passa para UDSCardLibrary. Fontes: DataTable (FDSCardDefinition) + assets UDSCardData.
 */
UCLASS(BlueprintType, Const)
class DUNGEONSTANCE_API UDSCardRegistry : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	const FDSCardDefinition* FindCard(FName CardId) const;
	void GetCardsForStance(EDSStance Stance, TArray<const FDSCardDefinition*>& OutCards) const;
	int32 NumCards() const;

	/** Classe da habilidade de apoio para um AbilityId (nula se não mapeada). */
	TSubclassOf<UGameplayAbility> FindSupportAbility(FName AbilityId) const;

	/** Monta o catálogo a partir de structs (testes / geração em runtime). */
	void SetCardsForTesting(const TArray<FDSCardDefinition>& InCards);

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Cards", meta = (RequiredAssetDataTags = "RowStructure=/Script/DungeonStance.DSCardDefinition"))
	TObjectPtr<UDataTable> CardTable;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Cards")
	TArray<TObjectPtr<UDSCardData>> CardAssets;

	/** AbilityId (lost_warrior_aura, shared_bastion...) → Blueprint da UDSGA_SupportAura configurada. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Cards")
	TMap<FName, TSubclassOf<UGameplayAbility>> SupportAbilities;

private:
	void BuildCacheIfNeeded() const;

	mutable TMap<FName, FDSCardDefinition> Cache;
	mutable bool bCacheBuilt = false;
};
