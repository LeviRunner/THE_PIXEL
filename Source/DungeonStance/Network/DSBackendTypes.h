#pragma once

#include "CoreMinimal.h"
#include "Core/DSTypes.h"
#include "DSBackendTypes.generated.h"

// Tipos do backend de conta/progressão (Network). Usados por UI (perfil/rank) e Core (fim de run).

/** Implementação de backend escolhida em UDSNetworkSettings. */
UENUM(BlueprintType)
enum class EDSBackendType : uint8
{
	/** Offline: perfil local em USaveGame. Padrão para PIE/LAN. */
	Null	UMETA(DisplayName = "Offline (perfil local)"),
	/** Nakama via REST (opção A do README). */
	Nakama	UMETA(DisplayName = "Nakama (REST)")
};

/** Progressão permanente de uma classe (espelha `player_progression` do Nakama). */
USTRUCT(BlueprintType)
struct DUNGEONSTANCE_API FDSClassProgress
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "DS|Backend")
	EDSHeroClass HeroClass = EDSHeroClass::None;

	UPROPERTY(BlueprintReadOnly, Category = "DS|Backend")
	int32 Rank = 1;

	/** XP acumulado rumo ao próximo rank (zera ao subir). */
	UPROPERTY(BlueprintReadOnly, Category = "DS|Backend")
	int32 XP = 0;

	UPROPERTY(BlueprintReadOnly, Category = "DS|Backend")
	int32 XPToNext = 0;

	UPROPERTY(BlueprintReadOnly, Category = "DS|Backend")
	int32 MatchesPlayed = 0;

	UPROPERTY(BlueprintReadOnly, Category = "DS|Backend")
	int32 Victories = 0;
};

/** Perfil da conta: identidade + progressão por classe. */
USTRUCT(BlueprintType)
struct DUNGEONSTANCE_API FDSPlayerProfile
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "DS|Backend")
	FString PlayerId;

	UPROPERTY(BlueprintReadOnly, Category = "DS|Backend")
	FString DisplayName;

	UPROPERTY(BlueprintReadOnly, Category = "DS|Backend")
	TArray<FDSClassProgress> Classes;

	/** Retorna a progressão da classe ou nullptr. */
	const FDSClassProgress* FindClass(EDSHeroClass HeroClass) const;

	/** Rank da classe (1 se a classe ainda não tem registro). */
	int32 GetClassRank(EDSHeroClass HeroClass) const;
};

// Callbacks nativos das implementações de IDSBackendService (sempre chamados no game thread).
DECLARE_DELEGATE_TwoParams(FDSOnBackendAuthComplete, bool /*bSuccess*/, const FString& /*ErrorMessage*/);
DECLARE_DELEGATE_ThreeParams(FDSOnBackendProfileComplete, bool /*bSuccess*/, const FDSPlayerProfile& /*Profile*/, const FString& /*ErrorMessage*/);
DECLARE_DELEGATE_TwoParams(FDSOnBackendSubmitComplete, bool /*bSuccess*/, const FString& /*ErrorMessage*/);
DECLARE_DELEGATE_ThreeParams(FDSOnBackendTokenVerified, bool /*bSuccess*/, const FString& /*PlayerId*/, const FString& /*ErrorMessage*/);

namespace DSBackendIds
{
	/** Ids de classe usados pelo backend (classes.ts): "barbarian", "demon_hunter", "mage"... */
	DUNGEONSTANCE_API FString HeroClassToBackendId(EDSHeroClass HeroClass);

	/** Inverso de HeroClassToBackendId. Retorna None para ids desconhecidos. */
	DUNGEONSTANCE_API EDSHeroClass HeroClassFromBackendId(const FString& BackendId);

	/** Validação defensiva de um resultado de run antes de persistir. */
	DUNGEONSTANCE_API bool ValidateRunResult(const FDSRunResult& Result, FString& OutReason);

	/** Ids de jogador/dispositivo: 1..128 chars, só [A-Za-z0-9-_]. */
	DUNGEONSTANCE_API bool IsSafeIdentifier(const FString& Value, int32 MinLength = 1, int32 MaxLength = 128);
}
