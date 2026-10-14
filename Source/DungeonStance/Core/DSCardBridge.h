#pragma once

#include "CoreMinimal.h"
#include "Core/DSTypes.h"

class ADSPlayerState;
struct FRandomStream;

/**
 * Ponto ÚNICO onde o Core chama UDSCardLibrary (área Gameplay).
 *
 * As assinaturas da biblioteca não estavam fechadas no CONTRATOS.md quando o
 * Core foi escrito; se a Gameplay publicar outra assinatura, só DSCardBridge.cpp
 * precisa mudar. Suposição atual (ver README do Core):
 *   static TArray<FName> UDSCardLibrary::RollCardChoices(EDSStance, const TArray<FDSOwnedCard>&, int32 NumChoices, FRandomStream&);
 *   static bool UDSCardLibrary::ValidateCardPick(FName CardId, EDSStance, const TArray<FDSOwnedCard>&, int32 PendingPicks, FString& OutReason);
 *   static bool UDSCardLibrary::ApplyCardToPlayer(ADSPlayerState*, FName CardId);
 */
namespace DSCardBridge
{
	/** Sorteia até NumChoices cartas da stance do jogador. Vazio = árvore inteira no máximo. */
	TArray<FName> RollChoices(const ADSPlayerState& Player, int32 NumChoices, FRandomStream& Random);

	/** Valida a escolha no servidor (árvore certa, carta sorteável, escolha disponível). */
	bool ValidatePick(const ADSPlayerState& Player, FName CardId, FString& OutReason);

	/** Aplica os efeitos da carta (GEs/habilidades) no jogador. */
	bool ApplyToPlayer(ADSPlayerState& Player, FName CardId);
}
