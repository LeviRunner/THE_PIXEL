#pragma once

#include "CoreMinimal.h"
#include "Templates/Function.h"
#include "Gameplay/Abilities/DSSupportAbilityData.h"

/** Números finais de uma habilidade de apoio depois das cartas do lançador (abilities.ts ResolvedAbility). */
struct FDSResolvedSupport
{
	EDSSupportKind Kind = EDSSupportKind::DamageBuff;
	float Cooldown = 0.f;
	float Radius = 0.f;
	float Duration = 0.f;
	float Magnitude = 0.f;
	/** Multiplicadores/bônus temporários para aliados. */
	float DamageMultiplier = 1.f;
	float AttackSpeedMultiplier = 1.f;
	float MoveSpeedMultiplier = 1.f;
	float ArmorBonus = 0.f;
	float CritChanceBonus = 0.f;
	float DamageReduction = 0.f;
	/** Fração da vida máx. curada por segundo durante Duration. */
	float RegenPct = 0.f;
	bool bTaunts = false;
};

/** Port de abilities.ts resolveAbility. Funções puras (testáveis). */
namespace DSSupportResolver
{
	inline constexpr float MaxSupportCooldownReduction = 0.5f;
	inline constexpr float MaxZoneDamageReduction = 0.5f;

	/** CardStat(stat, base) = aggregateCardStat das cartas do lançador. */
	using FCardStatFn = TFunctionRef<float(FName /*Stat*/, float /*Base*/)>;

	DUNGEONSTANCE_API float SupportCooldownReduction(const UDSSupportAbilityData& Data, FCardStatFn CardStat);
	DUNGEONSTANCE_API FDSResolvedSupport Resolve(const UDSSupportAbilityData& Data, FCardStatFn CardStat);
}
