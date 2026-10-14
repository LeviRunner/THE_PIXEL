// Testes de automação das fórmulas puras de DSRunRules.
// Valores esperados copiados dos testes do protótipo (Nakama_Legacy_Reference/test)
// quando existem; os demais vêm da ESPEC_REGRAS_DUNGEON_STANCE.md.
// Rodar: Session Frontend → Automation → filtro "DungeonStance.Core.RunRules".

#include "Misc/AutomationTest.h"
#include "Core/DSRunRules.h"

#if WITH_DEV_AUTOMATION_TESTS

namespace DSRunRulesTestUtils
{
	constexpr EAutomationTestFlags TestFlags = EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter;
	constexpr float Tolerance = 1.e-4f;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FDSRunRulesBossWaveTest, "DungeonStance.Core.RunRules.BossWave", DSRunRulesTestUtils::TestFlags)
bool FDSRunRulesBossWaveTest::RunTest(const FString& Parameters)
{
	// squad_logic.test.ts: chefes a cada 5ª onda e na última.
	TestTrue(TEXT("Onda 5 é de chefe"), DSRunRules::IsBossWave(5, 10));
	TestTrue(TEXT("Onda 10 é de chefe"), DSRunRules::IsBossWave(10, 10));
	TestFalse(TEXT("Onda 4 não é de chefe"), DSRunRules::IsBossWave(4, 10));
	TestFalse(TEXT("Onda 0 não é de chefe"), DSRunRules::IsBossWave(0, 10));
	TestTrue(TEXT("Última onda fora do intervalo também é de chefe"), DSRunRules::IsBossWave(7, 7));

	int32 BossCount = 0;
	for (int32 Wave = 1; Wave <= DSConstants::MaxWaves; ++Wave)
	{
		BossCount += DSRunRules::IsBossWave(Wave, DSConstants::MaxWaves) ? 1 : 0;
	}
	TestEqual(TEXT("Exatamente 2 chefes em 10 ondas"), BossCount, 2);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FDSRunRulesEnemyCountTest, "DungeonStance.Core.RunRules.EnemyCount", DSRunRulesTestUtils::TestFlags)
bool FDSRunRulesEnemyCountTest::RunTest(const FString& Parameters)
{
	TestEqual(TEXT("Onda 1: 9 regulares"), DSRunRules::GetRegularEnemyCount(1), 9);
	TestEqual(TEXT("Onda 10: 36 regulares"), DSRunRules::GetRegularEnemyCount(10), 36);

	// Com 4 jogadores a escala é ×1,0: igual ao protótipo (regulares + chefe).
	TestEqual(TEXT("Onda 2, party de 4"), DSRunRules::GetScaledWaveEnemyCount(2, 10, 4), 12);
	TestEqual(TEXT("Onda 5, party de 4 (+chefe)"), DSRunRules::GetScaledWaveEnemyCount(5, 10, 4), 22);
	TestEqual(TEXT("Onda 1, solo: round(9 × 0,55)"), DSRunRules::GetScaledWaveEnemyCount(1, 10, 1), 5);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FDSRunRulesWaveScalingTest, "DungeonStance.Core.RunRules.WaveScaling", DSRunRulesTestUtils::TestFlags)
bool FDSRunRulesWaveScalingTest::RunTest(const FString& Parameters)
{
	using namespace DSRunRulesTestUtils;
	TestEqual(TEXT("Vida onda 1, rank 1"), DSRunRules::GetWaveHealthMultiplier(1, 1.f), 1.f, Tolerance);
	TestEqual(TEXT("Vida onda 3, rank 10"), DSRunRules::GetWaveHealthMultiplier(3, 10.f), 1.24f * 1.36f, Tolerance);
	TestEqual(TEXT("Dano onda 1, rank 1"), DSRunRules::GetWaveDamageMultiplier(1, 1.f), 1.f, Tolerance);
	TestEqual(TEXT("Dano onda 3, rank 10"), DSRunRules::GetWaveDamageMultiplier(3, 10.f), 1.12f * 1.18f, Tolerance);
	TestEqual(TEXT("Rank abaixo de 1 não reduz"), DSRunRules::GetWaveHealthMultiplier(1, 0.f), 1.f, Tolerance);

	// squad_logic.test.ts: vida escala com o rank da party.
	TestTrue(TEXT("Rank 30 > rank 1"), DSRunRules::GetWaveHealthMultiplier(1, 30.f) > DSRunRules::GetWaveHealthMultiplier(1, 1.f));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FDSRunRulesPartyScalingTest, "DungeonStance.Core.RunRules.PartyScaling", DSRunRulesTestUtils::TestFlags)
