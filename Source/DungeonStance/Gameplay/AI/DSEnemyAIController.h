#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "Gameplay/Data/DSEnemyData.h"
#include "DSEnemyAIController.generated.h"

class ADSCharacterBase;
class ADSEnemyCharacter;

/**
 * IA dos inimigos (port de bestiary_ai.ts), só servidor. Decide a cada ThinkInterval:
 * escolhe alvo (provocação > totem > herói mais próximo) e executa o comportamento do UDSEnemyData:
 *  Melee, Swarm (leque), Charger (investida), Blinker (teleporte), Kiter, Ranged, Passive, Brood.
 * Movimento por NavMesh (MoveToActor/MoveToLocation). Pensado para ser substituído por
 * StateTree depois, mantendo os mesmos "verbos" no ADSEnemyCharacter.
 */
UCLASS()
class DUNGEONSTANCE_API ADSEnemyAIController : public AAIController
{
	GENERATED_BODY()

public:
	ADSEnemyAIController();

	virtual void Tick(float DeltaSeconds) override;

	UFUNCTION(BlueprintPure, Category = "DS|AI")
	ADSCharacterBase* GetCurrentTarget() const { return CurrentTarget.Get(); }

protected:
	virtual void OnPossess(APawn* InPawn) override;
	virtual void OnUnPossess() override;

	/** Intervalo entre decisões (o protótipo rodava a 10 Hz). */
	UPROPERTY(EditDefaultsOnly, Category = "DS|AI", meta = (Units = "s"))
	float ThinkInterval = 0.15f;

private:
	void Think();
	ADSCharacterBase* SelectTarget() const;
	ADSCharacterBase* FindNearestHero() const;

	void ThinkMelee(ADSCharacterBase* Target, const FDSEnemyAITuning& Tuning);
	void ThinkSwarm(ADSCharacterBase* Target, const FDSEnemyAITuning& Tuning);
	void ThinkCharger(ADSCharacterBase* Target, const FDSEnemyAITuning& Tuning);
	void ThinkBlinker(ADSCharacterBase* Target, const FDSEnemyAITuning& Tuning);
	void ThinkRanged(ADSCharacterBase* Target, const FDSEnemyAITuning& Tuning, bool bKite);
	void ThinkPassive(ADSCharacterBase* Target, const FDSEnemyAITuning& Tuning);
	void ThinkBrood(ADSCharacterBase* Target, const FDSEnemyAITuning& Tuning);

	/** Distância entre bordas das cápsulas (2D). */
	float EdgeDistance(const ADSCharacterBase* Target) const;
	void FaceTarget(const ADSCharacterBase* Target) const;
	void ChaseAndHit(ADSCharacterBase* Target, const FDSEnemyAITuning& Tuning);
	void MoveAwayFrom(const ADSCharacterBase* Threat, float Distance);
	void Wander(const FDSEnemyAITuning& Tuning);
	void SummonBrood(const FDSEnemyAITuning& Tuning);

	TWeakObjectPtr<ADSEnemyCharacter> Enemy;
	TWeakObjectPtr<ADSCharacterBase> CurrentTarget;
	float ThinkAccumulator = 0.f;
	float NextLungeTime = 0.f;
	float NextWanderTime = 0.f;
	bool bBlinked = false;
	int32 BroodThresholdsUsed = 0;
	/** Lado do leque do enxame (+1/−1), sorteado ao possuir. */
	float SwarmSide = 1.f;
};
