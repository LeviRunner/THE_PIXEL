#pragma once

#include "CoreMinimal.h"
#include "Gameplay/Effects/DSGameplayEffectBase.h"
#include "DSStatusEffects.generated.h"

// Status de status.ts como GEs. Aplique sempre via UDSStatusLibrary (ela preenche os SetByCaller
// e aplica as regras de chefe/teto). Todos usam Data.Duration; a intensidade vai em Data.Magnitude
// e é lida por UDSStatusLibrary::GetStrongestStatusMagnitude ("o mais forte vence", como no protótipo).

/** Base: duração por SetByCaller + tag de status concedida. */
UCLASS(Abstract)
class DUNGEONSTANCE_API UDSGE_TimedStatus : public UDSGameplayEffectBase
{
	GENERATED_BODY()
protected:
	void InitTimedStatus(FGameplayTag StatusTag);
};

/**
 * Base dos DoTs: aplica Data.Damage (dano por tique) em IncomingDamage a cada 0,5 s
 * (DOT_INTERVAL_TICKS). Acúmulos por alvo com teto; cada aplicação renova a duração e
 * o tique escala com o nº de acúmulos. Aproximação do protótipo: os acúmulos compartilham
 * o spec (duração e dono do primeiro).
 */
UCLASS(Abstract)
class DUNGEONSTANCE_API UDSGE_DamageOverTime : public UDSGE_TimedStatus
{
	GENERATED_BODY()
public:
	static constexpr float TickInterval = 0.5f;
protected:
	void InitDot(FGameplayTag StatusTag, int32 MaxStacks);
};

/** Sangramento: 3 s padrão, até 3 acúmulos (1 + Hemorragia). */
UCLASS()
class DUNGEONSTANCE_API UDSGE_Bleed : public UDSGE_DamageOverTime
{
	GENERATED_BODY()
public:
	UDSGE_Bleed();
	static constexpr float DefaultDuration = 3.f;
	static constexpr int32 MaxStacks = 3;
};

/** Queimadura: 4 s, até 3 acúmulos (1 + Inferno). */
UCLASS()
class DUNGEONSTANCE_API UDSGE_Burn : public UDSGE_DamageOverTime
{
	GENERATED_BODY()
public:
	UDSGE_Burn();
	static constexpr float DefaultDuration = 4.f;
	static constexpr int32 MaxStacks = 3;
};

/** Veneno: 5 s, até 5 acúmulos. */
UCLASS()
class DUNGEONSTANCE_API UDSGE_Poison : public UDSGE_DamageOverTime
{
	GENERATED_BODY()
public:
	UDSGE_Poison();
	static constexpr float DefaultDuration = 5.f;
	static constexpr int32 MaxStacks = 5;
};

/** Marca: +X% de dano recebido de todo o esquadrão (4 s). */
UCLASS()
class DUNGEONSTANCE_API UDSGE_Mark : public UDSGE_TimedStatus
{
	GENERATED_BODY()
public:
	UDSGE_Mark();
	static constexpr float DefaultDuration = 4.f;
};

/** Exposto: +X% de dano recebido (Golpe Coordenado). */
UCLASS()
class DUNGEONSTANCE_API UDSGE_Exposed : public UDSGE_TimedStatus
{
	GENERATED_BODY()
public:
	UDSGE_Exposed();
};

/** Enfraquecido: −X% de dano causado (Coroa do Inverno). */
UCLASS()
class DUNGEONSTANCE_API UDSGE_Weaken : public UDSGE_TimedStatus
{
	GENERATED_BODY()
public:
	UDSGE_Weaken();
};

/** Lentidão: o personagem multiplica a velocidade por (1 − maior lentidão ativa). */
UCLASS()
class DUNGEONSTANCE_API UDSGE_Slow : public UDSGE_TimedStatus
{
	GENERATED_BODY()
public:
	UDSGE_Slow();
};

/** Atordoamento: bloqueia habilidades (ActivationBlockedTags) e movimento. */
UCLASS()
class DUNGEONSTANCE_API UDSGE_Stun : public UDSGE_TimedStatus
{
	GENERATED_BODY()
public:
	UDSGE_Stun();
};

/** Provocação: a IA mira o instigador do efeito enquanto ativo. */
UCLASS()
class DUNGEONSTANCE_API UDSGE_Taunt : public UDSGE_TimedStatus
{
	GENERATED_BODY()
public:
	UDSGE_Taunt();
};
