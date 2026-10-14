#pragma once

#include "CoreMinimal.h"
#include "Core/DSTypes.h"

class APawn;
class APlayerState;
class ADSPlayerState;
class UDSAbilitySystemComponent;
class UDSAttributeSet;

/**
 * Ponto único de acesso da área Gameplay ao ADSPlayerState (Core).
 * Se o Core usar nomes de getters diferentes, só este arquivo (.cpp) precisa mudar.
 *
 * Getters assumidos no ADSPlayerState (contrato): GetAbilitySystemComponent(), GetAttributeSet(),
 * GetHeroClass(), GetStance(), GetClassRank(), GetOwnedCards(), GetPendingCardPicks(), GetCurrentCardChoices().
 */
namespace DSPlayerStateBridge
{
	DUNGEONSTANCE_API ADSPlayerState* AsDSPlayerState(APlayerState* PlayerState);
	DUNGEONSTANCE_API UDSAbilitySystemComponent* GetASC(const ADSPlayerState* PlayerState);
	DUNGEONSTANCE_API UDSAttributeSet* GetAttributeSet(const ADSPlayerState* PlayerState);
	DUNGEONSTANCE_API EDSHeroClass GetHeroClass(const ADSPlayerState* PlayerState);
	DUNGEONSTANCE_API EDSStance GetStance(const ADSPlayerState* PlayerState);
	DUNGEONSTANCE_API int32 GetClassRank(const ADSPlayerState* PlayerState);
	DUNGEONSTANCE_API int32 GetPendingCardPicks(const ADSPlayerState* PlayerState);
	DUNGEONSTANCE_API TArray<FName> GetCurrentCardChoices(const ADSPlayerState* PlayerState);
	/** OwnedCards convertido para o mapa de acúmulos usado pelas regras de cards.ts. */
	DUNGEONSTANCE_API TMap<FName, int32> GetCardStacks(const ADSPlayerState* PlayerState);
	DUNGEONSTANCE_API APawn* GetPawn(const ADSPlayerState* PlayerState);

	/** Utilitário puro: TArray<FDSOwnedCard> → mapa CardId → acúmulos. */
	DUNGEONSTANCE_API TMap<FName, int32> ToStackMap(const TArray<FDSOwnedCard>& OwnedCards);
}
