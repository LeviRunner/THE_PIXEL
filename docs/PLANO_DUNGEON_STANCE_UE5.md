# Plano de Execução — Dungeon Stance (Unreal Engine 5)

**Jogo de lançamento:** *Dungeon Stance*, um roguelike cooperativo de ação em dungeons instanciadas para **1 a 5 jogadores** (Solo, Dupla, Trio, Esquadrão de 4, Grupo de 5).
**Visão de longo prazo:** se o Dungeon Stance tiver aceitação da comunidade, evoluir para um **MMORPG**.
**Stack:** Unreal Engine 5 (C++ + Blueprints + Gameplay Ability System) · 3ds Max para os assets · backend de conta a decidir (Nakama ou EOS/Steam).

Protótipo anterior (Godot 4 + Nakama, modo "Debandada", 4 jogadores) arquivado em `Godot_Legacy_Reference/` e `Nakama_Legacy_Reference/` para consulta.

---

## 1. Decisão técnica: C++ + Blueprints + GAS (sem AngelScript)

- **C++** para a arquitetura central: GameMode, GameState, PlayerState, replicação/RPCs, Attribute Sets e Gameplay Abilities do GAS.
- **Blueprints** para UI, animação, VFX, ajustes de habilidades e iteração rápida de design.
- **AngelScript evitado:** na UE5 ele exige um fork da engine mantido por terceiros, que pode atrasar a cada versão (5.x → 5.y), e tem pouca documentação junto do framework de gameplay da Unreal. A referência oficial para projetos modulares (Lyra Starter Game) usa só C++ e Blueprints.

## 2. O que é o Dungeon Stance

O modo nasce da Debandada do protótipo, que já está jogável e testada, e ganha identidade própria:

| Pilar | Descrição | Origem no protótipo |
| --- | --- | --- |
| **Dungeon** | Instância por grupo, 10 ondas, chefes nas ondas 5 e 10, 3 arenas iniciais e níveis de Tormento | `squad_match.ts`, `bestiary.ts` |
| **Stance** | Antes da run, cada herói escolhe sua Stance: Crítico, Arcana, Defesa ou Apoio. A cada nível do time, escolhe uma carta da árvore da sua Stance | as 4 árvores de cartas (`cards_*.ts`) |
| **Cast Framework** | Todo cast (ataque, skill da classe, esquiva, até 4 habilidades de apoio, procs) é uma Gameplay Ability com custo, recarga e alvo, validada no servidor | `abilities.ts`, `squad_procs.ts`, `classes.ts` |
| **Party de 1 a 5** | A dificuldade escala pelo número real de jogadores; a composição da party (Stances diferentes) gera sinergias | novo (protótipo era fixo em 4) |
| **Progressão** | 9 classes, rank permanente até 60, runas nos ranks 5 e 12, Espólio e inventário compartilhado | `progression.ts`, `runes.ts`, `loot.ts`, `inventory*.ts` |

Regras e fórmulas detalhadas: [`ESPEC_REGRAS_DUNGEON_STANCE.md`](ESPEC_REGRAS_DUNGEON_STANCE.md).

## 3. Diretrizes de migração (Godot/Nakama → UE5)

1. **Regras de negócio:** a fonte é `Nakama_Legacy_Reference/src/` (servidor autoritativo em TypeScript) e os testes de `Nakama_Legacy_Reference/test/`, **não** o GDScript. Portar fórmula por fórmula para `UDSRunRulesSubsystem` (C++ puro) e reescrever os testes como *Automation Tests* da UE5.
2. **Rede:** a simulação da dungeon sai do Nakama e vai para o servidor da Unreal (Dedicated Server; Listen Server aceito para Solo e coop entre amigos). Diferente do protótipo, **HP e dano passam a ser calculados no servidor** (GAS).
3. **Backend:** Nakama pode continuar só como conta, matchmaking, inventário e progressão. Decisão registrada em `Source/DungeonStance/Network/README.md`.
4. **Arte:** `/arts` **não é modificada** (concepts e trabalho em andamento no 3ds Max). Assets validados vão para `_Export_3dsMax_Final/`. O visual muda de 2.5D (sprites dos mockups) para 3D.

## 4. Árvore do repositório

