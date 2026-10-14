#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "Core/DSTypes.h"
#include "DSMatchmakerTicketService.generated.h"

/** Pedido de matchmaking (mesmas propriedades do ticket do protótipo: matchmaker.ts). */
USTRUCT(BlueprintType)
struct DUNGEONSTANCE_API FDSMatchmakingTicket
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DS|Matchmaking")
	EDSPartySize PartySize = EDSPartySize::Squad;

	/** Jogadores que já chegam juntos (grupo pré-formado). 1 = sozinho na fila. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DS|Matchmaking", meta = (ClampMin = "1", ClampMax = "5"))
	int32 GroupSize = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DS|Matchmaking")
	EDSHeroClass HeroClass = EDSHeroClass::None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DS|Matchmaking")
	EDSStance Stance = EDSStance::None;

	/** Informativo: o servidor do backend sempre sobrescreve com o rank gravado. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DS|Matchmaking")
	int32 ClassRank = 1;

	/** ±ranks aceitos (protótipo: 3, +2 a cada 5 s, teto 15). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DS|Matchmaking")
	int32 RankTolerance = 3;

	/** Arena preferida (vazio = aleatória). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DS|Matchmaking")
	FString Arena;
};

/** bSuccess + endereço do servidor UE a conectar ("ip:porta"), ou mensagem de erro. */
DECLARE_DELEGATE_TwoParams(FDSOnTicketMatched, bool /*bSuccess*/, const FString& /*ConnectStringOrError*/);

UINTERFACE(MinimalAPI, meta = (CannotImplementInterfaceInBlueprint))
class UDSMatchmakerTicketService : public UInterface
{
	GENERATED_BODY()
};

/**
 * Matchmaking por ticket (serviço externo). Quando registrado no UDSMatchmakingSubsystem,
 * substitui o fluxo "procura sessão → entra; senão hospeda".
 *
 * TODO(Nakama, opção A): implementar UDSNakamaMatchmaker com o socket realtime
 * (WebSockets: ws(s)://host:porta/ws?token=...):
 *   1. `matchmaker_add` com min/max = PartySize, count_multiple = 1, string_properties
 *      {selected_class, skill_tree, mode, arena} e numeric_properties {rank_tolerance};
 *      grupo pré-formado usa party_matchmaker_add (Nakama Parties).
 *   2. No Nakama, `registerMatchmakerMatched` deixa de criar `squad_match`: aloca um servidor
 *      dedicado UE (fleet/Agones/Edgegap — a decidir) e devolve "ip:porta" aos membros.
 *   3. `matchmaker_matched` no cliente → OnMatched(true, "ip:porta") → ClientTravel.
 * Com Steam/EOS (opção B), a implementação usaria a API de matchmaking do próprio serviço.
 */
class DUNGEONSTANCE_API IDSMatchmakerTicketService
{
	GENERATED_BODY()

public:
	/** Retorna false se não conseguiu nem enfileirar (OnMatched não será chamado). */
	virtual bool SubmitTicket(const FDSMatchmakingTicket& Ticket, FDSOnTicketMatched OnMatched) = 0;

	virtual void CancelTicket() = 0;
};
