# Especificação de Regras — Dungeon Stance (extraída do protótipo)

Fonte de verdade do protótipo: **`Nakama_Legacy_Reference/src/`** (servidor autoritativo em TypeScript, ~9.800 linhas, 15 suítes de teste em `test/`).
O GDScript em `Godot_Legacy_Reference/scripts/` é cliente, visual e modo offline — ele espelha estas regras, não as define.

Este documento é a referência para reimplementar as regras em C++/GAS na UE5. Cada seção aponta o arquivo de origem; em caso de dúvida, o código e os testes de lá valem mais do que este resumo.

## 0. Conversões

| Protótipo | UE5 |
| --- | --- |
| 40 px = 1 m | 1 uu = 1 cm → **1 px = 2,5 cm** |
| Simulação a 10 Hz (`TICK_RATE = 10`) | Tempo contínuo (segundos). Converter `ticks / 10` → s |
| Arena 4800 × 3000 px | 120 m × 75 m |
| Cliente dono do próprio HP e do dano informado (com teto) | **Servidor calcula dano e HP via GAS**. O teto `maxDamagePerHit` só existia por causa da confiança no cliente e pode ser descartado |

## 1. Party e sessão — `matchmaker.ts`, `squad_state.ts`, `squad_match.ts`

| Regra | Protótipo | Dungeon Stance |
| --- | --- | --- |
| Tamanho da party | `SQUAD_SIZE = 4` fixo (offline completava com IA) | **1 a 5** jogadores (Solo, Dupla, Trio, Esquadrão, Grupo de 5) |
| Spawn dos heróis | Anel de raio 60 px (150 cm), ângulo `index/4 × 2π` | Ângulo `index/N × 2π`, N = jogadores da sessão |
| Timeout para todos entrarem | 30 s | manter |
| Jogador inativo | 5 s sem estado = não bloqueia derrota nem ganha recompensa | manter |
| Partida vazia | encerra após 15 s | manter |
| Fim de partida | tela final fica 10 s | manter |

**Escala por número de jogadores — novo, a definir.** O protótipo nunca escalou inimigos pelo tamanho da party (sempre 4). Proposta inicial para balancear em playtest:
`quantidade_inimigos × (0,4 + 0,15 × N)` e `vida_do_chefe × (0,5 + 0,125 × N)` — com N = 4 ambos dão ×1,0, o mesmo valor do protótipo.

## 2. Fluxo da run (modo Dungeon Stance, herdado da Debandada) — `squad_match.ts`, `squad_logic.ts`

- **10 ondas** (`MAX_WAVES`). Chefes nas ondas **5 e 10** (`isBossWave`: múltiplo do intervalo de chefe, ou a última).
- Inimigos regulares por onda: `6 + 3 × onda`; +1 chefe nas ondas de chefe.
- Escala por onda e rank médio da party:
  - Vida: `(1 + 0,12 × (onda − 1)) × (1 + 0,04 × max(0, rankMédio − 1))`
  - Dano: `(1 + 0,06 × (onda − 1)) × (1 + 0,02 × max(0, rankMédio − 1))`
- Primeiro disparo de inimigos à distância escalonado: `recarga + aleatório(0..recarga)`.
- Pausa entre ondas: mínimo 4 s. Se houver carta a escolher: janela de **20 s** + 2 s de margem; quando todos escolheram, a próxima onda começa 1 s depois (nunca antes dos 4 s).
- Onda não limpa em **150 s** termina mesmo assim (evita travar com inimigo inalcançável).
- Run determinística por `seed` (RNG em `createRng`) — útil para replays e testes.

## 3. XP e níveis

### Nível do time (dentro da run) — `squad_logic.ts`
- XP para o próximo nível: `40 + 25 × nível`.
- Cada nível acima do 1 dá **1 escolha de carta** a cada jogador (`pendingPicks`). Se a árvore inteira estiver no máximo, a escolha é consumida em silêncio.

### Rank da classe (permanente) — `progression.ts`
- Rank máximo **60**. XP para o próximo rank: `round(250 × rank^1,2)`.
- Bônus por rank acima do 1: **+1% vida, +0,6% dano**.
- Runas liberadas nos ranks **5 e 12** (`runes.ts`).
- XP de fim de run por jogador: `100 + 60 × ondasLimpas + (vitória ? 300 : 0) + min(250, 5 × kills)`.

## 4. Classes — `classes.ts` (+ `CLASSES.md`)

