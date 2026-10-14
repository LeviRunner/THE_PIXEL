// Testes de automação das regras de cartas (port de cards.ts) e da resolução das habilidades de apoio.
// Valores esperados copiados de Nakama_Legacy_Reference/test (cards_tree.test.ts, cards_abilities.test.ts).
// Rodar: Session Frontend → Automation → filtro "DungeonStance.Gameplay.Cards".

#include "Misc/AutomationTest.h"
#include "Math/RandomStream.h"
#include "UObject/Package.h"
#include "Gameplay/Abilities/DSSupportAbilityData.h"
#include "Gameplay/Abilities/DSSupportResolver.h"
#include "Gameplay/Cards/DSCardLibrary.h"
#include "Gameplay/Data/DSCardData.h"

#if WITH_DEV_AUTOMATION_TESTS

namespace DSCardTestUtils
{
	constexpr EAutomationTestFlags TestFlags = EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter;
	constexpr float Tolerance = 1.e-4f;

	FDSCardModifier Mod(const TCHAR* Stat, EDSModifierOp Op, float Value)
	{
		FDSCardModifier M;
		M.Stat = Stat;
		M.Op = Op;
		M.Value = Value;
		return M;
	}

	FDSCardDefinition Card(const TCHAR* Id, EDSStance Stance, int32 Tier, int32 Branch, TArray<FName> Parents, int32 MaxStacks,
		float Weight, TArray<FDSCardModifier> Modifiers)
	{
		FDSCardDefinition C;
		C.CardId = Id;
		C.Stance = Stance;
		C.Tier = Tier;
		C.Branch = Branch;
		C.Parents = MoveTemp(Parents);
		C.MaxStacks = MaxStacks;
		C.Weight = Weight;
		C.Modifiers = MoveTemp(Modifiers);
		return C;
	}

	/** Subconjunto real de cards_critical.ts / cards_support.ts. */
	UDSCardRegistry* MakeRegistry()
	{
		constexpr EDSModifierOp Add = EDSModifierOp::Add;
		constexpr EDSModifierOp Mult = EDSModifierOp::Mult;
		TArray<FDSCardDefinition> Cards;
		Cards.Add(Card(TEXT("keen_eye"), EDSStance::Critical, 1, 0, {}, 5, 10.f, { Mod(TEXT("crit_chance"), Add, 0.05f) }));
		Cards.Add(Card(TEXT("blood_drop"), EDSStance::Critical, 1, 1, {}, 5, 10.f, { Mod(TEXT("lifesteal"), Add, 0.01f) }));
		Cards.Add(Card(TEXT("lethal_precision"), EDSStance::Critical, 2, 0, { TEXT("keen_eye") }, 3, 5.f,
			{ Mod(TEXT("crit_chance"), Add, 0.08f), Mod(TEXT("damage"), Mult, 1.05f) }));
		Cards.Add(Card(TEXT("critical_thirst"), EDSStance::Critical, 2, 1, { TEXT("keen_eye"), TEXT("blood_drop") }, 2, 4.f,
			{ Mod(TEXT("crit_heal_pct"), Add, 0.02f) }));
		Cards.Add(Card(TEXT("support_power"), EDSStance::Support, 1, 0, {}, 5, 10.f, { Mod(TEXT("support_power"), Add, 0.15f) }));
		Cards.Add(Card(TEXT("commander"), EDSStance::Support, 6, 0, { TEXT("command_voice") }, 1, 2.f,
			{ Mod(TEXT("support_power"), Add, 0.3f), Mod(TEXT("support_radius"), Mult, 1.25f) }));
		Cards.Add(Card(TEXT("tactical_recharge"), EDSStance::Support, 1, 3, {}, 4, 10.f, { Mod(TEXT("support_cooldown_reduction"), Add, 0.08f) }));
		Cards.Add(Card(TEXT("grand_tactician"), EDSStance::Support, 12, 3, {}, 1, 1.f, { Mod(TEXT("support_cdr_cap_bonus"), Add, 0.15f) }));
		Cards.Add(Card(TEXT("renewal"), EDSStance::Support, 11, 1, {}, 3, 5.f,
			{ Mod(TEXT("heal_cooldown_reduction"), Add, 0.1f), Mod(TEXT("support_heal_power"), Add, 0.1f) }));

		UDSCardRegistry* Registry = NewObject<UDSCardRegistry>(GetTransientPackage());
		Registry->SetCardsForTesting(Cards);
		return Registry;
	}

