#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "DSTargetLockComponent.generated.h"

class ADSCharacterBase;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FDSOnTargetLockChanged, ADSCharacterBase*, NewTarget);

/**
 * Lock-on (regra base de todas as câmeras): clique do scroll / R3 liga/desliga; scroll ou
 * analógico direito troca de alvo. Escolhe o alvo hostil vivo mais próximo do centro da mira;
 * solta se o alvo morrer ou sair do alcance. Toda mira (ataques, skills) usa o alvo travado
 * (ADSHeroCharacter::GetAimDirection). O cliente escolhe e o servidor valida (RPC).
 */
UCLASS(ClassGroup = (DungeonStance), meta = (BlueprintSpawnableComponent))
class DUNGEONSTANCE_API UDSTargetLockComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UDSTargetLockComponent();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	/** Liga (melhor alvo na mira) ou desliga o lock. */
	UFUNCTION(BlueprintCallable, Category = "DS|TargetLock")
	void ToggleLock();

	/** Troca para o próximo alvo à direita (Direction > 0) ou à esquerda (< 0). */
	UFUNCTION(BlueprintCallable, Category = "DS|TargetLock")
	void SwitchTarget(float Direction);

	UFUNCTION(BlueprintCallable, Category = "DS|TargetLock")
	void ClearLock();

	UFUNCTION(BlueprintPure, Category = "DS|TargetLock")
	ADSCharacterBase* GetLockedTarget() const { return LockedTarget; }

	UFUNCTION(BlueprintPure, Category = "DS|TargetLock")
	bool IsLocked() const { return LockedTarget != nullptr; }

	UPROPERTY(BlueprintAssignable, Category = "DS|TargetLock")
	FDSOnTargetLockChanged OnTargetLockChanged;

protected:
	UFUNCTION(Server, Reliable)
	void Server_SetLockedTarget(ADSCharacterBase* NewTarget);

	UFUNCTION()
	void OnRep_LockedTarget();

	/** Alcance para travar (cm). */
	UPROPERTY(EditDefaultsOnly, Category = "DS|TargetLock", meta = (Units = "cm"))
	float LockRange = 2500.f;

	/** Solta acima desta distância (cm). */
	UPROPERTY(EditDefaultsOnly, Category = "DS|TargetLock", meta = (Units = "cm"))
	float BreakRange = 3000.f;

	/** Ângulo máximo entre a mira e o alvo para travar (graus). */
	UPROPERTY(EditDefaultsOnly, Category = "DS|TargetLock", meta = (Units = "deg"))
	float MaxLockAngle = 75.f;

	/** Peso da distância na escolha (graus equivalentes por LockRange). */
	UPROPERTY(EditDefaultsOnly, Category = "DS|TargetLock")
	float DistanceWeight = 20.f;

	/** Velocidade com que a câmera vira para o alvo. */
	UPROPERTY(EditDefaultsOnly, Category = "DS|TargetLock")
	float CameraInterpSpeed = 8.f;

	/** Pitch fixo da câmera enquanto travado (graus, negativo = olhando para baixo). */
	UPROPERTY(EditDefaultsOnly, Category = "DS|TargetLock", meta = (Units = "deg"))
	float LockedCameraPitch = -20.f;

	UPROPERTY(ReplicatedUsing = OnRep_LockedTarget)
	TObjectPtr<ADSCharacterBase> LockedTarget;

private:
	ADSCharacterBase* GetOwnerCharacter() const;
	FVector GetViewForward() const;
	void GatherCandidates(TArray<ADSCharacterBase*>& OutCandidates) const;
	ADSCharacterBase* FindBestTarget() const;
	bool IsTargetValid(const ADSCharacterBase* Target) const;
	void SetLockedTargetLocal(ADSCharacterBase* NewTarget);
	void UpdateRotationMode() const;
	void RotateCameraTowardTarget(float DeltaTime) const;
};
