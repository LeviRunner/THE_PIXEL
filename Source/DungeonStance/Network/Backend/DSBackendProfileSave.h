#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "Network/DSBackendTypes.h"
#include "DSBackendProfileSave.generated.h"

/** Perfil local: device id persistente (todos os backends) e progressão offline (UDSNullBackend). */
UCLASS()
class DUNGEONSTANCE_API UDSBackendProfileSave : public USaveGame
{
	GENERATED_BODY()

public:
	static constexpr int32 CurrentVersion = 1;

	UPROPERTY()
	int32 SaveVersion = CurrentVersion;

	/** GUID gerado na primeira execução; usado como device id no Nakama. */
	UPROPERTY()
	FString DeviceId;

	UPROPERTY()
	FString PlayerId;

	UPROPERTY()
	FString DisplayName;

	UPROPERTY()
	TArray<FDSClassProgress> Classes;

	/** Carrega o slot ou cria um novo (sem gravar). Nunca retorna nullptr. */
	static UDSBackendProfileSave* LoadOrCreate(const FString& SlotName);

	bool SaveToSlot(const FString& SlotName);

	/** Garante DeviceId preenchido; retorna true se gerou um novo. */
	bool EnsureDeviceId();
};
