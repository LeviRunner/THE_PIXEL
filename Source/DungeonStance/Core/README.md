# Core Framework (C++)

Inicialização do jogo, sessão instanciada de **1 a 5 jogadores** e regras fundamentais da run do modo **Dungeon Stance**.
Fonte de verdade das regras: `Nakama_Legacy_Reference/src/` (squad_match.ts, squad_logic.ts, squad_state.ts, progression.ts, loot.ts, bestiary.ts).

## Implementado

| Arquivo | Classe / API | O que faz |
| --- | --- | --- |
| `DSRunRules.h/.cpp` | namespace `DSRunRules` | Todas as fórmulas puras do contrato + auxiliares (`AddTeamXP`, `GetPendingPicks`, `GetRankHealth/DamageMultiplier`, `GetEnemyCombatXP`, `GetEnemyBaseHealth/DPS`, `GetEnemyAverageGold`, `GetScaledWaveEnemyCount`, `GetWaveSeed`). Tempos do ciclo (30/4/20+2/1/150/10/15 s) como `constexpr`. |
| `Tests/DSRunRulesTests.cpp` | Automation Tests `DungeonStance.Core.RunRules.*` | 12 testes com os valores dos testes TS (squad_logic, progression, bestiary, espolio). |
| `DSRunRulesSubsystem.h/.cpp` | `UDSRunRulesSubsystem` (`UWorldSubsystem`) | Wrappers `BlueprintPure` das fórmulas + RNG da run: `InitializeSeed`, `GetRunRandom`, `MakeWaveRandom(Wave)` (seed + onda × 7919). |
| `DSGameMode.h/.cpp` | `ADSGameMode` | Ciclo completo da partida (ver diagrama abaixo), spawn de heróis em anel, spawn escalado de inimigos, XP do time, escolhas de carta, `FDSRunResult`, envio ao backend. Só `FTimerHandle`, sem Tick. |
| `DSArenaTypes.h` | `FDSEnemySpawnEntry`, `FDSBossSpawnEntry`, `FDSEnemySpawnRing` | Configuração da arena (EditDefaultsOnly no BP do GameMode). |
| `DSWaveSpawnHelpers.h/.cpp` | namespace | Sorteio ponderado de inimigo, escolha do chefe, ponto no anel 18–30 m projetado no navmesh. |
| `DSGameState.h/.cpp` | `ADSGameState` | Estado replicado do contrato + delegates; setters `Server*` só para o GameMode. |
| `DSPlayerState.h/.cpp` | `ADSPlayerState` | ASC (`Mixed`) + `UDSAttributeSet`, `NetUpdateFrequency` 100. Campos do contrato + `RunGold` e `FDSRunResult` replicado (`OnRunResultChanged`). |
| `DSPlayerController.h/.cpp` | `ADSPlayerController` | `Server_SelectHero`, `Server_SetReady`, `Server_PickCard` validados no servidor; `Client_CardPickRejected` → `OnCardPickRejected`. Envia o herói salvo no `UDSGameInstance` ao entrar. |
| `DSGameInstance.h/.cpp` | `UDSGameInstance`, `UDSLocalProfileSave` | Perfil local (`LocalPlayerId`, `LastClass`, `LastStance`, `DesiredPartySize`) salvo em slot. |
| `DSCardBridge.h/.cpp` | namespace | Único ponto que chama `UDSCardLibrary` (Gameplay). |
| `DSBackendBridge.h/.cpp` | namespace | Único ponto que chama `UDSBackendSubsystem::SubmitRunResults` (Network); compila sem o backend (só loga). |

## Ciclo da partida (`ADSGameMode`)

```
WaitingForPlayers ─(todos entraram e prontos | 30 s com alguém | 30 s vazio → fecha)
  → BetweenWaves 4 s → WaveInProgress (timeout 150 s)
  → onda limpa: CardPick 20+2 s (se alguém tem escolha) | BetweenWaves 4 s
      (todos escolheram → próxima onda em 1 s, nunca antes de 4 s)
  → onda 10 limpa: Victory | todos caídos: Defeat → tela final 10 s → Ended → CloseMatch
Ninguém conectado por 15 s → Ended sem recompensa.
```

- Opções de URL do mapa: `?PartySize=N` (jogadores esperados), `?Seed=N`, `?Torment=N`, `?PlayerId=...` (por jogador, no login).
- Inimigos por onda: `round((6 + 3·onda) × (0,4 + 0,15·N))` (+1 chefe nas ondas 5 e 10). Vida/dano × escala de onda e rank médio; chefe também × `(0,5 + 0,125·N)`.
- XP do time por inimigo: `round(4 + 0,05 × vida base)` (vida do ASC dividida pelo multiplicador aplicado). Cada nível ganho dá 1 escolha de carta a cada jogador; árvore no máximo → escolha consumida em silêncio.
- Ouro: `4 + 2·onda + kills na onda` por onda limpa; +60 na vitória. XP de rank: `GetEndOfRunRankXP`.
- Heróis caídos levantam no início de cada onda (`ReviveHero`, BlueprintNativeEvent).

## Configurar uma arena

Criar um Blueprint filho de `ADSGameMode` por arena e preencher: `ArenaIndex`, `RegularEnemies` (peso, onda mínima, elite), `Bosses` (onda 5 e 10), `EnemyCharacterClass`, `HeroPawnClasses` (uma por `EDSHeroClass`). O mapa precisa de um `PlayerStart` (centro do anel dos heróis) e de NavMesh.

## Diretrizes

- Nada de lógica visual aqui: só estado e regras de sessão.
- **Servidor autoritativo de verdade.** Dano e HP no servidor via GAS; o teto de dano por golpe do protótipo não existe aqui.
- Tamanho da party não é fixo: `DSConstants::MaxPartySize = 5`, e todo cálculo que dependia de "4" usa o número real de jogadores.
- Fórmulas: ver [`ESPEC_REGRAS_DUNGEON_STANCE.md`](../../../docs/ESPEC_REGRAS_DUNGEON_STANCE.md).
- Não existe `USecuritySubsystem` na engine. Validação anti-trapaça fica no servidor (GameMode + RPCs validadas + GAS).