| Classe | Papel | Vida | Dano | Atq/s | Alcance (cm) | Mov. (cm/s) | Recurso | Skill |
| --- | --- | ---: | ---: | ---: | ---: | ---: | --- | --- |
| Bárbaro | Melee / Tank | 1400 | 38 | 1,1 | 188 | 475 | Fúria | Redemoinho |
| Cruzado | Tank / Buff | 1500 | 30 | 0,9 | 213 | 450 | Fé | Investida Sagrada |
| Caçador de Demônios | Ranged DPS | 900 | 26 | 2,0 | 1050 | 575 | Disciplina | Rajada |
| Monge | Melee / Utilidade | 1050 | 22 | 2,4 | 188 | 550 | Espírito | Onda de Luz |
| Necromante | Invocador / DoT | 950 | 24 | 1,0 | 900 | 488 | Essência | Lança de Ossos |
| Arcanista | Ranged AoE | 850 | 40 | 0,9 | 950 | 500 | Mana | Pulso Arcano |
| Cavaleiro Sangrento | Híbrido / Roubo de vida | 1150 | 30 | 1,4 | 400 | 513 | Sangue | Ceifa Sangrenta |
| Mago | Burst / Longo alcance | 820 | 42 | 0,85 | 1400 | 488 | Mana | Meteoro |
| Arqueiro | Ranged DPS | 900 | 30 | 1,6 | 1500 | 538 | Foco | Flecha Perfurante |

Comum a todas: crítico 5%, dano crítico ×1,5, armadura 10, esquiva de 425 cm com recarga de 1,2 s e ~0,24 s de invulnerabilidade, pulo de até 140 cm, redução de recarga máxima de 60%.
Tipos de ataque: `melee` (arco de 110°, só o mais próximo), `melee_aoe` (arco, todos), `projectile`, `projectile_aoe` (explode ao acertar, no terreno ou no fim do alcance).
Recurso: Bárbaro e Cavaleiro Sangrento começam **vazios** e enchem batendo e apanhando; os outros começam cheios e regeneram.
Itens marcados como "ainda não implementado" em `CLASSES.md` (aura do Cruzado, lentidão do Arcanista etc.) são pendências a implementar direto na UE5.

### Corrida (sprint) — `squad_logic.ts`
Estamina máx. 100; velocidade ×1,6; gasto 20/s; regeneração 28/s; precisa de 10 para começar a correr.

## 5. Stances (caminhos) e cartas — `cards.ts`, `cards_*.ts`

**Proposta de design:** a *Stance* do Dungeon Stance é o caminho que o jogador escolhe antes da run. São as 4 árvores que já existem no protótipo:

| Stance | Nome no protótipo | Ramos |
| --- | --- | --- |
| Crítico | Caminho do Crítico | Precisão, Sede Carmesim, Brutalidade, Ritmo Letal |
| Arcana | Caminho do Dano Mágico | Chama, Mente, Fluxo, Tempestade |
| Defesa | Caminho da Defesa | Couraça, Vitalidade, Égide, Inabalável |
| Apoio | Caminho do Apoio | Comando, Cura, Proteção, Tática |

- A carta só aparece no sorteio se estiver abaixo do máximo de acúmulos **e** se ao menos um pai na árvore já estiver comprado (`parentsOwned`, `isCardRollable`).
- O servidor valida cada escolha (`validateCardPick`): árvore certa, carta sorteável, escolhas disponíveis pelo nível do time.
- Atributos finais = base + soma dos modificadores das cartas (`aggregateCardStat`).

## 6. Habilidades de apoio (Cast Framework) — `abilities.ts`, `squad_abilities.ts`

Até **4 slots** (Q/E/R/F); a 5ª substitui uma, à escolha. Redução de recarga das habilidades de apoio: máx. 50%.

| Habilidade | Tipo | Recarga (s) | Raio (cm) | Duração (s) | Magnitude |
| --- | --- | ---: | ---: | ---: | --- |
| Aura do Guerreiro Perdido | damage_buff | 20 | 700 | 6 | +30% dano |
| Bastião Compartilhado | shield | 16 | 600 | 6 | escudo de 160 |
| Pulso Revigorante | heal | 14 | 650 | — | cura 18% da vida máx. |
| Grito de Batalha | haste | 18 | 650 | 6 | +25% vel. ataque |
| Estandarte de Guerra | zone | 24 | 500 | 10 | +15% |
| Santuário | zone | 30 | 400 | 5 | 5% vida/s, 30% redução de dano |
| Totem Guardião | zone (provoca) | 26 | 400 | 12 | 20%, atrai inimigos |
| Chamado Tático | rally | 20 | 650 | 4 | reseta esquiva, +30% movimento |

