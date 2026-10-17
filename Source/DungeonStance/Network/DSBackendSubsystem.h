#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Network/DSBackendTypes.h"
#include "DSBackendSubsystem.generated.h"

class IDSBackendService;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FDSOnBackendAuthenticated, bool, bWasSuccessful, const FString&, PlayerId);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FDSOnBackendProfileFetched, bool, bWasSuccessful, const FDSPlayerProfile&, Profile);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FDSOnBackendRunResultsSubmitted, bool, bWasSuccessful);

/**
 * Porta de entrada do backend de conta para Core/UI. Escolhe a implementação por
 * UDSNetworkSettings::Backend (Null ou Nakama), guarda o perfil em cache e expõe delegates.
 */
UCLASS()
class DUNGEONSTANCE_API UDSBackendSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	/** Login com o device id persistente deste computador (por instância de PIE). */
	UFUNCTION(BlueprintCallable, Category = "DS|Backend")
	void Authenticate();

	/** Perfil + rank por classe; resultado em OnProfileFetched e em GetCachedProfile(). */
	UFUNCTION(BlueprintCallable, Category = "DS|Backend")
	void FetchProfile();

	/** SÓ SERVIDOR (ou Standalone): persiste o fim de run. Rejeitado em clientes. */
	UFUNCTION(BlueprintCallable, Category = "DS|Backend")
	void SubmitRunResults(const TArray<FDSRunResult>& Results);

	/** SÓ SERVIDOR: valida o token enviado pelo cliente no login e devolve o PlayerId confiável. */
	void VerifyPlayerToken(const FString& SessionToken, FDSOnBackendTokenVerified OnComplete);

	UFUNCTION(BlueprintPure, Category = "DS|Backend")
	bool IsAuthenticated() const;

	UFUNCTION(BlueprintPure, Category = "DS|Backend")
	FString GetPlayerId() const;

	UFUNCTION(BlueprintPure, Category = "DS|Backend")
	const FDSPlayerProfile& GetCachedProfile() const { return CachedProfile; }

	UFUNCTION(BlueprintPure, Category = "DS|Backend")
	int32 GetClassRank(EDSHeroClass HeroClass) const { return CachedProfile.GetClassRank(HeroClass); }

	UFUNCTION(BlueprintPure, Category = "DS|Backend")
	EDSBackendType GetBackendType() const { return ActiveBackendType; }

	/** Token de sessão a enviar ao servidor UE no login (offline: PlayerId). Nunca logar. */
	FString GetSessionToken() const;

	/** Acesso direto à implementação (C++). */
	IDSBackendService* GetBackend() const;

	UPROPERTY(BlueprintAssignable, Category = "DS|Backend")
	FDSOnBackendAuthenticated OnAuthenticated;

	UPROPERTY(BlueprintAssignable, Category = "DS|Backend")
	FDSOnBackendProfileFetched OnProfileFetched;

	UPROPERTY(BlueprintAssignable, Category = "DS|Backend")
	FDSOnBackendRunResultsSubmitted OnRunResultsSubmitted;

private:
	void CreateBackend();
	UObject* CreateNullBackend();
	FString GetProfileSlotName() const;
	FString GetOrCreateDeviceId() const;
	bool CanSubmitResults() const;
	void SyncGameInstancePlayerId() const;

	void HandleAuthComplete(bool bSuccess, const FString& ErrorMessage);
	void HandleProfileComplete(bool bSuccess, const FDSPlayerProfile& Profile, const FString& ErrorMessage);
	void HandleSubmitComplete(bool bSuccess, const FString& ErrorMessage);

	UPROPERTY()
	TObjectPtr<UObject> BackendObject;

	UPROPERTY()
	FDSPlayerProfile CachedProfile;

	EDSBackendType ActiveBackendType = EDSBackendType::Null;
	bool bAuthInFlight = false;
	bool bProfileInFlight = false;
};
