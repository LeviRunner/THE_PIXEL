#pragma once

#include "CoreMinimal.h"
#include "Gameplay/Abilities/DSGameplayAbility.h"
#include "DSGA_AreaStrike.generated.h"

class UAnimMontage;

/**
 * Skill de classe genérica em área (classes.ts SkillKind "nova" e "cone"):
 * acerta todos os hostis num arco (360° = nova) com DamageMultiplier × Dano.
 * Usada por Redemoinho (Bárbaro), Onda de Luz (Monge), Pulso Arcano (Arcanista),
 * Investida Sagrada (Cruzado, cone 80°) e Ceifa Sangrenta (Cavaleiro, cone 120°).
 * Opcional: impacto atrasado (ImpactDelay) e repetição (HitCount × HitInterval).
 */
UCLASS()
class DUNGEONSTANCE_API UDSGA_AreaStrike : public UDSGameplayAbility
{
	GENERATED_BODY()

public:
	UDSGA_AreaStrike(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

protected:
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;

	/** Executa um golpe (servidor). Retorna nº de atingidos. */
	virtual int32 PerformStrike();

	UFUNCTION()
	void OnStrikeTimer();

	UPROPERTY(EditDefaultsOnly, Category = "DS|Strike", meta = (Units = "cm"))
	float Radius = 350.f;

	/** 360 = em volta (nova). */
	UPROPERTY(EditDefaultsOnly, Category = "DS|Strike", meta = (Units = "deg", ClampMin = 1, ClampMax = 360))
	float ArcDegrees = 360.f;

	UPROPERTY(EditDefaultsOnly, Category = "DS|Strike")
	float DamageMultiplier = 2.f;

	/** Golpes (1 = instantâneo). */
	UPROPERTY(EditDefaultsOnly, Category = "DS|Strike", meta = (ClampMin = 1))
	int32 HitCount = 1;

	UPROPERTY(EditDefaultsOnly, Category = "DS|Strike", meta = (Units = "s"))
	float HitInterval = 0.25f;

	/** Atraso até o primeiro impacto (telegrafia). */
	UPROPERTY(EditDefaultsOnly, Category = "DS|Strike", meta = (Units = "s"))
	float ImpactDelay = 0.f;

	/** Roubo de vida extra da skill (Ceifa Sangrenta), fração do dano. */
	UPROPERTY(EditDefaultsOnly, Category = "DS|Strike")
	float BonusLifesteal = 0.f;

	UPROPERTY(EditDefaultsOnly, Category = "DS|Strike")
	TObjectPtr<UAnimMontage> Montage;

	/** GameplayCue de VFX/SFX no impacto (opcional). */
	UPROPERTY(EditDefaultsOnly, Category = "DS|Strike", meta = (Categories = "GameplayCue"))
	FGameplayTag ImpactCueTag;

private:
	void ScheduleNextStrike(float Delay);

	int32 StrikesDone = 0;
	FTimerHandle StrikeTimer;
};

/** Redemoinho do Bárbaro: nova de 140 px (350 cm), ×2,2, 30 de Fúria, recarga 6 s. */
UCLASS()
class DUNGEONSTANCE_API UDSGA_Whirlwind : public UDSGA_AreaStrike
{
	GENERATED_BODY()

public:
	UDSGA_Whirlwind(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());
};
