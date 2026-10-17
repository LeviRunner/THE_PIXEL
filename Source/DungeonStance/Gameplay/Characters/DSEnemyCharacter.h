#pragma once

#include "CoreMinimal.h"
#include "Gameplay/Characters/DSCharacterBase.h"
#include "Gameplay/Data/DSEnemyData.h"
#include "GameplayEffectTypes.h"
#include "DSEnemyCharacter.generated.h"

class ADSEnemyCharacter;
class UDSEnemyData;

/** Inimigo morreu (servidor). Killer = controller de quem deu o golpe final. */
DECLARE_MULTICAST_DELEGATE_TwoParams(FDSOnEnemyDied, ADSEnemyCharacter* /*Enemy*/, AController* /*Killer*/);

/**
 * Inimigo com ASC próprio (replicação Minimal). Spawnado pelo ADSGameMode, que chama
 * InitializeFromData e escuta OnEnemyDied. A IA (ADSEnemyAIController) decide; este ator executa
 * os ataques (corpo a corpo por intervalo e projéteis) no servidor.
 */
UCLASS()
class DUNGEONSTANCE_API ADSEnemyCharacter : public ADSCharacterBase
{
	GENERATED_BODY()

public:
	ADSEnemyCharacter(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void PostInitializeComponents() override;

	/**
	 * Servidor, logo após o spawn. Level = nível efetivo da arena; multiplicadores da onda (DSRunRules);
	 * bElite aplica ×vida/×DPS do data asset. MaxHealth final = vida do nível × HealthMultiplier.
	 */
	void InitializeFromData(const UDSEnemyData* Data, int32 Level, float HealthMultiplier, float DamageMultiplier, bool bElite);

	/** Contrato com o Core. */
	FDSOnEnemyDied OnEnemyDied;

	UFUNCTION(BlueprintPure, Category = "DS|Enemy")
	const UDSEnemyData* GetEnemyData() const { return EnemyData; }

	const FDSEnemyStats& GetStats() const { return Stats; }

	UFUNCTION(BlueprintPure, Category = "DS|Enemy")
	bool IsElite() const { return bElite; }

	UFUNCTION(BlueprintPure, Category = "DS|Enemy")
	bool IsBoss() const;

	UFUNCTION(BlueprintPure, Category = "DS|Enemy")
	bool IsStunned() const;

	UFUNCTION(BlueprintPure, Category = "DS|Enemy")
	float GetHealthFraction() const;

	/** Já apanhou alguma vez (comportamento passivo). */
	bool HasBeenDamaged() const { return bHasBeenDamaged; }

	// --- Ataques (servidor) ---
	bool IsAttackReady() const;
	/** Golpe corpo a corpo: dano fixo = DPS de contato × intervalo. */
	bool PerformMeleeAttack(ADSCharacterBase* Target);
	/** Disparo de projétil esquivável. */
	bool PerformRangedAttack(ADSCharacterBase* Target);
	bool IsRanged() const { return Stats.AttackRange > 0.f; }

	/** Investida (charger/brood): multiplica a velocidade por um tempo. */
	void StartLunge(float SpeedMultiplier, float Duration);

	/** Escala o próximo ataque (primeira rajada escalonada, como buildWave). */
	void DelayNextAttack(float Seconds);

protected:
	virtual void Die(AController* Killer) override;
	virtual void HandleDamageTaken(AActor* DamageInstigator, AActor* DamageCauser, const FGameplayEffectSpec* Spec, float Magnitude) override;
	virtual float GetExtraSpeedMultiplier() const override;

	UFUNCTION()
	void OnRep_EnemyData();

	/** Visual do ataque em todas as máquinas. */
	UFUNCTION(NetMulticast, Unreliable)
	void Multicast_OnAttack(bool bRanged);

	UFUNCTION(BlueprintImplementableEvent, Category = "DS|Enemy", meta = (DisplayName = "On Attack"))
	void K2_OnAttack(bool bRanged);

	/** Data replicado: clientes trocam a malha/visual. */
	UFUNCTION(BlueprintImplementableEvent, Category = "DS|Enemy", meta = (DisplayName = "On Enemy Data Set"))
	void K2_OnEnemyDataSet(const UDSEnemyData* Data, bool bIsElite);

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "DS|Enemy")
	TObjectPtr<UDSAbilitySystemComponent> EnemyAbilitySystem;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "DS|Enemy")
	TObjectPtr<UDSAttributeSet> EnemyAttributes;

	UPROPERTY(ReplicatedUsing = OnRep_EnemyData)
	TObjectPtr<UDSEnemyData> EnemyData;

	UPROPERTY(Replicated)
	bool bElite = false;

private:
	void ApplyStats(float HealthMultiplier, float DamageMultiplier);
	FGameplayEffectSpecHandle MakeFlatDamageSpec(float Damage) const;
	void ApplyMesh();

	FDSEnemyStats Stats;
	float NextAttackTime = 0.f;
	float LungeEndTime = 0.f;
	float LungeMultiplier = 1.f;
	bool bHasBeenDamaged = false;
};
