// Testes de automação das fórmulas de dano, status e bestiário (Combat/DSDamageMath.h, Combat/DSEnemyRules.h).
// Referências: BasePlayer.gd (mitigação do herói), PLANO.md §8 (monstro), status.ts, bestiary.test.ts.
// Rodar: Session Frontend → Automation → filtro "DungeonStance.Gameplay.Damage".

#include "Misc/AutomationTest.h"
#include "Gameplay/Combat/DSDamageMath.h"
#include "Gameplay/Combat/DSEnemyRules.h"

#if WITH_DEV_AUTOMATION_TESTS

namespace DSDamageTestUtils
{
	constexpr EAutomationTestFlags TestFlags = EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter;
	constexpr float Tolerance = 1.e-3f;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FDSDamageArmorTest, "DungeonStance.Gameplay.Damage.Armor", DSDamageTestUtils::TestFlags)
bool FDSDamageArmorTest::RunTest(const FString& Parameters)
{
	using namespace DSDamageMath;
	const float Tol = DSDamageTestUtils::Tolerance;
	TestNearlyEqual(TEXT("Sem armadura"), ArmorMitigation(0.f, HeroArmorConstant), 1.f, Tol);
	TestNearlyEqual(TEXT("Herói 100 de armadura = metade"), ArmorMitigation(100.f, HeroArmorConstant), 0.5f, Tol);
	TestNearlyEqual(TEXT("Bárbaro (30)"), ArmorMitigation(30.f, HeroArmorConstant), 100.f / 130.f, Tol);
	TestNearlyEqual(TEXT("Monstro 200 de armadura = metade"), ArmorMitigation(200.f, MonsterArmorConstant), 0.5f, Tol);
	TestNearlyEqual(TEXT("Armadura negativa vira 0"), ArmorMitigation(-50.f, HeroArmorConstant), 1.f, Tol);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FDSDamageFormulaTest, "DungeonStance.Gameplay.Damage.Formula", DSDamageTestUtils::TestFlags)
bool FDSDamageFormulaTest::RunTest(const FString& Parameters)
{
	using namespace DSDamageMath;
	const float Tol = DSDamageTestUtils::Tolerance;

	FDamageParams Base;
	Base.BaseDamage = 100.f;
	Base.ArmorConstant = MonsterArmorConstant;
	TestNearlyEqual(TEXT("Sem modificadores"), ComputeDamage(Base), 100.f, Tol);

	FDamageParams Crit = Base;
	Crit.bCrit = true;
	Crit.CritMultiplier = 1.5f;
	TestNearlyEqual(TEXT("Crítico ×1,5"), ComputeDamage(Crit), 150.f, Tol);

	// Marca e Exposto multiplicam entre si (status.ts incomingDamageMultiplier).
	FDamageParams Marked = Base;
	Marked.MarkVulnerability = 0.06f;
	Marked.ExposedVulnerability = 0.1f;
	TestNearlyEqual(TEXT("Marca × Exposto"), ComputeDamage(Marked), 100.f * 1.06f * 1.1f, Tol);

	FDamageParams Weakened = Base;
	Weakened.WeakenPct = 0.25f;
	TestNearlyEqual(TEXT("Enfraquecido −25%"), ComputeDamage(Weakened), 75.f, Tol);

	// Minion nível 5 (armadura 2L = 10) recebendo golpe de 38 do Bárbaro, crítico, marcado.
	FDamageParams Full;
	Full.BaseDamage = 38.f;
	Full.bCrit = true;
	Full.CritMultiplier = 1.5f;
	Full.MarkVulnerability = 0.06f;
	Full.TargetArmor = DSEnemyRules::BaseArmor(5);
	Full.ArmorConstant = MonsterArmorConstant;
	TestNearlyEqual(TEXT("Cadeia completa"), ComputeDamage(Full), 38.f * 1.5f * 1.06f * (200.f / 210.f), Tol);

	FDamageParams Reduced = Base;
	Reduced.ArmorConstant = HeroArmorConstant;
	Reduced.TargetArmor = 100.f;
	Reduced.TargetDamageReduction = 0.3f; // Santuário
	TestNearlyEqual(TEXT("Armadura + redução de dano"), ComputeDamage(Reduced), 100.f * 0.5f * 0.7f, Tol);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FDSDamageStatusRulesTest, "DungeonStance.Gameplay.Damage.StatusRules", DSDamageTestUtils::TestFlags)
