#pragma once

#include "CoreMinimal.h"
#include "DSTypes.generated.h"

// Tipos compartilhados por Core, Gameplay, Network e UI.
// Arquivo da base do projeto: mudanças aqui afetam as 4 áreas (ver CONTRATOS.md).

namespace DSConstants
{
	inline constexpr int32 MinPartySize = 1;
	inline constexpr int32 MaxPartySize = 5;
	inline constexpr int32 MaxWaves = 10;
	inline constexpr int32 BossWaveInterval = 5;
	inline constexpr int32 MaxClassRank = 60;
	inline constexpr int32 MaxSupportAbilitySlots = 4;
}

/** As 9 classes jogáveis (classes.ts / CLASSES.md). */
UENUM(BlueprintType)
enum class EDSHeroClass : uint8
{
	None,
	Barbarian		UMETA(DisplayName = "Bárbaro"),
	Crusader		UMETA(DisplayName = "Cruzado"),
	DemonHunter		UMETA(DisplayName = "Caçador de Demônios"),
	Monk			UMETA(DisplayName = "Monge"),
	Necromancer		UMETA(DisplayName = "Necromante"),
	Arcanist		UMETA(DisplayName = "Arcanista"),
	BloodKnight		UMETA(DisplayName = "Cavaleiro Sangrento"),
	Wizard			UMETA(DisplayName = "Mago"),
	Archer			UMETA(DisplayName = "Arqueiro")
};

/** Stance escolhida antes da run = árvore de cartas (cards.ts). */
UENUM(BlueprintType)
enum class EDSStance : uint8
{
	None,
	Critical	UMETA(DisplayName = "Crítico"),
	Arcane		UMETA(DisplayName = "Arcana"),
	Defense		UMETA(DisplayName = "Defesa"),
	Support		UMETA(DisplayName = "Apoio")
};

/** Fase da partida (squad_match.ts). */
UENUM(BlueprintType)
enum class EDSMatchPhase : uint8
{
	WaitingForPlayers,
	WaveInProgress,
	BetweenWaves,
	CardPick,
	Victory,
	Defeat,
	Ended
};

/** Tamanho de party exibido no lobby. O valor numérico é o número de jogadores. */
UENUM(BlueprintType)
enum class EDSPartySize : uint8
{
	Solo = 1	UMETA(DisplayName = "Solo"),
	Duo = 2		UMETA(DisplayName = "Dupla"),
	Trio = 3	UMETA(DisplayName = "Trio"),
	Squad = 4	UMETA(DisplayName = "Esquadrão de 4"),
	Group = 5	UMETA(DisplayName = "Grupo de 5")
};

/** Tipo de ataque básico (classes.ts). */
UENUM(BlueprintType)
enum class EDSAttackType : uint8
{
	Melee,
	MeleeAoE,
	Projectile,
	ProjectileAoE
};

/** Carta possuída por um jogador na run atual. */
USTRUCT(BlueprintType)
struct FDSOwnedCard
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FName CardId;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int32 Stacks = 0;
};

/** Resumo de fim de run enviado ao backend (progresso permanente). */
USTRUCT(BlueprintType)
struct FDSRunResult
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly)
	FString PlayerId;

	UPROPERTY(BlueprintReadOnly)
	EDSHeroClass HeroClass = EDSHeroClass::None;

	UPROPERTY(BlueprintReadOnly)
	int32 WavesCleared = 0;

	UPROPERTY(BlueprintReadOnly)
	int32 Kills = 0;

	UPROPERTY(BlueprintReadOnly)
	bool bVictory = false;

	UPROPERTY(BlueprintReadOnly)
	int32 RankXPEarned = 0;

	UPROPERTY(BlueprintReadOnly)
	int32 GoldEarned = 0;
};
