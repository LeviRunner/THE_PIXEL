#pragma once

#include "CoreMinimal.h"
#include "Core/DSTypes.h"

/**
 * Fórmulas puras da run do Dungeon Stance, portadas do protótipo
 * (Nakama_Legacy_Reference/src: squad_logic.ts, squad_match.ts, squad_state.ts,
 * progression.ts, loot.ts, bestiary.ts). Sem UObject, sem rede: tudo testável
 * em DSRunRulesTests.cpp.
 *
 * Unidades: tempo em segundos (o protótipo usava ticks a 10 Hz) e distância em
 * cm (1 px do protótipo = 2,5 cm).
 */
namespace DSRunRules
{
	// ---------------------------------------------------------------------
	// Conversões
	// ---------------------------------------------------------------------

	/** 40 px = 1 m no protótipo → 1 px = 2,5 cm. */
	inline constexpr float CmPerPrototypePixel = 2.5f;

	// ---------------------------------------------------------------------
	// Tempos do ciclo da partida (squad_match.ts), em segundos
	// ---------------------------------------------------------------------

	inline constexpr float JoinTimeoutSec = 30.f;
	inline constexpr float WaveBreakSec = 4.f;
	inline constexpr float CardPickSec = 20.f;
	inline constexpr float CardPickNetworkMarginSec = 2.f;
	inline constexpr float PickBreakSec = CardPickSec + CardPickNetworkMarginSec;
	inline constexpr float AllPickedGraceSec = 1.f;
	inline constexpr float WaveTimeoutSec = 150.f;
	inline constexpr float EndScreenSec = 10.f;
	inline constexpr float EmptyMatchTimeoutSec = 15.f;
	/** Jogador sem atualizar estado por esse tempo conta como inativo (squad_state.ts). */
	inline constexpr float StalePlayerSec = 5.f;

	// ---------------------------------------------------------------------
	// Spawn (squad_state.ts / level.ts)
	// ---------------------------------------------------------------------

	/** Raio do anel de spawn dos heróis: 60 px = 150 cm. */
	inline constexpr float HeroSpawnRadiusCm = 60.f * CmPerPrototypePixel;
	/** Inimigos nascem entre 18 e 30 m do centro da party. */
	inline constexpr float EnemySpawnRingMinCm = 1800.f;
	inline constexpr float EnemySpawnRingMaxCm = 3000.f;
	/** Distância mínima entre um inimigo recém-nascido e um herói (12 m). */
	inline constexpr float EnemySpawnPlayerClearanceCm = 1200.f;
	inline constexpr int32 EnemySpawnAttempts = 32;

	/** Semente por onda: seed + onda × 7919 (squad_match.ts startNextWave). */
	inline constexpr int32 WaveSeedPrime = 7919;

	// ---------------------------------------------------------------------
	// Ondas e escala
	// ---------------------------------------------------------------------

	inline constexpr int32 BaseRegularEnemies = 6;
	inline constexpr int32 RegularEnemiesPerWave = 3;
	inline constexpr float HealthPerWave = 0.12f;
	inline constexpr float HealthPerRank = 0.04f;
	inline constexpr float DamagePerWave = 0.06f;
	inline constexpr float DamagePerRank = 0.02f;

	/** Escala por tamanho da party (proposta da ESPEC §1; N = 4 → ×1,0). */
	inline constexpr float PartyEnemyCountBase = 0.4f;
	inline constexpr float PartyEnemyCountPerPlayer = 0.15f;
	inline constexpr float PartyBossHealthBase = 0.5f;
	inline constexpr float PartyBossHealthPerPlayer = 0.125f;

	// ---------------------------------------------------------------------
	// XP do time (squad_logic.ts) e rank da classe (progression.ts)
	// ---------------------------------------------------------------------

	inline constexpr int32 TeamXPBase = 40;
	inline constexpr int32 TeamXPPerLevel = 25;

	inline constexpr float RankXPBase = 250.f;
	inline constexpr float RankXPExponent = 1.2f;
	inline constexpr float RankHealthBonusPerRank = 0.01f;
	inline constexpr float RankDamageBonusPerRank = 0.006f;

	inline constexpr int32 RunXPParticipation = 100;
	inline constexpr int32 RunXPPerWave = 60;
	inline constexpr int32 RunXPVictoryBonus = 300;
	inline constexpr int32 RunXPPerKill = 5;
	inline constexpr int32 RunXPKillCap = 250;

	// ---------------------------------------------------------------------
	// Arenas, inimigos e ouro (bestiary.ts, loot.ts)
	// ---------------------------------------------------------------------

	inline constexpr int32 ArenaBaseLevel = 3;
	inline constexpr int32 ArenaLevelPerIndex = 4;
	inline constexpr int32 ArenaLevelPerTorment = 2;

	inline constexpr int32 GoldPerWaveBase = 4;
	inline constexpr int32 GoldPerWaveStep = 2;
	inline constexpr int32 GoldPerKill = 1;
	inline constexpr int32 GoldVictoryBonus = 60;

	// ---------------------------------------------------------------------
	// Funções do contrato (CONTRATOS.md)
	// ---------------------------------------------------------------------

