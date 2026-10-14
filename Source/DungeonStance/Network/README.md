# Network & Matchmaking

Conexões, sessões e sincronização das instâncias do Dungeon Stance.

## Decisão (registrar aqui)

| Opção | Simulação da dungeon | Login / matchmaking / progresso | Situação |
| --- | --- | --- | --- |
| **A — recomendada para o lançamento** | UE5 Dedicated Server (ou Listen Server no Solo/coop entre amigos) | Nakama mantido como backend de conta, matchmaking, inventário e progressão (REST/WebSocket) | Aproveita o que já existe e testado em `Nakama_Legacy_Reference/` (inventário, progressão, espólio) |
| B | UE5 Dedicated Server | Online Subsystem nativo (Steam / EOS Sessions) | Menos peças, mas progressão e inventário precisam de outro backend |

**Status:** a decidir. O código já suporta as duas: sessões passam por `IOnlineSession` (Null hoje, Steam/EOS trocando só o `DefaultPlatformService`) e o backend de conta é uma interface (`IDSBackendService`) com implementação offline e Nakama.

## O que foi implementado

| Arquivo | O quê |
| --- | --- |
| `DSNetworkSettings.h/.cpp` | `UDSNetworkSettings` (Project Settings > Game > Dungeon Stance Network): backend (Null/Nakama), host/porta/SSL do Nakama, **nomes** das variáveis de ambiente das chaves, ids de RPC, timeouts, `MaxPartySize`, `LobbyMap`/`DungeonMap`, LAN com Null |
| `DSSessionTypes.h` | `FDSSessionInfo`, delegates do contrato, chaves custom da sessão (`DS_PARTY_SIZE`, `DS_GAME_MODE`) |
| `DSSessionSubsystem.h/.cpp` | `HostSession(EDSPartySize, bIsLAN)` (MaxPlayers = tamanho → abre o lobby `?listen?PartySize=N`), `FindSessions(EDSPartySize)` (filtra por tamanho + vaga; timeout próprio), `JoinSession(int32)` → `ClientTravel`, `DestroySession`, `StartSoloRun`, `TravelToDungeon` (servidor). Recria a sessão se já houver uma; destrói a sessão ao perder conexão |
| `DSGameSession.h/.cpp` | `ADSGameSession`: fixa `MaxPlayers` pelo `?PartySize=N` (ou pela sessão) → `ApproveLogin` recusa o 6º jogador / quem passar da party |
| `DSBackendTypes.h/.cpp` | `EDSBackendType`, `FDSClassProgress`, `FDSPlayerProfile`, delegates nativos, mapeamento `EDSHeroClass` ↔ ids do Nakama (`Wizard` = `"mage"`), validação de `FDSRunResult` |
| `DSBackendService.h` | `IDSBackendService`: `Authenticate`, `FetchProfile`, `SubmitRunResults`, `VerifyPlayerToken`, `GetSessionToken` |
| `DSBackendSubsystem.h/.cpp` | Escolhe a implementação pelas settings, device id persistente (um por instância de PIE), cache do perfil, `GetClassRank`, delegates Blueprint. `SubmitRunResults` é recusado em `NM_Client`. Preenche `UDSGameInstance::LocalPlayerId` após o login |
| `Backend/DSNullBackend` + `DSBackendProfileSave` | Offline: perfil/rank por classe em `USaveGame` (slot `DSBackendProfile`; **não** é o `DSLocalProfile` do Core). Aplica XP com `DSRunRules::GetRankXPToNext` (mesma regra de `progression.ts::applyXp`) |
| `Backend/DSNakamaBackend` + `DSNakamaJson` | REST: `POST /v2/account/authenticate/device?create=true` (Basic server key) → `GET /v2/account` (user id) → `POST /v2/rpc/load_all_progressions`. `SubmitRunResults` → `POST /v2/rpc/submit_run_results?http_key=…`. Todas as respostas são validadas (tipos, UUID, faixas de rank/XP, tamanho de token) |
| `DSMatchmakingSubsystem.h/.cpp` | Fila por tamanho (1–5) + grupo pré-formado: procura sessão com vagas ≥ grupo (prefere a mais cheia, depois menor ping) → entra; se não achar ou o join falhar → hospeda. Solo → `StartSoloRun`. Usa `IDSMatchmakerTicketService` se registrado |
| `Matchmaking/DSMatchmakerTicketService.h` | Interface + `FDSMatchmakingTicket` para o matchmaker do Nakama (TODO documentado no header) |

## Como configurar