bool FDSDamageStatusRulesTest::RunTest(const FString& Parameters)
{
	using namespace DSDamageMath;
	const float Tol = DSDamageTestUtils::Tolerance;
	TestNearlyEqual(TEXT("Lentidão comum"), EffectiveSlow(0.3f, false), 0.3f, Tol);
	TestNearlyEqual(TEXT("Chefe recebe metade"), EffectiveSlow(0.3f, true), 0.15f, Tol);
	TestNearlyEqual(TEXT("Teto de 70%"), EffectiveSlow(0.95f, false), MaxSlow, Tol);
	// Atordoamento em chefe vira lentidão de 30% (o fator de chefe é desfeito antes).
	TestNearlyEqual(TEXT("Stun em chefe = 30%"), EffectiveSlow(BossStunAsSlow / BossSlowFactor, true), 0.3f, Tol);
	TestNearlyEqual(TEXT("Fúria cheia +30%"), ResourceDamageBonus(100.f, 100.f, 0.3f), 1.3f, Tol);
	TestNearlyEqual(TEXT("Fúria pela metade +15%"), ResourceDamageBonus(50.f, 100.f, 0.3f), 1.15f, Tol);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FDSDamageBestiaryTest, "DungeonStance.Gameplay.Damage.Bestiary", DSDamageTestUtils::TestFlags)
bool FDSDamageBestiaryTest::RunTest(const FString& Parameters)
{
	// bestiary.ts §8.1: L1 = 66,5 de vida; L15 = 402,5 (tabela §8.3 arredonda para 403).
	TestNearlyEqual(TEXT("Vida base L1"), DSEnemyRules::BaseHealth(1), 66.5f, 1.e-3f);
	TestNearlyEqual(TEXT("DPS base L1"), DSEnemyRules::BaseDps(1), 16.5f, 1.e-3f);
	TestEqual(TEXT("XP de combate (vida 403)"), DSEnemyRules::CombatXP(403.f), 24);

	// Chefe nível 15: round(403) × 30 = 12 090 (comentário de archetypeStats).
	const FDSEnemyArchetypeStats Boss = DSEnemyRules::GetArchetypeDefaults(EDSEnemyArchetype::Boss);
	const FDSEnemyStats BossStats = DSEnemyRules::ComputeStats(Boss, 15, false, 2.5f, 1.5f, 10.f, 2.2f, 1.f);
	TestNearlyEqual(TEXT("Chefe L15"), BossStats.MaxHealth, 12090.f, 1.e-3f);

	// Elite: ×2,5 vida, ×1,5 DPS; projétil = 2,2 × DPS do nível (só à distância).
	const FDSEnemyArchetypeStats Shooter = DSEnemyRules::GetArchetypeDefaults(EDSEnemyArchetype::Shooter);
	const FDSEnemyStats Normal = DSEnemyRules::ComputeStats(Shooter, 4, false, 2.5f, 1.5f, 10.f, 2.2f, 1.f);
	const FDSEnemyStats Elite = DSEnemyRules::ComputeStats(Shooter, 4, true, 2.5f, 1.5f, 10.f, 2.2f, 1.f);
	TestTrue(TEXT("Atirador tem projétil"), Normal.ProjectileDamage > 0.f);
	TestNearlyEqual(TEXT("Projétil = round(30 × 2,2)"), Normal.ProjectileDamage, FMath::RoundToFloat(30.f * 2.2f), 1.e-3f);
	TestTrue(TEXT("Elite tem mais vida"), Elite.MaxHealth > Normal.MaxHealth * 2.4f);
	TestNearlyEqual(TEXT("Alcance do atirador (340 px)"), Normal.AttackRange, 850.f, 1.e-3f);
	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