	using FStacks = TMap<FName, int32>;
}


IMPLEMENT_SIMPLE_AUTOMATION_TEST(FDSCardParentsGateTest, "DungeonStance.Gameplay.Cards.ParentsGate", DSCardTestUtils::TestFlags)
bool FDSCardParentsGateTest::RunTest(const FString& Parameters)
{
	const UDSCardRegistry* Registry = DSCardTestUtils::MakeRegistry();
	const FDSCardDefinition* Child = Registry->FindCard(TEXT("critical_thirst"));
	if (!TestNotNull(TEXT("critical_thirst existe"), Child))
	{
		return false;
	}
	// cards_tree.test.ts "unlocks a child once one parent has a stack"
	TestFalse(TEXT("Sem pais: bloqueada"), UDSCardLibrary::ParentsOwned(*Child, {}));
	TestTrue(TEXT("Um pai basta"), UDSCardLibrary::ParentsOwned(*Child, DSCardTestUtils::FStacks{ { TEXT("blood_drop"), 1 } }));
	TestTrue(TEXT("Sorteável com keen_eye"), UDSCardLibrary::IsCardRollable(*Child, DSCardTestUtils::FStacks{ { TEXT("keen_eye"), 1 } }));
	TestFalse(TEXT("No máximo de acúmulos"), UDSCardLibrary::IsCardRollable(*Child, DSCardTestUtils::FStacks{ { TEXT("keen_eye"), 1 }, { TEXT("critical_thirst"), 2 } }));

	const FDSCardDefinition* TierOne = Registry->FindCard(TEXT("keen_eye"));
	TestTrue(TEXT("Tier 1 sempre liberado"), TierOne && UDSCardLibrary::ParentsOwned(*TierOne, {}));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FDSCardRollTest, "DungeonStance.Gameplay.Cards.Roll", DSCardTestUtils::TestFlags)
bool FDSCardRollTest::RunTest(const FString& Parameters)
{
	const UDSCardRegistry* Registry = DSCardTestUtils::MakeRegistry();
	FRandomStream Random(1234);

	// Build vazio: só tier 1 da árvore (2 cartas de Crítico no subconjunto).
	const TArray<FName> Empty = UDSCardLibrary::RollCardChoices(Registry, EDSStance::Critical, {}, 3, Random);
	TestEqual(TEXT("Só as 2 de tier 1"), Empty.Num(), 2);
	for (const FName& Id : Empty)
	{
		const FDSCardDefinition* Card = Registry->FindCard(Id);
		TestTrue(TEXT("Tier 1"), Card && Card->Tier == 1 && Card->Stance == EDSStance::Critical);
	}

	// Com keen_eye: os filhos entram; sem repetição.
	const TArray<FName> WithParent = UDSCardLibrary::RollCardChoices(Registry, EDSStance::Critical, DSCardTestUtils::FStacks{ { TEXT("keen_eye"), 1 } }, 10, Random);
	TestEqual(TEXT("4 cartas sorteáveis"), WithParent.Num(), 4);
	TestTrue(TEXT("lethal_precision liberada"), WithParent.Contains(FName(TEXT("lethal_precision"))));
	TestTrue(TEXT("critical_thirst liberada"), WithParent.Contains(FName(TEXT("critical_thirst"))));
	TSet<FName> Unique(WithParent);
	TestEqual(TEXT("Sem repetição"), Unique.Num(), WithParent.Num());

	// Determinístico por seed.
	FRandomStream A(42);
	FRandomStream B(42);
	const TArray<FName> RollA = UDSCardLibrary::RollCardChoices(Registry, EDSStance::Critical, DSCardTestUtils::FStacks{ { TEXT("keen_eye"), 1 } }, 2, A);
	const TArray<FName> RollB = UDSCardLibrary::RollCardChoices(Registry, EDSStance::Critical, DSCardTestUtils::FStacks{ { TEXT("keen_eye"), 1 } }, 2, B);
	TestTrue(TEXT("Mesma seed, mesmo sorteio"), RollA == RollB);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FDSCardValidateTest, "DungeonStance.Gameplay.Cards.ValidatePick", DSCardTestUtils::TestFlags)
