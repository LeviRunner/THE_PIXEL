#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GameplayEffectTypes.h"
#include "DSProjectile.generated.h"

class ADSCharacterBase;
class UProjectileMovementComponent;
class USphereComponent;

/** Parâmetros de disparo (servidor). */
USTRUCT(BlueprintType)
struct FDSProjectileParams
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite) float Speed = 2000.f;
	/** Distância máxima (cm). No fim do alcance, projéteis em área explodem. */
	UPROPERTY(BlueprintReadWrite) float Range = 1000.f;
	/** > 0 = explode em área (projectile_aoe) ao acertar, no terreno ou no fim do alcance. */
	UPROPERTY(BlueprintReadWrite) float AoERadius = 0.f;
	/** Quantos inimigos extras atravessa (tiros perfurantes: alto). */
	UPROPERTY(BlueprintReadWrite) int32 PierceCount = 0;
};

/**
 * Projétil replicado (heróis e inimigos). Colisão e dano só no servidor; clientes só simulam
 * o movimento. O dano vem num FGameplayEffectSpecHandle pronto (snapshot do atacante).
 */
UCLASS()
class DUNGEONSTANCE_API ADSProjectile : public AActor
{
	GENERATED_BODY()

public:
	ADSProjectile();

	/** Spawna e inicializa (servidor). Direction no plano ou 3D. */
	static ADSProjectile* SpawnProjectile(TSubclassOf<ADSProjectile> ProjectileClass, ADSCharacterBase* Shooter, const FVector& Origin,
		const FVector& Direction, const FGameplayEffectSpecHandle& DamageSpec, const FDSProjectileParams& Params);

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void Tick(float DeltaSeconds) override;

protected:
	virtual void BeginPlay() override;

	UFUNCTION()
	void OnOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void OnHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);

	UFUNCTION()
	void OnRep_Speed();

	/** Gancho visual de impacto/explosão em todas as máquinas. */
	UFUNCTION(BlueprintImplementableEvent, Category = "DS|Projectile", meta = (DisplayName = "On Impact"))
	void K2_OnImpact(FVector Location, float Radius);

	UFUNCTION(NetMulticast, Unreliable)
	void Multicast_Impact(FVector_NetQuantize Location, float Radius);

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "DS|Projectile")
	TObjectPtr<USphereComponent> Collision;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "DS|Projectile")
	TObjectPtr<UProjectileMovementComponent> Movement;

	UPROPERTY(ReplicatedUsing = OnRep_Speed)
	float ReplicatedSpeed = 0.f;

private:
	bool TryHitCharacter(ADSCharacterBase* Target);
	void Explode(const FVector& Location);
	void ApplyDamage(ADSCharacterBase* Target) const;
	void Finish(const FVector& Location);

	FGameplayEffectSpecHandle DamageSpec;
	TWeakObjectPtr<ADSCharacterBase> Shooter;
	FDSProjectileParams Params;
	FVector StartLocation = FVector::ZeroVector;
	TSet<TWeakObjectPtr<AActor>> AlreadyHit;
	int32 RemainingPierce = 0;
	bool bFinished = false;
};