bool FDSRunRulesPartyScalingTest::RunTest(const FString& Parameters)
{
	using namespace DSRunRulesTestUtils;
	TestEqual(TEXT("Quantidade N=1"), DSRunRules::GetPartyEnemyCountMultiplier(1), 0.55f, Tolerance);
	TestEqual(TEXT("Quantidade N=4 (protótipo)"), DSRunRules::GetPartyEnemyCountMultiplier(4), 1.f, Tolerance);
	TestEqual(TEXT("Quantidade N=5"), DSRunRules::GetPartyEnemyCountMultiplier(5), 1.15f, Tolerance);
	TestEqual(TEXT("Quantidade N=9 limitado a 5"), DSRunRules::GetPartyEnemyCountMultiplier(9), 1.15f, Tolerance);
	TestEqual(TEXT("Quantidade N=0 limitado a 1"), DSRunRules::GetPartyEnemyCountMultiplier(0), 0.55f, Tolerance);

	TestEqual(TEXT("Chefe N=1"), DSRunRules::GetPartyBossHealthMultiplier(1), 0.625f, Tolerance);
	TestEqual(TEXT("Chefe N=4 (protótipo)"), DSRunRules::GetPartyBossHealthMultiplier(4), 1.f, Tolerance);
	TestEqual(TEXT("Chefe N=5"), DSRunRules::GetPartyBossHealthMultiplier(5), 1.125f, Tolerance);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FDSRunRulesTeamXPTest, "DungeonStance.Core.RunRules.TeamXP", DSRunRulesTestUtils::TestFlags)
bool FDSRunRulesTeamXPTest::RunTest(const FString& Parameters)
{
	TestEqual(TEXT("Nível 1 → 65"), DSRunRules::GetTeamXPToNext(1), 65);
	TestEqual(TEXT("Nível 2 → 90"), DSRunRules::GetTeamXPToNext(2), 90);

	// squad_logic.test.ts: sobe vários níveis e guarda o resto.
	const int32 Amount = DSRunRules::GetTeamXPToNext(1) + DSRunRules::GetTeamXPToNext(2) + 3;
	const DSRunRules::FTeamXPResult Multi = DSRunRules::AddTeamXP(0, 1, Amount);
	TestEqual(TEXT("Resto de XP"), Multi.XP, 3);
	TestEqual(TEXT("Nível final"), Multi.Level, 3);
	TestEqual(TEXT("Níveis ganhos"), Multi.LevelsGained, 2);

	// squad_logic.test.ts: ignora valores negativos.
	const DSRunRules::FTeamXPResult Negative = DSRunRules::AddTeamXP(5, 1, -10);
	TestEqual(TEXT("XP negativo ignorado"), Negative.XP, 5);
	TestEqual(TEXT("Nível mantido"), Negative.Level, 1);
	TestEqual(TEXT("Nenhum nível ganho"), Negative.LevelsGained, 0);

	TestEqual(TEXT("Escolhas pendentes nível 3, 0 usadas"), DSRunRules::GetPendingPicks(3, 0), 2);
	TestEqual(TEXT("Escolhas pendentes nunca negativas"), DSRunRules::GetPendingPicks(1, 4), 0);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FDSRunRulesRankXPTest, "DungeonStance.Core.RunRules.RankXP", DSRunRulesTestUtils::TestFlags)