bool FDSCardValidateTest::RunTest(const FString& Parameters)
{
	const UDSCardRegistry* Registry = DSCardTestUtils::MakeRegistry();
	auto Validate = [Registry](const TCHAR* Id, EDSStance Stance, const TMap<FName, int32>& S, int32 Pending)
	{
		return UDSCardLibrary::ValidateCardPick(Registry, FName(Id), Stance, S, Pending);
	};
	TestEqual(TEXT("Pai não possuído"), Validate(TEXT("lethal_precision"), EDSStance::Critical, {}, 1), EDSCardPickResult::ParentNotOwned);
	TestEqual(TEXT("Válida com pai"), Validate(TEXT("lethal_precision"), EDSStance::Critical, DSCardTestUtils::FStacks{ { TEXT("keen_eye"), 1 } }, 1), EDSCardPickResult::Ok);
	TestEqual(TEXT("Carta desconhecida"), Validate(TEXT("constructor"), EDSStance::Critical, {}, 1), EDSCardPickResult::UnknownCard);
	TestEqual(TEXT("Árvore errada"), Validate(TEXT("keen_eye"), EDSStance::Support, {}, 1), EDSCardPickResult::WrongTree);
	TestEqual(TEXT("No máximo"), Validate(TEXT("commander"), EDSStance::Support, DSCardTestUtils::FStacks{ { TEXT("commander"), 1 } }, 5), EDSCardPickResult::MaxStacks);
	TestEqual(TEXT("Sem escolha disponível"), Validate(TEXT("support_power"), EDSStance::Support, {}, 0), EDSCardPickResult::NoPickAvailable);
	TestEqual(TEXT("Mensagem do protótipo"), UDSCardLibrary::PickResultToString(EDSCardPickResult::ParentNotOwned), FString(TEXT("parent not owned")));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FDSCardAggregateTest, "DungeonStance.Gameplay.Cards.AggregateStat", DSCardTestUtils::TestFlags)
bool FDSCardAggregateTest::RunTest(const FString& Parameters)
{
	const UDSCardRegistry* Registry = DSCardTestUtils::MakeRegistry();
	// cards_abilities.test.ts "aggregateCardStat"
	TestNearlyEqual(TEXT("support_power 2× + commander"),
		UDSCardLibrary::AggregateCardStat(Registry, DSCardTestUtils::FStacks{ { TEXT("support_power"), 2 }, { TEXT("commander"), 1 } }, TEXT("support_power"), 0.f), 0.6f, DSCardTestUtils::Tolerance);
	TestNearlyEqual(TEXT("support_radius mult"),
		UDSCardLibrary::AggregateCardStat(Registry, DSCardTestUtils::FStacks{ { TEXT("commander"), 1 } }, TEXT("support_radius"), 1.f), 1.25f, DSCardTestUtils::Tolerance);
	TestNearlyEqual(TEXT("Carta inexistente não soma"),
		UDSCardLibrary::AggregateCardStat(Registry, DSCardTestUtils::FStacks{ { TEXT("ghost_card"), 3 } }, TEXT("support_power"), 0.f), 0.f, DSCardTestUtils::Tolerance);
	// (base + Σadd) × Πmult com acúmulos: dano 38 × 1,05² = 41,895
	TestNearlyEqual(TEXT("Mult por acúmulo"),
		UDSCardLibrary::AggregateCardStat(Registry, DSCardTestUtils::FStacks{ { TEXT("lethal_precision"), 2 } }, TEXT("damage"), 38.f), 38.f * 1.05f * 1.05f, DSCardTestUtils::Tolerance);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FDSSupportResolveTest, "DungeonStance.Gameplay.Cards.SupportResolve", DSCardTestUtils::TestFlags)