```
/
├── README.md                          Visão geral atual
├── GUIA_EQUIPE.md                     Uso do Git/LFS em equipe
├── .gitignore · .gitattributes        Regras do Git / Git LFS
│
├── docs/
│   ├── PLANO_DUNGEON_STANCE_UE5.md        Este plano
│   ├── ESPEC_REGRAS_DUNGEON_STANCE.md     Regras extraídas do protótipo
│   ├── ARVORES.MD · CLASSES.md · LISTA_MODELOS.MD · PLANO.md
│   ├── Plano_Execucao_Squad_Roguelike.txt (legado)
│   └── video_para_animacao.md             Pipeline vídeo IA → 3ds Max → UE5
│
├── arts/                              NÃO MODIFICAR — concepts e assets brutos
├── _Export_3dsMax_Final/              FBX validados para a UE5
│   ├── Characters/ · Environments/ · Items/ · Props/
│
├── Source/DungeonStance/              Módulo C++ do jogo (criado junto do .uproject)
│   ├── Core/       GameMode, GameState, PlayerState, regras da run
│   ├── Gameplay/   GAS: Stances, Cast Framework, status, inimigos
│   ├── UI/         HUD de party de 1 a 5, cartas, lobby
│   └── Network/    Sessões, matchmaking, backend
│
├── Godot_Legacy_Reference/            Cliente Godot antigo (consulta)
├── Nakama_Legacy_Reference/           Servidor Nakama antigo — fonte das regras
├── build/                             Build jogável do protótipo
└── tools/                             Scripts de build e conversão
```

Quando o projeto UE5 for criado, `DungeonStance.uproject`, `Config/` e `Content/` ficam na raiz, ao lado de `Source/`.

## 5. Roteiro

### Fase 0 — Fundação (agora)
- [x] Arquivar protótipo (`*_Legacy_Reference`) e criar estrutura de pastas
- [x] Extrair regras do servidor Nakama para a especificação
- [ ] Criar o projeto C++ `DungeonStance` na UE5 (template Third Person ou Top Down) dentro desta pasta
- [ ] Habilitar plugins: Gameplay Abilities, Enhanced Input, CommonUI, StateTree
- [ ] Decidir backend (Nakama ou EOS/Steam)

### Fase 1 — Vertical slice (1 classe, 1 arena, 1 a 5 jogadores)
- [ ] Core: GameMode/GameState/PlayerState com ciclo de ondas e escala por número de jogadores
- [ ] GAS: Attribute Set, ataque, esquiva e 1 skill (Bárbaro — Redemoinho)
- [ ] 3 arquétipos de inimigo (minion, runner, shooter) + 1 chefe
- [ ] HUD de party de 1 a 5 e escolha de cartas (1 Stance completa)
- [ ] Testes: *Automation Tests* das fórmulas portadas + sessão de 5 clientes no PIE

### Fase 2 — Conteúdo do lançamento
- [ ] 9 classes, 4 Stances completas, 8 habilidades de apoio, todos os status
- [ ] 3 arenas, bestiário completo, Tormento
- [ ] Progressão permanente, runas, inventário, Espólio
- [ ] Assets 3D finais via `_Export_3dsMax_Final/`

### Fase 3 — Lançamento do Dungeon Stance
- [ ] Servidores dedicados, matchmaking por tamanho de party
- [ ] Balanceamento por playtest (principalmente a escala de 1 a 5 jogadores)
- [ ] Coleta de métricas de retenção e feedback da comunidade

### Fase 4 — Evolução para MMORPG (se houver aceitação)
- [ ] Hub social persistente entre as dungeons
- [ ] Mundo aberto coop por instância (base já prototipada em `world_*.ts`: 28 skills de coleta, combate e suporte, nós de recurso, acampamentos de monstros)
- [ ] Crafting e economia (modelos de `LISTA_MODELOS.MD`)
- [ ] Escala de rede: Iris / Replication Graph, servidores por zona, MassEntity para multidões

**Decisões de agora que protegem a evolução para MMORPG:** dados de balanceamento em Data Assets, servidor sempre autoritativo, inventário e progressão persistidos fora do servidor de jogo e Stances/classes independentes do modo de jogo.

## 6. Próximos passos imediatos

1. Criar o projeto UE5 `DungeonStance` (C++) nesta pasta.
2. Portar as fórmulas puras de `ESPEC_REGRAS_DUNGEON_STANCE.md` §2 e §3 (ondas, XP, rank) com testes.
3. Montar o vertical slice da Fase 1.