	/** Onda de chefe: múltiplo do intervalo de chefe ou a última onda. */
	DUNGEONSTANCE_API bool IsBossWave(int32 Wave, int32 MaxWaves = DSConstants::MaxWaves);

	/** Inimigos regulares da onda (sem o chefe): 6 + 3 × onda. */
	DUNGEONSTANCE_API int32 GetRegularEnemyCount(int32 Wave);

	/** Vida: (1 + 0,12 × (onda − 1)) × (1 + 0,04 × max(0, rankMédio − 1)). */
	DUNGEONSTANCE_API float GetWaveHealthMultiplier(int32 Wave, float AvgRank);

	/** Dano: (1 + 0,06 × (onda − 1)) × (1 + 0,02 × max(0, rankMédio − 1)). */
	DUNGEONSTANCE_API float GetWaveDamageMultiplier(int32 Wave, float AvgRank);

	/** Quantidade de inimigos × (0,4 + 0,15 × N), N limitado a 1..5. */
	DUNGEONSTANCE_API float GetPartyEnemyCountMultiplier(int32 NumPlayers);

	/** Vida do chefe × (0,5 + 0,125 × N), N limitado a 1..5. */
	DUNGEONSTANCE_API float GetPartyBossHealthMultiplier(int32 NumPlayers);

	/** XP para o próximo nível do time: 40 + 25 × nível. */
	DUNGEONSTANCE_API int32 GetTeamXPToNext(int32 Level);

	/** XP para o próximo rank da classe: round(250 × rank^1,2); 0 no rank máximo. */
	DUNGEONSTANCE_API int32 GetRankXPToNext(int32 Rank);

	/** XP de rank no fim da run: 100 + 60 × ondas + (vitória ? 300 : 0) + min(250, 5 × kills). */
	DUNGEONSTANCE_API int32 GetEndOfRunRankXP(int32 WavesCleared, bool bVictory, int32 Kills);

	/** Posição do herói Index no anel de N jogadores (ângulo Index/N × 2π), em cm, Z = 0. */
	DUNGEONSTANCE_API FVector GetSpawnOffset(int32 Index, int32 NumPlayers, float RadiusCm = HeroSpawnRadiusCm);

	/** Nível efetivo da arena: 3 + 4 × índice + 2 × Tormento. */
	DUNGEONSTANCE_API int32 GetArenaEffectiveLevel(int32 ArenaIndex, int32 Torment);

	/** Ouro da onda limpa: 4 + 2 × onda + kills na onda. */
	DUNGEONSTANCE_API int32 GetWaveGold(int32 Wave, int32 Kills);

	// ---------------------------------------------------------------------
	// Auxiliares (também portadas do protótipo)
	// ---------------------------------------------------------------------

	/** Total de inimigos da onda já escalado pela party (+1 chefe em onda de chefe). */
	DUNGEONSTANCE_API int32 GetScaledWaveEnemyCount(int32 Wave, int32 MaxWaves, int32 NumPlayers);

	/** Resultado de AddTeamXP (squad_logic.ts addTeamXp). */
	struct FTeamXPResult
	{
		int32 XP = 0;
		int32 Level = 1;
		int32 LevelsGained = 0;
	};

	/** Soma XP ao time, subindo quantos níveis couberem e guardando o resto. Ignora valores negativos. */
	DUNGEONSTANCE_API FTeamXPResult AddTeamXP(int32 CurrentXP, int32 CurrentLevel, int32 Amount);

	/** Escolhas de carta ganhas e não usadas: nívelTime − 1 − escolhasUsadas (mín. 0). */
	DUNGEONSTANCE_API int32 GetPendingPicks(int32 TeamLevel, int32 PicksUsed);

	/** Bônus permanente do rank: +1% vida por rank acima do 1 (3 casas decimais). */
	DUNGEONSTANCE_API float GetRankHealthMultiplier(int32 Rank);

	/** Bônus permanente do rank: +0,6% dano por rank acima do 1 (3 casas decimais). */
	DUNGEONSTANCE_API float GetRankDamageMultiplier(int32 Rank);

	/** XP de combate de um inimigo pela vida base (sem escala de onda): round(4 + 0,05 × vida). */
	DUNGEONSTANCE_API int32 GetEnemyCombatXP(float BaseHealth);

	/** Vida do minion de nível L: 50 + 16L + 0,5L². */
	DUNGEONSTANCE_API float GetEnemyBaseHealth(int32 Level);

	/** DPS do minion de nível L: 12 + 4,5L. */
	DUNGEONSTANCE_API float GetEnemyBaseDPS(int32 Level);

	/** Ouro médio do inimigo de nível L: round(2 + 1,6L + 0,05L²). */
	DUNGEONSTANCE_API int32 GetEnemyAverageGold(int32 Level);

	/** Semente determinística de uma onda a partir da semente da run. */
	DUNGEONSTANCE_API int32 GetWaveSeed(int32 RunSeed, int32 Wave);

	/** Limita o número de jogadores ao intervalo válido da party (1..5). */
	DUNGEONSTANCE_API int32 ClampPartySize(int32 NumPlayers);
}
