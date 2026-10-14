#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "Network/DSBackendService.h"
#include "DSNullBackend.generated.h"

class UDSBackendProfileSave;

/**
 * Backend offline: perfil e rank por classe gravados em USaveGame local.
 * Em sessões LAN/listen, o host só persiste o resultado do PRÓPRIO jogador local
 * (os outros jogadores mantêm o perfil em suas máquinas; não há fonte confiável offline).
 */
UCLASS()
class DUNGEONSTANCE_API UDSNullBackend : public UObject, public IDSBackendService
{
	GENERATED_BODY()

public:
	void Configure(const FString& InSlotName);

	// IDSBackendService
	virtual FName GetBackendName() const override { return FName(TEXT("Null")); }
	virtual void Authenticate(const FString& DeviceId, FDSOnBackendAuthComplete OnComplete) override;
	virtual void FetchProfile(FDSOnBackendProfileComplete OnComplete) override;
	virtual void SubmitRunResults(const TArray<FDSRunResult>& Results, FDSOnBackendSubmitComplete OnComplete) override;
	virtual void VerifyPlayerToken(const FString& SessionToken, FDSOnBackendTokenVerified OnComplete) override;
	virtual bool IsAuthenticated() const override { return bAuthenticated; }
	virtual FString GetPlayerId() const override { return PlayerId; }
	/** Offline não há token: devolve o PlayerId local (VerifyPlayerToken só checa a sintaxe). */
	virtual FString GetSessionToken() const override { return PlayerId; }

private:
	FDSPlayerProfile BuildProfile() const;
	static FDSClassProgress MakeDefaultProgress(EDSHeroClass HeroClass);
	static FDSClassProgress ApplyRunResult(const FDSClassProgress& Current, const FDSRunResult& Result);

	UPROPERTY()
	TObjectPtr<UDSBackendProfileSave> SaveData;

	FString SlotName;
	FString PlayerId;
	bool bAuthenticated = false;
};