bool FDSRunRulesRankXPTest::RunTest(const FString& Parameters)
{
	using namespace DSRunRulesTestUtils;
	// Valores calculados com a fórmula do progression.ts (Math.round(250 × rank^1,2)).
	TestEqual(TEXT("Rank 1"), DSRunRules::GetRankXPToNext(1), 250);
	TestEqual(TEXT("Rank 2"), DSRunRules::GetRankXPToNext(2), 574);
	TestEqual(TEXT("Rank 3"), DSRunRules::GetRankXPToNext(3), 934);
	TestEqual(TEXT("Rank 10"), DSRunRules::GetRankXPToNext(10), 3962);
	TestEqual(TEXT("Rank máximo → 0"), DSRunRules::GetRankXPToNext(DSConstants::MaxClassRank), 0);

	// progression.test.ts: cada rank exige mais XP que o anterior.
	bool bMonotonic = true;
	for (int32 Rank = 1; Rank < DSConstants::MaxClassRank - 1; ++Rank)
	{
		bMonotonic &= DSRunRules::GetRankXPToNext(Rank + 1) > DSRunRules::GetRankXPToNext(Rank);
	}
	TestTrue(TEXT("Curva de rank crescente"), bMonotonic);

	// progression.test.ts: multiplicadores derivados do rank.
	TestEqual(TEXT("Vida rank 1"), DSRunRules::GetRankHealthMultiplier(1), 1.f, Tolerance);
	TestEqual(TEXT("Dano rank 1"), DSRunRules::GetRankDamageMultiplier(1), 1.f, Tolerance);
	TestEqual(TEXT("Vida rank 14"), DSRunRules::GetRankHealthMultiplier(14), 1.13f, Tolerance);
	TestEqual(TEXT("Dano rank 14"), DSRunRules::GetRankDamageMultiplier(14), 1.078f, Tolerance);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FDSRunRulesEndOfRunXPTest, "DungeonStance.Core.RunRules.EndOfRunXP", DSRunRulesTestUtils::TestFlags)
bool FDSRunRulesEndOfRunXPTest::RunTest(const FString& Parameters)
{
	// squad_logic.test.ts computeRewardXp.
	TestEqual(TEXT("Só participação"), DSRunRules::GetEndOfRunRankXP(0, false, 0), 100);
	TestEqual(TEXT("10 ondas, vitória, 10 kills"), DSRunRules::GetEndOfRunRankXP(10, true, 10), 100 + 600 + 300 + 50);
	TestEqual(TEXT("Kills limitados a 250 XP"), DSRunRules::GetEndOfRunRankXP(0, false, 10000), 100 + 250);
	TestEqual(TEXT("Valores negativos ignorados"), DSRunRules::GetEndOfRunRankXP(-3, false, -5), 100);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FDSRunRulesSpawnOffsetTest, "DungeonStance.Core.RunRules.SpawnOffset", DSRunRulesTestUtils::TestFlags)
bool FDSRunRulesSpawnOffsetTest::RunTest(const FString& Parameters)
{
	constexpr float Radius = 150.f;
	constexpr float PositionTolerance = 0.01f;
	TestTrue(TEXT("Índice 0 em +X"), DSRunRules::GetSpawnOffset(0, 4, Radius).Equals(FVector(150.f, 0.f, 0.f), PositionTolerance));
	TestTrue(TEXT("Índice 1 de 4 em +Y"), DSRunRules::GetSpawnOffset(1, 4, Radius).Equals(FVector(0.f, 150.f, 0.f), PositionTolerance));
	TestTrue(TEXT("Índice 2 de 4 em -X"), DSRunRules::GetSpawnOffset(2, 4, Radius).Equals(FVector(-150.f, 0.f, 0.f), PositionTolerance));

	const FVector FiveA = DSRunRules::GetSpawnOffset(1, 5, Radius);
	const FVector Expected(FMath::Cos(UE_TWO_PI / 5.f) * Radius, FMath::Sin(UE_TWO_PI / 5.f) * Radius, 0.f);
	TestTrue(TEXT("Party de 5 usa 72°"), FiveA.Equals(Expected, PositionTolerance));

	for (int32 Index = 0; Index < DSConstants::MaxPartySize; ++Index)
	{
		const float Distance = DSRunRules::GetSpawnOffset(Index, DSConstants::MaxPartySize, Radius).Size();
		TestEqual(FString::Printf(TEXT("Índice %d no raio"), Index), Distance, Radius, PositionTolerance);
	}
	TestTrue(TEXT("Solo nasce no anel"), DSRunRules::GetSpawnOffset(0, 1, Radius).Equals(FVector(150.f, 0.f, 0.f), PositionTolerance));
	TestTrue(TEXT("N=0 não divide por zero"), !DSRunRules::GetSpawnOffset(0, 0, Radius).ContainsNaN());
	TestEqual(TEXT("Raio padrão = 60 px"), DSRunRules::HeroSpawnRadiusCm, 150.f);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FDSRunRulesArenaLevelTest, "DungeonStance.Core.RunRules.ArenaLevel", DSRunRulesTestUtils::TestFlags)