Na UE5: cada uma vira um `UGameplayAbility` com GE de custo/recarga; o efeito em área aplica GEs nos aliados dentro do raio.

## 7. Status — `status.ts`

| Status | Duração | Acúmulos máx. | Observação |
| --- | --- | ---: | --- |
| Sangramento | 3 s | 3 | DoT, aplica a cada 0,5 s |
| Queimadura | 4 s | 3 | DoT |
| Veneno | 5 s | 5 | DoT |
| Marca | 4 s | — | aumenta o dano recebido |
| Lentidão | variável | — | máx. 70%; chefes recebem metade |
| Atordoamento | máx. 1,5 s | — | em chefe vira lentidão de 30%; tem recarga interna |
| Enfraquecer / Exposto / Provocação | variável | — | dano causado ↓ / dano recebido ↑ / força o alvo |

Na UE5: um `UGameplayEffect` por status, com stacking configurado, e Gameplay Tags (`Status.Bleed`, `Status.Stun`…).

## 8. Inimigos — `bestiary.ts`, `bestiary_ai.ts`, `enemies.ts`

Base do minion de nível L: vida `50 + 16L + 0,5L²`, DPS `12 + 4,5L`, armadura `2L`, XP de combate `round(4 + 0,05 × vida)`, ouro médio `round(2 + 1,6L + 0,05L²)`.

| Arquétipo | ×Vida | ×DPS | Vel. (cm/s) | À distância |
| --- | ---: | ---: | ---: | --- |
| minion | 1,0 | 1,0 | 175 | — |
| runner | 0,6 | 0,7 | 325 | — |
| brute | 3,7 | 2,3 | 113 | — |
| shooter | 0,75 | 0,4 | 150 | recarga 2,6 s, alcance 850 cm |
| caster | 0,8 | 0,5 | 138 | recarga 4,0 s, alcance 750 cm |
| swarm | 0,3 | 0,35 | 275 | — |
| passive | 0,5 | 0,3 | 150 | — |
| boss | 30 | 3,5 | 130 | — |

Elite: vida ×2,5, DPS ×1,5. Dano de projétil = 2,2 × DPS base do nível.
Comportamentos de IA: melee, swarm, charger, blinker, kiter, ranged, passive, brood (chefe que invoca 5 ratos aos 60% e 30% da vida).

Arenas (nível efetivo = `3 + 4 × índice + 2 × Tormento`):

| Arena | Índice | Chefe onda 5 | Chefe onda 10 |
| --- | ---: | --- | --- |
| Clareira da Floresta | 0 | boss | boss |
| Galerias Afogadas | 1 | drowned_zombie_elite | mother_mange |
| Cemitério dos Silenciosos | 2 | wandering_skeleton_elite | boss |

## 9. Loot e Espólio — `loot.ts`, `espolio_match.ts`, `inventory*.ts`

- 1 sorteio principal por onda limpa, +1 a cada 8 kills na onda (máx. +2 por onda).
- Ouro por onda: `4 + 2 × onda + kills`; vitória: +60.
- Cada jogador sorteia a própria parte do **Baú do Esquadrão** (sem disputa entre jogadores); máx. 16 pilhas diferentes. No fim, vira o **Espólio**.
- Cofre cheio: o excedente vira ouro a 60% do valor base.
- Inventário: Cofre da Conta (120 slots, compartilhado entre classes), Mochila por classe (28), 12 slots de equipamento + Cinto de Batalha (4 consumíveis). Só o servidor grava.

## 10. Anti-trapaça herdada (para referência)

O protótipo limitava 45 golpes/s por jogador, conferia movimento plausível (velocidade + margem de esquiva + sprint) e limitava o dano por golpe a `danoBase × multRank × 16 × (1 + 0,2 × (nívelTime − 1))`. Com servidor autoritativo na UE5, a maior parte disso some; manter só a validação de movimento (Character Movement Component já faz correção de posição no servidor).

## 11. Mundo Aberto Coop (base para o futuro MMORPG) — `world_*.ts`

Já existe no protótipo, fora do escopo do lançamento: instância de mundo por grupo a 10 Hz, acampamentos de monstros com respawn, nós de coleta, fogueiras e **28 skills estilo RuneScape** (Coleta / Combate / Suporte, nível máx. 50, por conta). A lista de modelos em `LISTA_MODELOS.MD` (mineração, forja, alquimia…) pertence a essa fase.
