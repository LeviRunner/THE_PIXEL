#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GameplayTagContainer.h"
#include "DSSupportAbilityData.generated.h"

class ADSSupportZone;
class UTexture2D;

/** abilities.ts AbilityKind. */
UENUM(BlueprintType)
enum class EDSSupportKind : uint8
{
	DamageBuff	UMETA(ToolTip = "+Magnitude de dano por Duration"),
	Shield		UMETA(ToolTip = "Escudo de Magnitude"),
	Heal		UMETA(ToolTip = "Cura Magnitude × vida máxima"),
	Haste		UMETA(ToolTip = "+Magnitude de velocidade de ataque"),
	Zone		UMETA(ToolTip = "Área no chão: buffs/regen/redução para aliados dentro"),
	Rally		UMETA(ToolTip = "Reseta a esquiva e +Magnitude de movimento")
};

/**
 * Uma das 8 habilidades de apoio (abilities.ts ABILITY_DEFINITIONS). Distâncias em cm (px × 2,5).
 * Consumida por UDSGA_SupportAura. Crie um asset por habilidade (DA_Support_LostWarriorAura...).
 */
UCLASS(BlueprintType, Const)
class DUNGEONSTANCE_API UDSSupportAbilityData : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	virtual FPrimaryAssetId GetPrimaryAssetId() const override;

	/** AbilityId do protótipo (lost_warrior_aura...). Igual ao GrantedAbilityId das cartas. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Support") FName AbilityId;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Support") FText DisplayName;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Support", meta = (MultiLine = true)) FText Description;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Support") TSoftObjectPtr<UTexture2D> Icon;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Support") FLinearColor Color = FLinearColor::White;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Support") EDSSupportKind Kind = EDSSupportKind::DamageBuff;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Support", meta = (Units = "s")) float Cooldown = 20.f;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Support", meta = (Units = "cm")) float Radius = 700.f;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Support", meta = (Units = "s")) float Duration = 6.f;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Support") float Magnitude = 0.3f;
	/** Zona: redução de dano para aliados dentro (Santuário 0,3; Totem usa min(0,5, Magnitude)). */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Support") float DamageReduction = 0.f;
	/** Zona: armadura extra (Estandarte: 15 × (1 + poder)). */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Support") float ZoneArmorBonus = 0.f;
	/** Zona: Magnitude vira multiplicador de dano (Estandarte). */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Support") bool bZoneDamageBuff = false;
	/** Zona: Magnitude vira regeneração de % da vida máx./s (Santuário). */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Support") bool bZoneRegen = false;
	/** Zona: Magnitude vira redução de dano (Totem, máx. 0,5). */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Support") bool bZoneMagnitudeIsDamageReduction = false;
	/** Conta como cura (Renovação reduz a recarga). */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Support") bool bIsHeal = false;
	/** Totem: inimigos próximos preferem atacá-lo. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Support") bool bTaunts = false;

	/** Tag de recarga própria (Cooldown.Support.*). */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Support", meta = (Categories = "Cooldown.Support"))
	FGameplayTag CooldownTag;

	/** Ator da zona (Zone). Padrão: ADSSupportZone. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Support") TSubclassOf<ADSSupportZone> ZoneClass;
	/** GameplayCue ao lançar (VFX do círculo, som). */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Support", meta = (Categories = "GameplayCue")) FGameplayTag CastCueTag;
};
