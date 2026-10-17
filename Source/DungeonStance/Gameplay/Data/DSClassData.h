#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Core/DSTypes.h"
#include "DSClassData.generated.h"

class UAbilitySystemComponent;
class UDSAbilitySet;
class ADSProjectile;
class UTexture2D;

/** Atributos base de uma classe (classes.ts BaseStats convertidos: 1 px = 2,5 cm). */
USTRUCT(BlueprintType)
struct FDSBaseStats
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly) float MaxHealth = 1000.f;
	UPROPERTY(EditAnywhere, BlueprintReadOnly) float Damage = 25.f;
	/** Ataques por segundo. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly) float AttackSpeed = 1.f;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (Units = "cm")) float AttackRange = 200.f;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (Units = "cm/s")) float MoveSpeed = 500.f;
	UPROPERTY(EditAnywhere, BlueprintReadOnly) float MaxResource = 100.f;
	UPROPERTY(EditAnywhere, BlueprintReadOnly) float CooldownReduction = 0.f;
	UPROPERTY(EditAnywhere, BlueprintReadOnly) float CritChance = 0.05f;
	UPROPERTY(EditAnywhere, BlueprintReadOnly) float CritDamage = 1.5f;
	UPROPERTY(EditAnywhere, BlueprintReadOnly) float Armor = 10.f;
	UPROPERTY(EditAnywhere, BlueprintReadOnly) float Lifesteal = 0.f;
	/** Esquiva passiva (Caçador de Demônios: 0,12). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly) float DodgeChance = 0.f;
	UPROPERTY(EditAnywhere, BlueprintReadOnly) float HealthRegen = 0.f;
	UPROPERTY(EditAnywhere, BlueprintReadOnly) float MaxStamina = 100.f;
	UPROPERTY(EditAnywhere, BlueprintReadOnly) int32 SummonLimit = 0;
	/** Esquiva (dash): 170 px = 425 cm, recarga 1,2 s, ~0,24 s de invulnerabilidade. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (Units = "cm")) float DodgeDistance = 425.f;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (Units = "s")) float DodgeCooldown = 1.2f;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (Units = "s")) float DodgeInvulnerability = 0.24f;
};

/** Recurso da classe (Fúria, Mana...). */
USTRUCT(BlueprintType)
struct FDSResourceRules
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly) FText DisplayName;
	/** Por segundo (negativo = escoa). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly) float RegenPerSecond = 0.f;
	/** Ganho por ataque que acerta. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly) float GainOnHit = 0.f;
	/** Ganho ao sofrer dano: GainOnDamaged × dano / 10 (BasePlayer.gd). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly) float GainOnDamaged = 0.f;
	/** Bárbaro e Cavaleiro Sangrento começam vazios. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly) bool bStartsEmpty = false;
	/** Bônus de dano proporcional à barra (Fúria do Bárbaro: 0,3). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly) float FullResourceDamageBonus = 0.f;
};

/** Forma do ataque básico (classes.ts AttackKind). */
USTRUCT(BlueprintType)
struct FDSBasicAttackInfo
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly) EDSAttackType AttackType = EDSAttackType::Melee;
	/** Arco do golpe corpo a corpo. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (Units = "deg")) float MeleeArcDegrees = 110.f;
	UPROPERTY(EditAnywhere, BlueprintReadOnly) TSubclassOf<ADSProjectile> ProjectileClass;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (Units = "cm/s")) float ProjectileSpeed = 2000.f;
	/** Raio da explosão (projectile_aoe). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (Units = "cm")) float AoERadius = 200.f;
	/** Monge: a cada N ataques o golpe causa ComboFinisherMultiplier (0 = sem combo). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly) int32 ComboLength = 0;
	UPROPERTY(EditAnywhere, BlueprintReadOnly) float ComboFinisherMultiplier = 2.f;
};

/**
 * Definição de uma classe jogável (classes.ts + CLASSES.md). Um asset por classe: DA_Class_Barbarian etc.
 * Concede os AbilitySets (ataque, skill, comuns) e inicializa os atributos no servidor.
 */
UCLASS(BlueprintType, Const)
class DUNGEONSTANCE_API UDSClassData : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	virtual FPrimaryAssetId GetPrimaryAssetId() const override;

	/** Atributos com o bônus de rank (+1% vida, +0,6% dano por rank acima do 1). */
	FDSBaseStats GetStatsForRank(int32 Rank) const;

	/** Define os valores base dos atributos no ASC (servidor). */
	void ApplyBaseStats(UAbilitySystemComponent* ASC, int32 Rank) const;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Class") EDSHeroClass HeroClass = EDSHeroClass::None;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Class") FText DisplayName;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Class") FText Role;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Class", meta = (MultiLine = true)) FText Description;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Class") TSoftObjectPtr<UTexture2D> Icon;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Class") FLinearColor Color = FLinearColor::White;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Stats") FDSBaseStats BaseStats;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Stats") FDSResourceRules Resource;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Stats") FDSBasicAttackInfo BasicAttack;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Progression") float HealthBonusPerRank = 0.01f;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Progression") float DamageBonusPerRank = 0.006f;

	/** Habilidades concedidas (ataque básico, skill da classe, esquiva/sprint comuns). */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Abilities") TArray<TObjectPtr<UDSAbilitySet>> AbilitySets;

	/** Ajustes específicos (cleave_radius, aura_radius, summon_damage...) — classes.ts mechanics. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Abilities") TMap<FName, float> Mechanics;

	UFUNCTION(BlueprintPure, Category = "DS|Class")
	float GetMechanic(FName Key, float Default = 0.f) const;
};
