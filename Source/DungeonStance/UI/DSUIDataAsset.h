#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GameplayTagContainer.h"
#include "Core/DSTypes.h"
#include "DSUIDataAsset.generated.h"

class UTexture2D;

/** Apresentação de uma classe jogável (nome, ícone, recurso). */
USTRUCT(BlueprintType)
struct FDSClassUIInfo
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "DS|UI")
	FText DisplayName;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "DS|UI", meta = (MultiLine = true))
	FText Description;

	/** Nome do recurso da classe (Fúria, Fé, Mana...). */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "DS|UI")
	FText ResourceName;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "DS|UI")
	FLinearColor ResourceColor = FLinearColor::White;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "DS|UI")
	TSoftObjectPtr<UTexture2D> Icon;
};

/** Apresentação de uma stance (caminho de cartas). */
USTRUCT(BlueprintType)
struct FDSStanceUIInfo
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "DS|UI")
	FText DisplayName;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "DS|UI", meta = (MultiLine = true))
	FText Description;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "DS|UI")
	FLinearColor Color = FLinearColor::White;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "DS|UI")
	TSoftObjectPtr<UTexture2D> Icon;
};

/** Ícone de status (Status.Bleed, Status.Stun...). A ordem do array define a ordem na tela. */
USTRUCT(BlueprintType)
struct FDSStatusUIInfo
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "DS|UI", meta = (Categories = "Status"))
	FGameplayTag StatusTag;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "DS|UI")
	FText DisplayName;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "DS|UI")
	TSoftObjectPtr<UTexture2D> Icon;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "DS|UI")
	bool bIsDebuff = true;
};

/** Ícone/nome genérico (habilidades e cartas). */
USTRUCT(BlueprintType)
struct FDSDisplayUIInfo
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "DS|UI")
	FText DisplayName;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "DS|UI", meta = (MultiLine = true))
	FText Description;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "DS|UI")
	TSoftObjectPtr<UTexture2D> Icon;
};

/**
 * Tabela de apresentação da UI (nomes, ícones, cores). Evita assets fixos no C++.
 * Instância esperada: DA_DSUIData, referenciada no BP_DSHUD.
 */
UCLASS(BlueprintType)
class DUNGEONSTANCE_API UDSUIDataAsset : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	const FDSClassUIInfo* FindClassInfo(EDSHeroClass HeroClass) const;
	const FDSStanceUIInfo* FindStanceInfo(EDSStance Stance) const;
	const FDSStatusUIInfo* FindStatusInfo(const FGameplayTag& StatusTag) const;

	/** Procura a primeira tag de asset da habilidade que tenha entrada em Abilities. */
	const FDSDisplayUIInfo* FindAbilityInfo(const FGameplayTagContainer& AbilityAssetTags) const;
	const FDSDisplayUIInfo* FindCardInfo(FName CardId) const;

	/** Nome exibido; usa o DisplayName do enum quando não houver entrada. */
	UFUNCTION(BlueprintPure, Category = "DS|UI")
	FText GetHeroClassName(EDSHeroClass HeroClass) const;

	UFUNCTION(BlueprintPure, Category = "DS|UI")
	FText GetStanceName(EDSStance Stance) const;

	UFUNCTION(BlueprintPure, Category = "DS|UI")
	FText GetPartySizeName(EDSPartySize PartySize) const;

	const TArray<FDSStatusUIInfo>& GetStatuses() const { return Statuses; }

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "DS|UI")
	TMap<EDSHeroClass, FDSClassUIInfo> Classes;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "DS|UI")
	TMap<EDSStance, FDSStanceUIInfo> Stances;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "DS|UI")
	TMap<EDSPartySize, FText> PartySizeNames;

	/** Status exibidos nos quadros da party (só os listados aqui aparecem). */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "DS|UI")
	TArray<FDSStatusUIInfo> Statuses;

	/** Chave: tag de asset da habilidade (ex.: Ability.Support.Sanctuary). */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "DS|UI", meta = (Categories = "Ability"))
	TMap<FGameplayTag, FDSDisplayUIInfo> Abilities;

	/**
	 * Fallback de apresentação das cartas por CardId.
	 * TODO(Gameplay): ler nome/ícone direto de UDSCardData quando a API estiver fechada.
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "DS|UI")
	TMap<FName, FDSDisplayUIInfo> Cards;
};