1. **Mapas:** Project Settings > Game > Dungeon Stance Network → `LobbyMap` e `DungeonMap`.
2. **GameSession:** `ADSGameMode` (Core) precisa de `GameSessionClass = ADSGameSession::StaticClass()`.
3. **LAN / PIE (agora):** `[OnlineSubsystem] DefaultPlatformService=Null` (já está). Com Null a busca só funciona em LAN (`bForceLANWithNullSubsystem`). Em PIE com 2+ jogadores, use *Play As Standalone*/*Net Mode: Standalone* em janelas separadas para testar Host/Find/Join.
4. **Nakama (opção A):**
   - `Backend = Nakama`, `NakamaHost`, `NakamaPort` (7350), `bNakamaUseSSL` (obrigatório fora de dev).
   - Cliente: variável de ambiente `DS_NAKAMA_SERVER_KEY` (nome configurável). A server key não é secreta de verdade (vai no cliente), mas não fica no repositório.
   - Servidor dedicado: `DS_NAKAMA_HTTP_KEY` = `runtime.http_key` do Nakama. **Nunca** distribuir aos clientes.
   - Se a configuração for inválida, o subsistema loga erro e cai para o backend offline.

Exemplo `Config/DefaultGame.ini` (arquivo da base — pedir em `PENDENCIAS.md`):

```ini
[/Script/DungeonStance.DSNetworkSettings]
Backend=Null
NakamaHost=127.0.0.1
NakamaPort=7350
bNakamaUseSSL=False
LobbyMap=/Game/Maps/Lobby.Lobby
DungeonMap=/Game/Maps/Dungeon.Dungeon
```

## Fluxo de progresso (servidor autoritativo)

1. Cliente: `UDSBackendSubsystem::Authenticate()` → `FetchProfile()` (rank por classe para a UI).
2. Cliente envia `GetSessionToken()` ao servidor UE no login (ex.: opção de URL `?DSToken=`), **nunca** um PlayerId cru.
3. Servidor (`ADSGameMode::PreLogin/PostLogin`, Core): `VerifyPlayerToken(Token)` → guarda o PlayerId confiável no `ADSPlayerState`.
4. Fim da run (servidor): monta `TArray<FDSRunResult>` com esses ids e chama `SubmitRunResults`.

## Pendências / TODO

- **RPC `submit_run_results` no Nakama (novo):** servidor→servidor (`ctx.userId` vazio), payload `{"results":[{"user_id","class_id","waves_cleared","kills","victory","rank_xp","gold"}]}`; deve clampar `rank_xp`/`gold`, reaproveitar `grantMatchXp` e o espólio (`espolio_match.ts`) e ser idempotente (adicionar `run_id`).
- **Solo com Nakama:** a máquina do jogador não tem `http_key`, então o progresso solo não persiste. Opções: rodar solo também em servidor dedicado, ou um RPC de cliente com validação forte no Nakama.
- **Matchmaker por ticket (Nakama):** implementar `IDSMatchmakerTicketService` com o socket realtime (`WebSockets`), `party_matchmaker_add` para grupos e alocação de servidor dedicado no `matchmakerMatched` (Agones/Edgegap/fleet — a decidir).
- **Grupo pré-formado sem serviço de ticket:** hoje só o líder entra na fila; os membros precisam segui-lo (convite via sessão/presença — Steam/EOS) — `UDSPartySubsystem` não foi implementado.
- **Refresh de token** (`/v2/account/session/refresh`) e reautenticação automática em 401.
- **Token no login** trafega no canal da UE: habilitar criptografia do NetDriver (AESGCM PacketHandler) antes de produção.
- **Servidor dedicado** exige engine compilada do código-fonte (target `DungeonStanceServer`).

## O que muda em relação ao protótipo

- No Godot, a partida rodava **dentro do Nakama** (match handler em TypeScript, 10 Hz). Na UE5, a simulação passa para o servidor da Unreal; o Nakama (se mantido) deixa de simular combate.
- Matchmaker: `SQUAD_SIZE = 4` fixo → party de **1 a 5**, com fila por tamanho e opção de grupo pré-formado. Solo não precisa de matchmaking.
- Persistência (rank da classe, runas, inventário, espólio) continua sendo escrita só pelo servidor.

## Evolução para MMORPG (pós-lançamento)

Se o Dungeon Stance tiver aceitação, o caminho de evolução é: hub social persistente → mundo aberto coop por instância (o protótipo já tem `world_*.ts` com 28 skills de coleta/combate/suporte) → shards maiores. Pontos de atenção: replicação em escala (Iris / Replication Graph), servidores por zona, e MassEntity para multidões de NPCs.
