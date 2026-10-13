# Core Framework (C++)

Inicialização do jogo, sessão instanciada de **1 a 5 jogadores** e regras fundamentais da run do modo **Dungeon Stance**.

## Classes esperadas

| Classe | Base UE5 | Responsabilidade |
| --- | --- | --- |
| `ADSGameMode` | `AGameModeBase` | Só no servidor: ciclo da partida (`Waiting → Running → Ended`), ondas, timeouts, fim de run. Equivale a `squad_match.ts`. |
| `ADSGameState` | `AGameStateBase` | Replicado: onda atual, nível/XP do time, fase, timer da escolha de cartas. |
| `ADSPlayerState` | `APlayerState` | Replicado: classe, rank, caminho (árvore), cartas, kills, `UAbilitySystemComponent` do jogador. |
| `ADSPlayerController` | `APlayerController` | Input (Enhanced Input), lock-on de alvo, RPCs de escolha de carta. |
| `UDSGameInstance` | `UGameInstance` | Perfil local, ponte com login/matchmaking (ver `Network/`). |
| `UDSRunRulesSubsystem` | `UWorldSubsystem` | Fórmulas puras (escala de ondas, XP, limites) testáveis sem rede. |

> Não existe `USecuritySubsystem` na engine. Validação anti-trapaça fica no próprio servidor autoritativo (GameMode + GAS no servidor).

## Diretrizes

- Nada de lógica visual aqui: só estado e regras de sessão.
- **Servidor autoritativo de verdade.** No protótipo, o cliente era dono do próprio HP e informava o dano dos golpes (com teto por classe). Na UE5 o dano e o HP são calculados no servidor via GAS; o teto de dano por golpe deixa de ser necessário.
- Tamanho da party não fica fixo no código: constante `MaxPartySize = 5`, e todo cálculo que dependia de "4" (posições de spawn, escala de inimigos) usa o número real de jogadores.
- Fórmulas: ver [`ESPEC_REGRAS_DUNGEON_STANCE.md`](../../../docs/ESPEC_REGRAS_DUNGEON_STANCE.md).

## Arquivos de módulo

Ao criar o projeto C++ pela UE5 (`DungeonStance.uproject`), a engine gera `DungeonStance.Build.cs`, `DungeonStance.Target.cs` e `DungeonStanceEditor.Target.cs`. Estas subpastas (`Core/`, `Gameplay/`, `UI/`, `Network/`) ficam dentro do módulo `DungeonStance` — não são módulos separados.