bool FDSSupportResolveTest::RunTest(const FString& Parameters)
{
	const UDSCardRegistry* Registry = DSCardTestUtils::MakeRegistry();
	auto StatFor = [Registry](const TMap<FName, int32>& S)
	{
		return [Registry, S](FName Stat, float Base) { return UDSCardLibrary::AggregateCardStat(Registry, S, Stat, Base); };
	};

	// war_banner: zona, 10 s, raio 200 px = 500 cm, +15% dano, +15 armadura.
	UDSSupportAbilityData* Banner = NewObject<UDSSupportAbilityData>(GetTransientPackage());
	Banner->Kind = EDSSupportKind::Zone;
	Banner->Cooldown = 24.f;
	Banner->Radius = 500.f;
	Banner->Duration = 10.f;
	Banner->Magnitude = 0.15f;
	Banner->ZoneArmorBonus = 15.f;
	Banner->bZoneDamageBuff = true;
	{
		const auto NoCards = StatFor({});
		const FDSResolvedSupport R = DSSupportResolver::Resolve(*Banner, NoCards);
		TestNearlyEqual(TEXT("Estandarte: duração"), R.Duration, 10.f, DSCardTestUtils::Tolerance);
		TestNearlyEqual(TEXT("Estandarte: raio"), R.Radius, 500.f, DSCardTestUtils::Tolerance);
		TestNearlyEqual(TEXT("Estandarte: dano"), R.DamageMultiplier, 1.15f, DSCardTestUtils::Tolerance);
		TestNearlyEqual(TEXT("Estandarte: armadura"), R.ArmorBonus, 15.f, DSCardTestUtils::Tolerance);
	}

	// battle_cry: teto de 50% de redução; Grande Tático sobe para 65%.
	UDSSupportAbilityData* Cry = NewObject<UDSSupportAbilityData>(GetTransientPackage());
	Cry->Kind = EDSSupportKind::Haste;
	Cry->Cooldown = 18.f;
	Cry->Duration = 6.f;
	Cry->Magnitude = 0.25f;
	{
		const auto Capped = StatFor(DSCardTestUtils::FStacks{ { TEXT("tactical_recharge"), 40 } });
		TestNearlyEqual(TEXT("Recarga no teto"), DSSupportResolver::Resolve(*Cry, Capped).Cooldown, 18.f * 0.5f, DSCardTestUtils::Tolerance);
		const auto Raised = StatFor(DSCardTestUtils::FStacks{ { TEXT("tactical_recharge"), 40 }, { TEXT("grand_tactician"), 1 } });
		TestNearlyEqual(TEXT("Teto elevado"), DSSupportResolver::Resolve(*Cry, Raised).Cooldown, 18.f * 0.35f, DSCardTestUtils::Tolerance);
	}

	// rallying_pulse: Renovação reduz a recarga das curas.
	UDSSupportAbilityData* Pulse = NewObject<UDSSupportAbilityData>(GetTransientPackage());
	Pulse->Kind = EDSSupportKind::Heal;
	Pulse->Cooldown = 14.f;
	Pulse->Duration = 0.f;
	Pulse->Magnitude = 0.18f;
	Pulse->bIsHeal = true;
	{
		const auto Renewal = StatFor(DSCardTestUtils::FStacks{ { TEXT("renewal"), 2 } });
		const FDSResolvedSupport R = DSSupportResolver::Resolve(*Pulse, Renewal);
		TestNearlyEqual(TEXT("Recarga de cura"), R.Cooldown, 14.f * 0.8f, DSCardTestUtils::Tolerance);
		TestNearlyEqual(TEXT("Cura com poder de cura"), R.Magnitude, 0.18f * 1.2f, DSCardTestUtils::Tolerance);
	}
	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
