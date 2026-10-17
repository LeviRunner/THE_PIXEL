#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Gameplay/Abilities/DSSupportResolver.h"
#include "DSSupportZone.generated.h"

class ADSCharacterBase;

/**
 * Zona de apoio no chão (Estandarte de Guerra, Santuário, Totem Guardião).
 * Servidor: a cada TickInterval aplica UDSGE_ZoneBuff (renovado) e a regeneração nos aliados dentro.
 * Totem (bTaunts): a IA inimiga próxima prefere atacar o dono da zona (ver ADSEnemyAIController).
 */
UCLASS()
class DUNGEONSTANCE_API ADSSupportZone : public AActor
{
	GENERATED_BODY()

public:
	ADSSupportZone();

	/** Servidor: configura e começa a pulsar. */
	void InitZone(ADSCharacterBase* InCaster, const FDSResolvedSupport& InResolved);

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UFUNCTION(BlueprintPure, Category = "DS|Zone")
	float GetRadius() const { return Radius; }

	UFUNCTION(BlueprintPure, Category = "DS|Zone")
	bool IsTaunting() const { return bTaunts; }

	ADSCharacterBase* GetCaster() const { return Caster.Get(); }

	/** Zona provocadora mais próxima que alcança Location (para a IA). */
	static ADSSupportZone* FindTauntZoneAt(const UWorld* World, const FVector& Location);

protected:
	/** Visual: círculo do raio etc. Chamado em todas as máquinas quando o raio replica. */
	UFUNCTION(BlueprintImplementableEvent, Category = "DS|Zone", meta = (DisplayName = "On Zone Started"))
	void K2_OnZoneStarted(float InRadius, float InDuration);

	UFUNCTION()
	void OnRep_Radius();

	void Pulse();

	UPROPERTY(EditDefaultsOnly, Category = "DS|Zone", meta = (Units = "s"))
	float TickInterval = 0.5f;

	UPROPERTY(ReplicatedUsing = OnRep_Radius)
	float Radius = 0.f;

	UPROPERTY(Replicated)
	float Duration = 0.f;

	UPROPERTY(Replicated)
	bool bTaunts = false;

private:
	TWeakObjectPtr<ADSCharacterBase> Caster;
	FDSResolvedSupport Resolved;
	FTimerHandle PulseTimer;
};
