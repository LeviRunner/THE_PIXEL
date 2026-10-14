#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "Core/DSTypes.h"
#include "Network/DSBackendTypes.h"
#include "DSNetworkSettings.generated.h"

class UWorld;

/**
 * Configuração de rede/backend (Project Settings > Game > Dungeon Stance Network).
 * Gravado em Config/DefaultGame.ini, seção [/Script/DungeonStance.DSNetworkSettings].
 *
 * SEGREDOS: nenhum segredo fica aqui. As chaves do Nakama são lidas de variáveis de
 * ambiente cujo NOME está configurado abaixo (NakamaServerKeyEnvVar / NakamaHttpKeyEnvVar).
 */
UCLASS(Config = Game, DefaultConfig, meta = (DisplayName = "Dungeon Stance Network"))
class DUNGEONSTANCE_API UDSNetworkSettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	UDSNetworkSettings();

	static const UDSNetworkSettings* Get() { return GetDefault<UDSNetworkSettings>(); }

	// --- Backend ---------------------------------------------------------

	/** Backend de conta/progressão. Decisão A/B pendente (ver Network/README.md). */
	UPROPERTY(Config, EditAnywhere, Category = "Backend")
	EDSBackendType Backend = EDSBackendType::Null;

	/** Slot do USaveGame do perfil offline (Null) e do device id. NÃO usar "DSLocalProfile" (slot do UDSGameInstance). */
	UPROPERTY(Config, EditAnywhere, Category = "Backend")
	FString LocalProfileSlotName = TEXT("DSBackendProfile");

	/** Timeout das requisições HTTP ao backend, em segundos. */
	UPROPERTY(Config, EditAnywhere, Category = "Backend", meta = (ClampMin = "1.0", ClampMax = "120.0"))
	float HttpTimeoutSeconds = 10.f;

	UPROPERTY(Config, EditAnywhere, Category = "Backend|Nakama")
	FString NakamaHost = TEXT("127.0.0.1");

	UPROPERTY(Config, EditAnywhere, Category = "Backend|Nakama", meta = (ClampMin = "1", ClampMax = "65535"))
	int32 NakamaPort = 7350;

	UPROPERTY(Config, EditAnywhere, Category = "Backend|Nakama")
	bool bNakamaUseSSL = false;

	/** Nome da variável de ambiente com a server key (usada pelo cliente para autenticar). */
	UPROPERTY(Config, EditAnywhere, Category = "Backend|Nakama")
	FString NakamaServerKeyEnvVar = TEXT("DS_NAKAMA_SERVER_KEY");

	/**
	 * Nome da variável de ambiente com a runtime http_key (chamadas servidor→servidor).
	 * Só deve existir na máquina do servidor dedicado; nunca distribuir aos clientes.
	 */
	UPROPERTY(Config, EditAnywhere, Category = "Backend|Nakama")
	FString NakamaHttpKeyEnvVar = TEXT("DS_NAKAMA_HTTP_KEY");

	/** RPC de leitura do perfil (já existe no protótipo: progression.ts). */
	UPROPERTY(Config, EditAnywhere, Category = "Backend|Nakama")
	FString NakamaProfileRpcId = TEXT("load_all_progressions");

	/** RPC servidor→servidor de fim de run (A CRIAR no Nakama; ver README). */
	UPROPERTY(Config, EditAnywhere, Category = "Backend|Nakama")
	FString NakamaSubmitRunRpcId = TEXT("submit_run_results");

	// --- Sessões ---------------------------------------------------------

	/** Teto de jogadores por sessão (1..5). */
	UPROPERTY(Config, EditAnywhere, Category = "Session", meta = (ClampMin = "1", ClampMax = "5"))
	int32 MaxPartySize = DSConstants::MaxPartySize;

	/** Mapa de lobby (host abre com ?listen; clientes entram via JoinSession). */
	UPROPERTY(Config, EditAnywhere, Category = "Session", meta = (AllowedClasses = "/Script/Engine.World"))
	TSoftObjectPtr<UWorld> LobbyMap;

	/** Mapa da dungeon (StartSoloRun / TravelToDungeon). */
	UPROPERTY(Config, EditAnywhere, Category = "Session", meta = (AllowedClasses = "/Script/Engine.World"))
	TSoftObjectPtr<UWorld> DungeonMap;

	/** Solo abre a dungeon com ?listen (permite amigos entrarem depois). */
	UPROPERTY(Config, EditAnywhere, Category = "Session")
	bool bSoloUsesListenServer = false;

	/** Com OnlineSubsystemNull a busca só funciona em LAN; força LAN nesse caso. */
	UPROPERTY(Config, EditAnywhere, Category = "Session")
	bool bForceLANWithNullSubsystem = true;

	/** Usa presença/lobbies (Steam/EOS). Ignorado em servidor dedicado. */
	UPROPERTY(Config, EditAnywhere, Category = "Session")
	bool bUsePresence = true;

	UPROPERTY(Config, EditAnywhere, Category = "Session", meta = (ClampMin = "1", ClampMax = "1000"))
	int32 MaxSearchResults = 100;

	/** Se a busca não responder neste tempo, é tratada como vazia. */
	UPROPERTY(Config, EditAnywhere, Category = "Session", meta = (ClampMin = "1.0", ClampMax = "120.0"))
	float FindSessionsTimeoutSeconds = 15.f;

	/** Valor anunciado no setting de modo de jogo (protótipo: "debandada"). */
	UPROPERTY(Config, EditAnywhere, Category = "Session")
	FString GameModeTag = TEXT("debandada");

	/** Número de jogadores do tamanho de party, limitado a [1, MaxPartySize]. */
	int32 ClampPartySize(EDSPartySize PartySize) const;

	/** Nome longo do pacote do mapa ("/Game/Maps/Lobby") ou vazio se não configurado. */
	FString GetLobbyMapName() const;
	FString GetDungeonMapName() const;
};
