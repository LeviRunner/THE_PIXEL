#pragma once

#include "CoreMinimal.h"
#include "Gameplay/Effects/DSGameplayEffectBase.h"
#include "DSGameplayEffects.generated.h"

// GEs genéricos do Cast Framework. Todos os números vêm por SetByCaller (Data Assets / abilities),
// exceto a estamina, cujos defaults do protótipo (squad_logic.ts) ficam aqui e podem ser
// sobrescritos num Blueprint filho.

/** Dano instantâneo: UDSDamageExecution. SetByCaller: Data.Magnitude (mult. do dano do atacante) e/ou Data.Damage (dano fixo). */
UCLASS()
class DUNGEONSTANCE_API UDSGE_Damage : public UDSGameplayEffectBase
{
	GENERATED_BODY()
public:
	UDSGE_Damage();
};

/** Cura instantânea (meta IncomingHeal). SetByCaller: Data.Heal. */
UCLASS()
class DUNGEONSTANCE_API UDSGE_Heal : public UDSGameplayEffectBase
{
	GENERATED_BODY()
public:
	UDSGE_Heal();
};

/** Escudo instantâneo (soma ao atributo Shield). SetByCaller: Data.Magnitude. */
UCLASS()
class DUNGEONSTANCE_API UDSGE_AddShield : public UDSGameplayEffectBase
{
	GENERATED_BODY()
public:
	UDSGE_AddShield();
};

/** Custo de recurso. SetByCaller: Data.Magnitude (valor negativo). */
UCLASS()
class DUNGEONSTANCE_API UDSGE_Cost : public UDSGameplayEffectBase
{
	GENERATED_BODY()
public:
	UDSGE_Cost();
};

/** Recarga genérica. SetByCaller: Data.Duration. As tags de recarga entram como DynamicGrantedTags no spec. */
UCLASS()
class DUNGEONSTANCE_API UDSGE_Cooldown : public UDSGameplayEffectBase
{
	GENERATED_BODY()
public:
	UDSGE_Cooldown();
};

/** I-frames da esquiva: concede State.Invulnerable. SetByCaller: Data.Duration. */
UCLASS()
class DUNGEONSTANCE_API UDSGE_Invulnerable : public UDSGameplayEffectBase
{
	GENERATED_BODY()
public:
	UDSGE_Invulnerable();
};

/**
 * Buff temporário das habilidades de apoio / zonas. SetByCaller obrigatórios (use UDSEffectLibrary::MakeSupportBuffSpec):
 * Data.Duration, Data.Buff.Damage (mult.), Data.Buff.AttackSpeed (mult.), Data.Buff.MoveSpeed (mult.),
 * Data.Buff.Armor (soma), Data.Buff.DamageReduction (soma), Data.Buff.CritChance (soma).
 */
UCLASS()
class DUNGEONSTANCE_API UDSGE_SupportBuff : public UDSGameplayEffectBase
{
	GENERATED_BODY()
public:
	UDSGE_SupportBuff();
};

/** Buff de zona (Estandarte, Santuário, Totem): mesmo formato, 1 instância por fonte renovada a cada tique. */
UCLASS()
class DUNGEONSTANCE_API UDSGE_ZoneBuff : public UDSGE_SupportBuff
{
	GENERATED_BODY()
public:
	UDSGE_ZoneBuff();
};

/**
 * Modificadores das cartas (infinito). Um par Add/Mult por atributo de DSCardStatBindings
 * (tags Data.Card.<Atributo>.Add/.Mult). Resultado = (base + ΣAdd) × ΠMult, como aggregateCardStat.
 * O UDSStanceComponent remove e reaplica a cada carta escolhida.
 */
UCLASS()
class DUNGEONSTANCE_API UDSGE_CardStats : public UDSGameplayEffectBase
{
	GENERATED_BODY()
public:
	UDSGE_CardStats();
};

/** Regeneração de vida e recurso a cada 0,25 s, lida dos atributos HealthRegen e ResourceRegen. */
UCLASS()
class DUNGEONSTANCE_API UDSGE_Regen : public UDSGameplayEffectBase
{
	GENERATED_BODY()
public:
	UDSGE_Regen();

	static constexpr float TickPeriod = 0.25f;
};

/** Regeneração de estamina (28/s), inibida enquanto State.Sprinting. */
UCLASS()
class DUNGEONSTANCE_API UDSGE_StaminaRegen : public UDSGameplayEffectBase
{
	GENERATED_BODY()
public:
	UDSGE_StaminaRegen();

	static constexpr float TickPeriod = 0.1f;
	static constexpr float DefaultRegenPerSecond = 28.f;
};

/** Gasto de estamina do sprint (20/s), aplicado pela UDSGA_Sprint enquanto ativa. */
UCLASS()
class DUNGEONSTANCE_API UDSGE_SprintDrain : public UDSGameplayEffectBase
{
	GENERATED_BODY()
public:
	UDSGE_SprintDrain();

	static constexpr float TickPeriod = 0.1f;
	static constexpr float DefaultDrainPerSecond = 20.f;
};
