# Checklist de Fases de Implementação — Dungeon Stance (UE 5.8)

Acompanhamento fase a fase da implementação. Marque `[x]` quando o item estiver **compilando, testado e na `main`**.
O que ainda está faltando ou depende de decisão fica em [PENDENCIAS.md](PENDENCIAS.md). Plano completo: [docs/PLANO_DUNGEON_STANCE_UE5.md](docs/PLANO_DUNGEON_STANCE_UE5.md).

Legenda das branches: `feat/projeto-base` · `feat/core` · `feat/gameplay` · `feat/network` · `feat/ui`.

---

## Fase 0 — Fundação

- [x] Arquivar protótipo (`*_Legacy_Reference`) e criar estrutura de pastas
- [x] Extrair regras do servidor Nakama para `docs/ESPEC_REGRAS_DUNGEON_STANCE.md`
- [x] Criar `DungeonStance.uproject` (UE 5.8), módulo C++, targets Game/Editor/Server — `feat/projeto-base`
- [x] Habilitar plugins: Gameplay Abilities, Enhanced Input, CommonUI, StateTree, OnlineSubsystem (Null), WebSockets
- [x] Tipos compartilhados (`DSTypes.h`), tags nativas (`DSGameplayTags`) e contratos entre áreas (`CONTRATOS.md`)
- [ ] Instalar Visual Studio 2022 (workload "Desenvolvimento de jogos com C++") e compilar o editor sem erros
- [ ] Abrir o projeto no editor, gerar os arquivos de projeto e rodar os Automation Tests (Session Frontend → Automation → `DungeonStance`)
- [ ] Decidir backend de conta (Nakama × EOS/Steam)
- [ ] Mesclar `feat/core`, `feat/gameplay`, `feat/network` e `feat/ui` na `main` via Pull Request

## Fase 1 — Vertical slice (1 classe, 1 arena, 1 a 5 jogadores)

### Core
- [ ] Fórmulas puras portadas (`DSRunRules`) com Automation Tests passando
- [ ] `ADSGameMode`: espera de jogadores, 10 ondas, chefes 5/10, pausas, timeout de onda, vitória/derrota
- [ ] `ADSGameState` / `ADSPlayerState` replicando onda, fase, XP do time, classe, Stance, cartas
- [ ] Escala de inimigos pelo número real de jogadores (1–5)
- [ ] Spawn de heróis em anel por N jogadores

### Gameplay
- [ ] ASC + Attribute Set + execução de dano (crítico, armadura, escudo)
- [ ] Ataque básico, esquiva (i-frames), sprint com estamina
- [ ] Skill do Bárbaro — Redemoinho
- [ ] Lock-on de alvo (mouse e R3)
- [ ] 3 arquétipos de inimigo (minion, runner, shooter) + 1 chefe, com IA melee e ranged
- [ ] 1 Stance completa (árvore de cartas, sorteio e validação no servidor)
- [ ] Data Assets do Bárbaro, dos 4 inimigos e das cartas da Stance criados no editor

### UI
- [ ] HUD da party de 1 a 5 (vida, escudo, recurso, status, caído)
- [ ] Barra de casts com recarga
- [ ] Indicador de onda/chefe e XP do time
- [ ] Escolha de cartas (20 s)
- [ ] Lobby mínimo (classe, Stance, tamanho da party, pronto) e tela de fim de run
- [ ] Blueprints `WBP_*` montados sobre as classes C++

### Network
- [ ] Sessão Listen Server com OnlineSubsystem Null (LAN/PIE)
- [ ] Solo sem matchmaking
- [ ] Backend Null (perfil local em SaveGame)

### Testes da fase
- [ ] Automation Tests das fórmulas e das regras de cartas passando
- [ ] PIE com 5 clientes (Net Mode: Play As Listen Server) jogando uma run completa
- [ ] Teste com latência simulada (`Net PktLag=100`, `PktLoss=2`)

## Fase 2 — Conteúdo do lançamento

- [ ] 9 classes com skills, recursos (Fúria/Fé/…) e Data Assets
- [ ] 4 Stances completas (Crítico, Arcana, Defesa, Apoio) — todas as cartas e ramos
- [ ] 8 habilidades de apoio (Q/E/R/F, 5ª substitui uma)
- [ ] Todos os status (sangramento, queimadura, veneno, marca, lentidão, atordoamento, enfraquecer, exposto, provocação)
- [ ] Bestiário completo + comportamentos de IA (melee, swarm, charger, blinker, kiter, ranged, passive, brood)
- [ ] 3 arenas e níveis de Tormento
- [ ] Progressão permanente: rank até 60, runas nos ranks 5 e 12
- [ ] Loot, Baú do Esquadrão, Espólio e inventário (Cofre 120, Mochila 28, equipamento 12, Cinto 4)
- [ ] Backend escolhido integrado (Nakama REST ou EOS/Steam) — progresso gravado só pelo servidor
- [ ] Assets 3D finais importados de `_Export_3dsMax_Final/`, animações e VFX
- [ ] Ficha do herói e inventário na UI

## Fase 3 — Lançamento do Dungeon Stance

- [ ] Engine compilada a partir do código-fonte para gerar o target `DungeonStanceServer`
- [ ] Servidores dedicados hospedados (ex.: AWS GameLift, Edgegap ou máquina própria)
- [ ] Matchmaking por tamanho de party (1–5) e grupos pré-formados
- [ ] Balanceamento por playtest da escala de 1 a 5 jogadores
- [ ] Métricas de retenção, telemetria e relatório de crashes
- [ ] Empacotamento (Shipping), loja (Steam) e pipeline de build/CI

## Fase 4 — Evolução para MMORPG (se houver aceitação)

- [ ] Hub social persistente entre dungeons
- [ ] Mundo aberto coop por instância (28 skills de `world_*.ts`)
- [ ] Crafting e economia (`docs/LISTA_MODELOS.MD`)
- [ ] Escala de rede: Iris / Replication Graph, servidores por zona, MassEntity
