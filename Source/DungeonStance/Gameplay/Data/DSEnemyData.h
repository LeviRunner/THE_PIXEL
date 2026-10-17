#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "DSEnemyData.generated.h"

class ADSProjectile;
class UDSAbilitySet;
class USkeletalMesh;

/** Arquétipos de bestiary.ts §8.1. */
UENUM(BlueprintType)
enum class EDSEnemyArchetype : uint8
{
	Minion,
	Runner,
	Brute,
	Shooter,
	Caster,
	Swarm,
	Passive,
	Boss
};

/** Comportamentos de IA (bestiary_ai.ts). */
UENUM(BlueprintType)
enum class EDSEnemyBehavior : uint8
{
	Melee		UMETA(ToolTip = "Persegue e bate"),
	Swarm		UMETA(ToolTip = "Persegue em bando, abrindo em leque para cercar"),
	Charger		UMETA(ToolTip = "Persegue e dá uma investida a 2,4x quando perto"),
	Blinker		UMETA(ToolTip = "Corredor que teleporta para trás do alvo com pouca vida"),
	Kiter		UMETA(ToolTip = "À distância: recua quando um herói chega perto"),
	Ranged		UMETA(ToolTip = "À distância: para a 80% do alcance e atira"),
	Passive		UMETA(ToolTip = "Vagueia até apanhar; foge com pouca vida"),
	Brood		UMETA(ToolTip = "Chefe investidor que invoca enxames a 60% e 30% da vida")
};

/** Multiplicadores do arquétipo sobre o minion padrão do nível (bestiary.ts ARCHETYPES, em cm). */
USTRUCT(BlueprintType)
struct FDSEnemyArchetypeStats
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly) float HealthMultiplier = 1.f;
	UPROPERTY(EditAnywhere, BlueprintReadOnly) float DpsMultiplier = 1.f;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (Units = "cm/s")) float MoveSpeed = 175.f;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (Units = "cm")) float Radius = 35.f;
	UPROPERTY(EditAnywhere, BlueprintReadOnly) bool bRanged = false;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (Units = "s", EditCondition = "bRanged")) float AttackCooldown = 0.f;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (Units = "cm", EditCondition = "bRanged")) float AttackRange = 0.f;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (Units = "cm/s", EditCondition = "bRanged")) float ProjectileSpeed = 0.f;
};

/** Atributos finais de um inimigo num nível (antes da escala de onda). */
USTRUCT(BlueprintType)
struct FDSEnemyStats
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly) float MaxHealth = 0.f;
	UPROPERTY(BlueprintReadOnly) float ContactDps = 0.f;
	UPROPERTY(BlueprintReadOnly) float ProjectileDamage = 0.f;
	UPROPERTY(BlueprintReadOnly) float Armor = 0.f;
	UPROPERTY(BlueprintReadOnly) float MoveSpeed = 0.f;
	UPROPERTY(BlueprintReadOnly) float Radius = 0.f;
	UPROPERTY(BlueprintReadOnly) float AttackRange = 0.f;
	UPROPERTY(BlueprintReadOnly) float AttackCooldown = 0.f;
	UPROPERTY(BlueprintReadOnly) float ProjectileSpeed = 0.f;
	UPROPERTY(BlueprintReadOnly) int32 CombatXP = 0;
	UPROPERTY(BlueprintReadOnly) int32 AverageGold = 0;
};

/** Ajustes de IA (bestiary_ai.ts, convertidos para cm/s). */
USTRUCT(BlueprintType)
struct FDSEnemyAITuning
{
	GENERATED_BODY()

	/** Corpo a corpo: distância (além dos raios) para bater. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (Units = "cm")) float MeleeReach = 60.f;
	/** Intervalo dos golpes corpo a corpo; dano por golpe = DPS × intervalo. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (Units = "s")) float MeleeInterval = 1.f;
	/** À distância: para a esta fração do alcance. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly) float RangedStopFraction = 0.8f;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (Units = "cm")) float LungeRange = 650.f;
	UPROPERTY(EditAnywhere, BlueprintReadOnly) float LungeSpeedMultiplier = 2.4f;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (Units = "s")) float LungeDuration = 0.8f;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (Units = "s")) float LungeCooldown = 4.f;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (Units = "cm")) float KiteDistance = 350.f;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (Units = "cm")) float KiteStep = 250.f;
	UPROPERTY(EditAnywhere, BlueprintReadOnly) float BlinkHealthFraction = 0.35f;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (Units = "cm")) float BlinkDistance = 600.f;
	UPROPERTY(EditAnywhere, BlueprintReadOnly) float FleeHealthFraction = 0.3f;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (Units = "cm")) float FleeStep = 500.f;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (Units = "cm")) float WanderRadius = 400.f;
	/** Swarm: ângulo de flanco (rad) e distância em que para de abrir o leque. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly) float SwarmFlankRadians = 0.35f;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (Units = "cm")) float SwarmCloseDistance = 225.f;
	/** Brood: frações de vida que disparam a invocação. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly) TArray<float> BroodHealthThresholds = { 0.6f, 0.3f };
	UPROPERTY(EditAnywhere, BlueprintReadOnly) int32 BroodSummonCount = 5;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (Units = "cm")) float BroodRing = 225.f;
};

/**
 * Definição de um inimigo (bestiary.ts MonsterDefinition / enemies.ts). Um asset por monstro.
 * O GameMode spawna ADSEnemyCharacter e chama InitializeFromData(Data, Level, HpMult, DmgMult, bElite).
 */
UCLASS(BlueprintType, Const)
class DUNGEONSTANCE_API UDSEnemyData : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	virtual FPrimaryAssetId GetPrimaryAssetId() const override;

	/** Stats no nível dado (fórmulas §8.1 × arquétipo × elite). */
	UFUNCTION(BlueprintPure, Category = "DS|Enemy")
	FDSEnemyStats ComputeStats(int32 Level, bool bElite) const;

#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Enemy") FName EnemyId;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Enemy") FText DisplayName;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Enemy", meta = (ClampMin = 1)) int32 BaseLevel = 1;
	/** Ao trocar, o editor preenche ArchetypeStats com os defaults do protótipo. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Enemy") EDSEnemyArchetype Archetype = EDSEnemyArchetype::Minion;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Enemy") FDSEnemyArchetypeStats ArchetypeStats;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Enemy") EDSEnemyBehavior Behavior = EDSEnemyBehavior::Melee;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Enemy") bool bIsBoss = false;
	/** Ajuste de velocidade do monstro sobre o arquétipo (speed_mult). */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Enemy") float SpeedMultiplier = 1.f;
	/** Tamanho do bando ao spawnar (enxames entram juntos). */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Enemy") int32 PackSize = 1;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Elite") float EliteHealthMultiplier = 2.5f;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Elite") float EliteDpsMultiplier = 1.5f;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Elite", meta = (Units = "cm")) float EliteRadiusBonus = 10.f;
	/** Dano do projétil = fator × DPS base do nível. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat") float ProjectileDpsFactor = 2.2f;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat") TSubclassOf<ADSProjectile> ProjectileClass;
	/** Habilidades extras do inimigo (opcional). */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat") TObjectPtr<UDSAbilitySet> AbilitySet;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AI") FDSEnemyAITuning AITuning;
	/** Brood: o que invocar. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AI") TObjectPtr<UDSEnemyData> BroodMinion;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Visual") TSoftObjectPtr<USkeletalMesh> Mesh;
};