bool FDSRunRulesArenaLevelTest::RunTest(const FString& Parameters)
{
	// bestiary.test.ts arenaLevel.
	TestEqual(TEXT("Clareira"), DSRunRules::GetArenaEffectiveLevel(0, 0), 3);
	TestEqual(TEXT("Galerias"), DSRunRules::GetArenaEffectiveLevel(1, 0), 7);
	TestEqual(TEXT("Cemitério"), DSRunRules::GetArenaEffectiveLevel(2, 0), 11);
	TestEqual(TEXT("Galerias Tormento 2"), DSRunRules::GetArenaEffectiveLevel(1, 2), 11);
	TestEqual(TEXT("Tormento negativo ignorado"), DSRunRules::GetArenaEffectiveLevel(0, -4), 3);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FDSRunRulesGoldTest, "DungeonStance.Core.RunRules.Gold", DSRunRulesTestUtils::TestFlags)
bool FDSRunRulesGoldTest::RunTest(const FString& Parameters)
{
	// espolio.test.ts waveGold(10, 20).
	TestEqual(TEXT("Onda 10, 20 kills"), DSRunRules::GetWaveGold(10, 20), 4 + 10 * 2 + 20);
	TestEqual(TEXT("Onda 1, sem kills"), DSRunRules::GetWaveGold(1, 0), 6);
	TestEqual(TEXT("Kills negativos ignorados"), DSRunRules::GetWaveGold(1, -7), 6);
	TestEqual(TEXT("Bônus de vitória"), DSRunRules::GoldVictoryBonus, 60);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FDSRunRulesEnemyFormulasTest, "DungeonStance.Core.RunRules.EnemyFormulas", DSRunRulesTestUtils::TestFlags)
bool FDSRunRulesEnemyFormulasTest::RunTest(const FString& Parameters)
{
	using namespace DSRunRulesTestUtils;
	// bestiary.ts §8.1 com L = 3: vida 50 + 48 + 4,5 = 102,5.
	TestEqual(TEXT("Vida base L3"), DSRunRules::GetEnemyBaseHealth(3), 102.5f, Tolerance);
	TestEqual(TEXT("DPS base L3"), DSRunRules::GetEnemyBaseDPS(3), 25.5f, Tolerance);
	TestEqual(TEXT("Ouro médio L3"), DSRunRules::GetEnemyAverageGold(3), 7);
	TestEqual(TEXT("XP de combate vida 103"), DSRunRules::GetEnemyCombatXP(103.f), 9);
	TestEqual(TEXT("XP de combate vida 0"), DSRunRules::GetEnemyCombatXP(0.f), 4);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FDSRunRulesSeedTest, "DungeonStance.Core.RunRules.Seed", DSRunRulesTestUtils::TestFlags)
bool FDSRunRulesSeedTest::RunTest(const FString& Parameters)
{
	TestEqual(TEXT("Semente da onda"), DSRunRules::GetWaveSeed(1000, 2), 1000 + 2 * 7919);
	TestTrue(TEXT("Sem estouro com semente máxima"), DSRunRules::GetWaveSeed(MAX_int32, 10) >= 0);

	// Mesma semente → mesma sequência (run determinística).
	FRandomStream A(DSRunRules::GetWaveSeed(42, 3));
	FRandomStream B(DSRunRules::GetWaveSeed(42, 3));
	bool bSame = true;
	for (int32 Index = 0; Index < 16; ++Index)
	{
		bSame &= A.RandRange(0, 1000) == B.RandRange(0, 1000);
	}
	TestTrue(TEXT("Sequência determinística"), bSame);
	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
