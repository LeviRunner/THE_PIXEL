# PLANO — Dungeon Stance e a expansão para MMORPG

> **Atualização (2026-10-13) — como ler este documento hoje:**
> - O projeto agora se chama **Dungeon Stance** e foi migrado para **Unreal Engine 5**. O plano de execução atual está em [`PLANO_DUNGEON_STANCE_UE5.md`](PLANO_DUNGEON_STANCE_UE5.md) e as regras de jogo em [`ESPEC_REGRAS_DUNGEON_STANCE.md`](ESPEC_REGRAS_DUNGEON_STANCE.md).
> - O modo de ondas, chamado aqui de **Debandada**, é a base do modo de lançamento **Dungeon Stance**: dungeon instanciada, 10 ondas, chefes nas ondas 5 e 10, party de **1 a 5 jogadores** (aqui o texto ainda fala em esquadrão de 4). Onde se lê "Debandada", leia "Dungeon Stance"; os identificadores de código (`'debandada'`, `squad_match`…) referem-se ao protótipo.
> - O mundo aberto **Crônicas do Império** (§6 em diante: habilidades de mundo, Capital, 25 reinos, metalurgia, Bolsa Imperial, missões) passa a ser a **futura expansão MMORPG**, a ser desenvolvida se o Dungeon Stance tiver aceitação da comunidade (Fase 4 do plano UE5).
> - As referências técnicas a Godot, GDScript e Nakama (§1, §17, §18) descrevem o protótipo, hoje arquivado fora do repositório em `Godot_Legacy_Reference/` e `Nakama_Legacy_Reference/`. Na UE5, a arquitetura é a do plano novo (C++ + Blueprints + GAS, servidor autoritativo da Unreal).

> **Status:** rascunho vivo, para ajustarmos juntos. Nada aqui está implementado ainda.
> **Escopo:** monstros, árvores de habilidades (48+ nós por caminho), Ficha do Herói (submenu 3D), Inventário Compartilhado, renomear o modo atual para **Debandada**, o novo modo de mundo aberto com habilidades no estilo RuneScape (até o nível 50), a **Capital do Império**, os 25 reinos, mercado, metalurgia, missões, conquistas, desbloqueios, esquemas de dados e roteiro.
> **Regra de ouro:** tudo é **inspirado** em RuneScape e Diablo Immortal, nunca copiado. Nomes, mapas, castelos e criaturas são originais.

---

## Sumário

0. [Convenções](#0-convenções)
1. [Onde estamos (estado atual)](#1-onde-estamos-estado-atual)
2. [Modo de lançamento: DUNGEON STANCE (antiga Debandada)](#2-modo-de-lançamento-dungeon-stance-antiga-debandada)
3. [Menu de classes: Ficha do Herói (submenu 3D)](#3-menu-de-classes-ficha-do-herói-submenu-3d)
4. [Inventário Compartilhado](#4-inventário-compartilhado)
5. [Árvores de habilidades: 48+ nós por caminho](#5-árvores-de-habilidades-48-nós-por-caminho)
6. [Expansão MMORPG: Crônicas do Império (mundo aberto)](#6-expansão-mmorpg-crônicas-do-império-mundo-aberto)
7. [Habilidades de mundo (estilo RuneScape) em 3 ramos](#7-habilidades-de-mundo-estilo-runescape-em-3-ramos)
8. [Bestiário: monstros do nível 1 ao 50](#8-bestiário-monstros-do-nível-1-ao-50)
9. [A Capital do Império (primeiro mapa)](#9-a-capital-do-império-primeiro-mapa)
10. [Os 25 reinos](#10-os-25-reinos)
11. [Itens, raridade e progressão de equipamento](#11-itens-raridade-e-progressão-de-equipamento)
12. [Metalurgia e cadeias de produção](#12-metalurgia-e-cadeias-de-produção)
13. [Mercado: a Bolsa Imperial](#13-mercado-a-bolsa-imperial)
14. [Missões (quests)](#14-missões-quests)
15. [Conquistas e o Diário da Capital](#15-conquistas-e-o-diário-da-capital)
16. [Sistema de desbloqueios](#16-sistema-de-desbloqueios)
17. [Arquitetura técnica: dados, servidor, cliente](#17-arquitetura-técnica-dados-servidor-cliente)
18. [Multiplayer no mundo aberto](#18-multiplayer-no-mundo-aberto)
19. [Antitrapaça e integridade da economia](#19-antitrapaça-e-integridade-da-economia)
20. [Roteiro (roadmap) por fases](#20-roteiro-roadmap-por-fases)
21. [Perguntas em aberto para decidir](#21-perguntas-em-aberto-para-decidir)
- [Apêndice A — Fórmulas e tabelas numéricas](#apêndice-a--fórmulas-e-tabelas-numéricas)
- [Apêndice B — Fontes de pesquisa](#apêndice-b--fontes-de-pesquisa)

---

## 0. Convenções

| Item | Convenção |
|---|---|
| Unidade de distância | **40 px de arena = 1 m** (igual a `PX_PER_METRE` em `level.ts`). Coordenadas de mapa neste documento em **metros**, origem no canto superior esquerdo, **x para leste, y para sul** (o mesmo eixo 2D que o servidor simula). |
| Tempo | segundos (s). Tick do servidor: 10 Hz (`TICK_RATE`). |
| IDs | `snake_case` em inglês, como já é no código (`keen_eye`, `goblin_grunt`). Nomes visíveis sempre em pt-BR. |
| Modificadores | o mesmo `StatModifier` de `runes.ts`: `{ stat, op: 'add' \| 'mult', value }`, agregados como `(base + Σadd) × Πmult`. |
| Etiquetas de esforço | **[N]** novo stat simples (só agregação e uso em 1 lugar) · **[S]** precisa do sistema de **status em inimigos** (marca, sangramento, lentidão, atordoamento) no servidor · **[G]** precisa de lógica nova de gameplay (gatilho, proc, área). |
| Raridade das cartas | `common` (peso 10), `rare` (peso 5), `epic` (peso 2). Capstones (nó 12 de cada ramo) usam `epic` com peso 1. **Proposta opcional:** um quarto valor `legendary` (peso 1), se quisermos diferenciar os capstones visualmente. |

---

## 1. Onde estamos (estado atual)

Resumo do que existe no código hoje e que este plano **herda**:

- **Cliente** Godot 4.7 (GDScript), **servidor** Nakama 3.41 (TypeScript, autoritativo). O servidor é a fonte única dos dados; `npm run export-data` gera `godot/data/game_data.json` para o modo offline (`LocalMatchHost` fala os mesmos op codes).
- **9 classes:** Bárbaro, Cruzado, Caçador de Demônios, Monge, Necromante, Arcanista, Cavaleiro Sangrento, Mago e Arqueiro. Cada uma com rank permanente (1–60), +1% vida e +0,6% dano por rank, runas nos ranks 5 e 12.
- **4 caminhos (árvores globais)** com 26 cartas no total (6 em Crítico, Dano Mágico e Defesa; 7 em Apoio, incluindo 4 habilidades ativas Q/E/R/F). Modelo de carta: `id, name, description, tree, rarity, weight, max_stacks, modifiers, ability?, branch, tier, parents`. Hoje `branch/tier/parents` são **só apresentação**.
- **Modo único:** ondas de monstros (10 ondas, chefe nas ondas 5 e 10), arena de **120 × 75 m** (4800 × 3000 px) com rio, pontes, platôs, rampas, ruínas e floresta (`FOREST_GLADE`, "Clareira da Floresta"). Online: esquadrão de 4 via matchmaker por rank. Offline: solo, dupla ou esquadrão com IA.
- **Inimigos atuais** (`ENEMY_ARCHETYPES` em `squad_logic.ts`):

| Tipo (código) | Nome no jogo | Vida | Vel. (px/s) | DPS contato | Alcance | Projétil | XP |
|---|---|---:|---:|---:|---:|---|---:|
| `grunt` | Goblin Soldado | 120 | 70 | 25 | corpo a corpo | — | 10 |
| `runner` | Demônio Corredor | 70 | 130 | 18 | corpo a corpo | — | 8 |
| `brute` | Goblin Brutamontes | 450 | 45 | 60 | corpo a corpo | — | 35 |
| `shooter` | Esqueleto-Zumbi Atirador | 90 | 60 | 10 | 340 | 55 dano, 280 px/s, a cada 2,6 s | 14 |
| `boss` | Senhor da Clareira | 3600 | 52 | 90 | 200 | 85 dano, 230 px/s, a cada 3,2 s | 160 |
| (invocação) | Esqueleto (lacaio do Necromante) | — | — | 14 dano | 260 | — | — |

  Escala da onda: vida × `(1 + 0,12·(onda−1)) × (1 + 0,04·(rank médio−1))`; DPS × `(1 + 0,06·(onda−1)) × (1 + 0,02·(rank−1))`.
- **Arte:** jogadores são **modelos 3D procedurais** (`ModularCharacter.gd`, `CharacterParts.gd`, `ClassGear.gd`, `MeshKit.gd`). Inimigos são **sprites billboard** recortados dos mockups (`SpriteActor3D`), e o brutamontes é o goblin escalado e escurecido. Uma cena em `res://art/enemies/<tipo>.tscn` substitui o sprite sem mudar código.

---

## 2. Modo de lançamento: DUNGEON STANCE (antiga Debandada)

O modo de ondas, que no protótipo se chamava **Debandada** ("a horda debandada que avança sobre o esquadrão"), é a base do **Dungeon Stance**, o modo de lançamento: runs curtas em dungeon instanciada para 1 a 5 jogadores, com cartas da Stance (caminho) escolhida, que já funcionam no protótipo.

### 2.1 O que muda (checklist para quem for implementar)
| Onde | Mudança |
|---|---|
| Menu principal | Botões passam a ler **Debandada — Solo / Dupla / Esquadrão** e **Debandada Online**. Ao lado, um botão novo (bloqueado até a fase M6 do roteiro): **Crônicas do Império**. |
| `GameStateManager` / prefs | Adicionar o conceito de **modo de jogo** (`game_mode: 'debandada' \| 'imperio'`), salvo em `user://prefs.cfg`. |
| Servidor | `label` da partida: `{ mode: 'debandada', ... }` (hoje `mode: 'squad'`). Manter `'squad'` aceito por compatibilidade durante uma versão. Query do matchmaker ganha `+properties.mode:debandada`. |
| Resultados | Tela de resultado mostra "Debandada vencida / Debandada fracassou", ondas sobrevividas e o **Espólio** (itens enviados ao Inventário Compartilhado, ver §4). |
| Docs | README e CLASSES: "modo de ondas" → "Debandada". |

### 2.2 Debandada daqui pra frente (evolução sugerida)
- **Arenas rotativas:** além da Clareira da Floresta, arenas temáticas com os monstros do bestiário (§8): Esgotos (ratos, diabretes), Cemitério (mortos-vivos), Mina (anões, golems), Fenda (demônios). Cada arena tem seu chefe na onda 5 e 10.
- **Dificuldades (Tormentos I–V):** cada nível de tormento soma +25% vida e +15% dano aos inimigos e +20% de espólio. Desbloqueadas por rank da classe (5, 12, 20, 30, 40).
- **Espólio:** materiais de monstro, ouro e, raramente, itens de equipamento do mundo aberto (tier pela dificuldade). É a ponte entre os dois modos: quem só joga Debandada também alimenta a economia, mas com taxa menor que o mundo aberto.
- **Equipamento na Debandada:** para manter a run justa, o equipamento do mundo aberto **não** entra nos atributos dentro da Debandada (só o rank e as runas, como hoje). O que entra: o **Cinto de Batalha** (até 4 consumíveis levados do inventário, §4.4). Ver pergunta em aberto Q3.

---

## 3. Menu de classes: Ficha do Herói (submenu 3D)

Hoje o painel de detalhe da classe tem **Selecionar classe**, **Árvore de Habilidades** e **Ver ficha completa** (que abre o PNG do mockup). A ficha vira um **submenu com o mesmo design** dos outros menus, mostrando o **modelo 3D da classe isolado**. A imagem do mockup deixa de aparecer no jogo (continua só como blueprint de arte em `tools/art/blueprints/`).

### 3.1 Nova ordem dos botões no painel da classe
```
[ Selecionar classe ]
[ Árvore de Habilidades ]   [ Ficha do Herói ]
──────────────────────────────────────────────
[ Inventário Compartilhado ]        (novo, §4)
```

### 3.2 Layout da Ficha do Herói
Inspirado no painel de personagem do Diablo Immortal (modelo ao centro, equipamento ao redor, atributos num painel lateral), mas com a identidade visual dos nossos menus atuais.

```
┌──────────────────────────────────────────────────────────────────────────┐
│  ◄ Voltar        FICHA DO HERÓI — NECROMANTE  · Rank 14 ███████░░ 62%     │
├──────────┬───────────────────────────────────────────┬───────────────────┤
│ ABAS     │   [Elmo]                       [Amuleto]  │ ATRIBUTOS         │
│ ▸ Visão  │   [Ombros]     (modelo 3D)     [Anel I]   │ Vida      1.088   │
│   Atrib. │   [Peito]    sobre pedestal    [Anel II]  │  base 950         │
│   Skill  │   [Mãos]     girando, luz de   [Capa]     │  rank +12%        │
│   Runas  │   [Pernas]   recorte e névoa   [Cinto]    │  equip +0         │
│   Equip. │   [Pés]                        [Munição]  │ Dano      26,6    │
│   Mundo  │   [Arma principal]   [Mão secundária]     │ Crítico   5%      │
│   Lore   │                                           │ ...               │
│   Conq.  │  ◄ girar ►   Poses: Parado|Ataque|Skill|  │ RECURSO: Essência │
│          │  Esquiva|Pulo|Invocação|Vitória  ⟲ Zoom   │ SKILL: Lança...   │
└──────────┴───────────────────────────────────────────┴───────────────────┘
```

**Abas da coluna esquerda**
| Aba | Conteúdo |
|---|---|
| Visão geral | Papel, recurso, skill, como jogar (texto do CLASSES.md), caminho recomendado. |
| Atributos | Tabela completa com a decomposição de cada stat: **base → rank → runas → equipamento → habilidades de mundo**. Tooltips explicam fórmulas. |
| Skill | Skill da classe com números (dano, raio, recarga, custo) e uma prévia em loop do modelo usando a skill. |
| Runas | Runas desbloqueadas/bloqueadas por rank, com o efeito exato. |
| Equipamento | Só no Crônicas do Império: slots, comparação de itens (verde/vermelho), requisito de nível. Na Debandada aparece o **Cinto de Batalha**. |
| Mundo | Resumo das habilidades de mundo (§7) que afetam essa classe (Ataque, Força, Magia...), com o nível de cada uma. |
| Lore | Texto de ambientação da classe, origem no Império, frase de efeito. |
| Conquistas | Conquistas ligadas à classe (§15) e títulos desbloqueados. |

**Visualizador 3D (centro)**
- `SubViewport` próprio com `Camera3D`, `WorldEnvironment` e luzes isoladas (o modelo não depende da cena de gameplay). Reaproveita `ModularCharacter.gd` + `ClassGear.gd` (o mesmo que a `--model-gallery` já usa).
- **Pedestal** de pedra com runa da classe gravada; fundo escuro com névoa volumétrica leve e luz de recorte (rim light) na cor da classe (`color` do `classes.ts`).
- **Controles:** arrastar com o mouse / analógico direito gira (turntable com inércia); roda/gatilhos dão zoom (3 níveis: corpo inteiro, meio corpo, rosto); botão ⟲ volta ao enquadramento padrão; giro automático lento quando ocioso por 4 s.
- **Poses:** botões tocam a animação procedural existente (idle, ataque por família de arma, skill com VFX de `CharacterVfx.gd`, dash, pulo, invocação no Necromante, "Vitória" nova).
- **Equipamento:** ao passar o mouse num slot, a peça correspondente brilha no modelo; ao trocar equipamento, o modelo atualiza na hora (preparação para equipamento visível no mundo aberto).
- **Custo:** o SubViewport só renderiza com a ficha aberta (`render_target_update_mode = UPDATE_WHEN_VISIBLE`), resolução 1024² com MSAA 4×.

**Gamepad:** LB/RB trocam abas, analógico direito gira, D-pad navega slots, Y alterna poses, B volta.

---

## 4. Inventário Compartilhado

Botão **abaixo** de "Árvore de Habilidades" e "Ficha do Herói", acessível **pelo menu** e **dentro da gameplay**. O uso muda conforme o modo.

### 4.1 Camadas de armazenamento
| Camada | Dono | Tamanho inicial | Onde fica | Para quê |
|---|---|---|---|---|
| **Cofre da Conta** (o "Inventário Compartilhado") | a conta (todas as classes) | 120 espaços, abas de 40; +40 por compra/conquista até 400 | Nakama storage `inventory`, chave `vault` | Guardar tudo; trocar itens entre classes; receber o Espólio da Debandada e o resultado do mercado. |
| **Mochila** | a classe em uso (no Crônicas do Império) | 28 espaços (homenagem ao inventário de 28 do RS) | `inventory`, chave `bag_<class_id>` | O que você carrega no mundo. Limita viagens de coleta. |
| **Equipamento** | a classe | 13 slots (§11.3) | `inventory`, chave `equip_<class_id>` | Itens vestidos. |
| **Cinto de Batalha** | a classe | 4 slots de consumível | dentro de `equip_<class_id>` | Poções/comidas usadas nas teclas 1–4 (Debandada e mundo). |
| **Baú do Esquadrão** | a sessão do grupo | 16 espaços | estado da partida (memória) | Saque compartilhado temporário; ao fim, cada item vai para quem o pegou ou é dividido por rodízio. |

> "Compartilhado" aqui significa **entre todas as suas classes**. Existe também o Baú do Esquadrão (temporário, entre jogadores). Ver Q1.

### 4.2 Regras por modo
| Situação | Debandada | Crônicas do Império |
|---|---|---|
| Menu principal | Acesso total ao Cofre: organizar, descartar, equipar o Cinto de Batalha, ver o Espólio recebido. | Igual, e também equipar a classe e mover itens para a Mochila (aplicado ao entrar no mundo). |
| Dentro da partida | Abre como **sobreposição só de leitura** + Cinto de Batalha (trocar consumíveis entre ondas, nunca durante a onda). O jogo **não pausa** online. | Mochila e equipamento sempre editáveis. O **Cofre** só aceita depósito/saque perto de um **Banco**, do **Baú do Acampamento** (Construção 20) ou com a **Bolsa Abissal** (Runografia 40, 5 cargas/dia). Fora disso, o Cofre aparece só para consulta. |
| Saque (loot) | Fica no Baú do Esquadrão durante a run; no fim vira **Espólio** e cai direto no Cofre. | Cai no chão como objeto; quem pegar leva para a Mochila. Saque de chefe é **individual** (cada um rola o seu, sem disputa). |
| Morte | Não perde nada. | Mochila fica num **túmulo** por 10 min no local; equipamento perde 10% de durabilidade. Itens marcados como "Protegidos" (3 primeiros espaços) ficam com você. |

### 4.3 Interface
- Grade de ícones 64 px com moldura pela raridade (§11.1), contador de pilha e filtros (Equipamento, Recursos, Consumíveis, Missão, Cosméticos).
- Painel lateral do item: nome, raridade, nível de item, requisitos (verde/vermelho), atributos, preço guia da Bolsa, origem ("Saque: Viúva do Breu").
- Arrastar e soltar entre Cofre ⇄ Mochila ⇄ Equipamento; clique direito = menu contextual (Usar, Equipar, Dividir pilha, Vender na Bolsa, Descartar).
- Busca por texto, ordenar por tipo/raridade/valor, **travar espaço** para não vender sem querer.
- Gamepad: grade navegável no D-pad, X = menu contextual, Y = alternar Cofre/Mochila.

### 4.4 Cinto de Batalha (ponte entre os modos)
Até 4 consumíveis (pilhas) usados nas teclas **1–4** (no gamepad, D-pad com segurar LT). Na Debandada as teclas 1/2/3 hoje escolhem cartas: a escolha de carta passa a usar **1/2/3 só com a tela de cartas aberta** (sem conflito, porque a tela de cartas pausa a entrada de combate).

### 4.5 Servidor
- Toda mudança de inventário é um **RPC** com validação (`inventory_move`, `inventory_split`, `inventory_discard`, `inventory_equip`), escrita com **controle de versão otimista** (o mesmo padrão de `grantMatchXp` em `progression.ts`).
- Objetos com `permissionWrite: 0` (só o servidor escreve) e `permissionRead: 1` (só o dono lê).
- Offline: o mesmo esquema em `user://offline_profile.json`, separado do online (como já é com o rank).


---

## 5. Árvores de habilidades: 48+ nós por caminho

Cada um dos 4 caminhos (Crítico, Dano Mágico, Defesa, Apoio) passa de 6–7 cartas para **48 nós**: **4 ramos (colunas `branch` 0–3) × 12 níveis (`tier` 1–12)**. Total: **192 nós**. As 26 cartas atuais **continuam com o mesmo `id`** (só mudam de posição quando necessário), então saves, testes e o `game_data.json` seguem compatíveis.

### 5.1 Regras de layout e de sorteio

| Regra | Detalhe |
|---|---|
| Estrutura | Cada ramo é uma "espinha" de 12 nós. Nos níveis **4 e 8** o nó tem um segundo pai no **ramo vizinho** (ligação cruzada), e o nó 12 (capstone) tem pai no próprio ramo e num vizinho. Isso desenha a árvore ramificada e permite "trocar de trilha" no meio da run. |
| Pais passam a valer (mudança de regra) | Hoje `parents` é só apresentação. Proposta: **um nó só entra no sorteio se pelo menos um pai tiver ≥ 1 stack** (nós de tier 1 sempre podem sair). Validado no servidor em `validateCardPick` (nova checagem `parent not owned`) e espelhado em `CardSelectionUI.roll_cards()`. |
| Sorteio | Igual ao atual: 3 cartas, ponderadas por `weight`, sem reposição, só do caminho escolhido, ignorando nós no máximo de stacks **e nós sem pai adquirido**. Se houver menos de 3 elegíveis, completa com nós tier 1 do caminho. |
| Raridades e pesos | Tier 1–2: `common` (10); tiers 3/4/8/10/11: `rare` (5); tiers 6 e 9: `epic` (2); tier 12: `epic` (1) — ou `legendary` (1) se adotarmos o 4º valor. Nós existentes mantêm a raridade atual. |
| Habilidades ativas (Apoio) | O Apoio ganha **4 habilidades novas** (Estandarte de Guerra, Santuário, Totem Guardião, Chamado Tático), totalizando 8. Continuam **4 teclas** (Q/E/R/F): ao pegar a 5ª, a tela pede qual substituir (o servidor guarda `ability_slots[4]`). |
| Nós que exigem sistemas novos | Etiquetas **[N] [S] [G]** (§0). A ordem de implementação recomendada é: primeiro todos os **[N]** e os que já usam stats existentes; depois o **sistema de status em inimigos** no servidor (`squad_logic.ts`), que destrava todos os **[S]**; por fim os **[G]**. Até lá, nós não implementados ficam com `weight: 0` (não saem no sorteio). |
| No Crônicas do Império | Os caminhos viram **progressão permanente** da classe: cada nível ganho em habilidades do ramo Combate (§7) dá 1 **Ponto de Caminho** para comprar nós (mesmas regras de pais e stacks). Um "respec" custa ouro (gasto de moeda). Ver Q5. |

**Stats existentes reaproveitados:** `crit_chance`, `crit_damage`, `damage`, `crit_heal_pct`, `crit_cooldown_refund`, `elemental_amp`, `cooldown_reduction`, `resource_regen`, `attack_speed`, `armor`, `health_regen`, `max_health`, `shield_on_wave`, `cc_resist`, `support_power`, `support_cooldown_reduction`, `support_radius`, `lifesteal`, `aoe_radius`, `attack_range`, `move_speed`, `shield_amount`, `resource_max`, `dodge_cooldown`.

**Stats novos [N] (só agregação + 1 ponto de uso):** `crit_chance_high_hp`, `crit_chance_low_hp`, `crit_chance_missing_hp`, `aoe_damage`, `resource_cost_reduction`, `lightning_chance`, `lightning_damage`, `contact_damage_reduction`, `projectile_damage_reduction`, `health_regen_pct`, `healing_received`, `shield_cap`, `elite_damage`, `support_duration`, `support_heal_power`, `cdr_cap_bonus`.

### 5.2 Caminho do Crítico (48 nós)
Ramos: **0 Precisão** (chance) · **1 Sede Carmesim** (sustento ao critar) · **2 Brutalidade** (dano crítico) · **3 Ritmo Letal** (recarga e velocidade ao critar).

| id | Nome | T | R | Pais | Máx | Rar. | Efeito | Ícone |
|---|---|---:|---:|---|---:|---|---|---|
| `keen_eye` ★ | Olho Afiado | 1 | 0 | — | 5 | C | +5% chance crítica | olho com pupila em losango |
| `lethal_precision` ★ | Precisão Letal | 2 | 0 | keen_eye | 3 | R | +8% chance crítica, dano ×1,05 | flecha atravessando um losango |
| `steady_hand` | Mão Firme | 3 | 0 | lethal_precision | 4 | R | +3% chance crítica; +6% extra contra alvos acima de 80% de vida [N] | mão segurando um fio de prumo |
| `weak_spot` | Ponto Fraco | 4 | 0 | steady_hand, blood_scent | 3 | R | Críticos **marcam** o alvo por 4 s: ele recebe +6% de dano de todo o esquadrão [S] | alvo com rachadura no centro |
| `hunters_patience` | Paciência do Caçador | 5 | 0 | weak_spot | 3 | C | +2% chance crítica por segundo sem critar (máx. +10%); zera ao critar [G] | ampulheta com um olho |
| `surgical_cut` | Corte Cirúrgico | 6 | 0 | hunters_patience | 2 | E | +6% chance crítica; +20% dano crítico contra alvos marcados [S] | bisturi curvo |
| `trained_eye` | Olho Treinado | 7 | 0 | surgical_cut | 4 | C | +4% chance crítica; +40 de alcance (só ataques à distância) | luneta de latão |
| `fatal_angle` | Ângulo Fatal | 8 | 0 | trained_eye, scab | 3 | R | +15% chance crítica em ataques fora do cone frontal de 90° do inimigo [G] | dois arcos cruzados com seta |
| `perfect_focus` | Foco Perfeito | 9 | 0 | fatal_angle | 1 | E | A cada 8 s, o próximo ataque é crítico garantido [G] | estrela de oito pontas num círculo |
| `calculated_risk` | Risco Calculado | 10 | 0 | perfect_focus | 3 | R | +1% chance crítica a cada 10% de vida perdida (máx. +9%) [N] | dado com face de caveira |
| `thousand_cuts` | Mil Cortes | 11 | 0 | calculated_risk | 3 | R | Críticos seguidos no mesmo alvo: +3% chance crítica (acumula 5×, 3 s) [G] | leque de lâminas |
| `eye_of_the_storm` | Olho da Tempestade | 12 | 0 | thousand_cuts, blood_tithe | 1 | E | +10% chance crítica; chance acima de 60% vira dano crítico (cada 1% = +2% dano crítico) [G] | olho no centro de um redemoinho |
| `blood_drop` | Gota Rubra | 1 | 1 | — | 5 | C | +1% roubo de vida | gota com reflexo de lua |
| `critical_thirst` ★ | Sede Crítica | 2 | 1 | keen_eye, blood_drop | 2 | R | Críticos curam 2% da vida máxima | cálice transbordando |
| `blood_scent` | Faro de Sangue | 3 | 1 | critical_thirst | 3 | R | +8% chance crítica contra alvos abaixo de 50% de vida [N] | focinho de lobo e gota |
| `open_wound` | Ferida Aberta | 4 | 1 | blood_scent, bone_breaker | 3 | R | Críticos causam **sangramento**: 20% do dano do golpe em 3 s [S] | corte com três gotas |
| `crimson_pact` | Pacto Carmim | 5 | 1 | open_wound | 3 | C | +4% vida máxima; críticos geram +2 de recurso | contrato com selo de sangue |
| `vampiric_edge` | Fio Vampírico | 6 | 1 | crimson_pact | 2 | E | +1,5% cura por crítico (`crit_heal_pct`); +3% roubo de vida | adaga com presas |
| `scab` | Casca de Ferida | 7 | 1 | vampiric_edge | 3 | C | Curas por crítico dão escudo de 50% do valor (teto 8% da vida máx.) [G] | crosta rachada |
| `feast` | Banquete | 8 | 1 | scab, savage_momentum | 3 | R | Matar com crítico cura 4% da vida máxima [G] | prato com coração |
| `crimson_frenzy` | Frenesi Carmim | 9 | 1 | feast | 1 | E | Abaixo de 35% de vida: +20% chance crítica e +20% roubo de vida [G] | olho vermelho com veias |
| `hemorrhage` | Hemorragia | 10 | 1 | crimson_frenzy | 3 | R | Sangramento dura +2 s e acumula até 3× [S] | coração rachado pingando |
| `blood_tithe` | Dízimo de Sangue | 11 | 1 | hemorrhage | 2 | R | 25% da cura excedente vai para o aliado mais ferido a até 8 m [G] | moedas de sangue numa bandeja |
| `red_sovereign` | Soberano Rubro | 12 | 1 | blood_tithe, annihilation | 1 | E | Críticos curam o esquadrão em 1% da vida máx. de cada um (máx. 1×/0,5 s); +5% roubo de vida [G] | coroa feita de gotas |
| `brutal_strike` ★ | Golpe Brutal | 1 | 2 | — | 5 | C | +25% dano crítico | punho fechado com estilhaços |
| `heavy_hand` | Mão Pesada | 2 | 2 | brutal_strike | 4 | C | +15% dano crítico, +3% dano | manopla com cravos |
| `bone_breaker` | Quebra-Ossos | 3 | 2 | heavy_hand | 3 | R | Críticos atordoam inimigos comuns por 0,3 s (1× a cada 4 s por alvo) [S] | osso partido ao meio |
| `deep_cut` | Corte Profundo | 4 | 2 | bone_breaker, adrenaline | 3 | R | +30% dano crítico contra alvos sangrando [S] | serra com lâmina rubra |
| `overkill` | Exagero | 5 | 2 | deep_cut | 3 | C | 50% do dano crítico excedente de um abate passa ao inimigo mais próximo [G] | caveira estilhaçando |
| `executioner` ★ | Executor | 6 | 2 | overkill | 1 | E | +75% dano crítico e +5% chance crítica | machado de carrasco |
| `savage_momentum` | Ímpeto Selvagem | 7 | 2 | executioner | 4 | C | +10% dano crítico; +5% movimento por 2 s após critar [G] | chifre de touro |
| `giant_slayer` | Matador de Gigantes | 8 | 2 | savage_momentum, hunters_cadence | 3 | R | +12% dano e +20% dano crítico contra brutamontes, elites e chefes [N] | espada sobre crânio gigante |
| `shatter` | Estilhaçar | 9 | 2 | giant_slayer | 1 | E | Críticos da skill explodem: 25% do dano em 2 m [G] | cristal explodindo |
| `cruelty` | Crueldade | 10 | 2 | shatter | 3 | R | +6% dano crítico por inimigo a até 5 m (máx. 5) [G] | punho com espinhos |
| `annihilation` | Aniquilação | 11 | 2 | cruelty | 3 | R | +25% dano crítico, −3% chance crítica (troca deliberada) | martelo partindo rocha |
| `world_ender` | Fim dos Mundos | 12 | 2 | annihilation, perpetual_motion | 1 | E | Dano crítico da skill ×1,5; críticos da skill derrubam comuns por 0,5 s [S] | planeta rachado |
| `quick_reflexes` | Reflexos Rápidos | 1 | 3 | — | 5 | C | +4% velocidade de ataque | luva com relâmpago |
| `relentless_rhythm` ★ | Ritmo Implacável | 2 | 3 | quick_reflexes, brutal_strike | 3 | R | Críticos reduzem a recarga da skill em 0,3 s | tambor de guerra |
| `adrenaline` | Adrenalina | 3 | 3 | relentless_rhythm | 4 | R | Críticos: +3% velocidade de ataque por 3 s (acumula 5×) [G] | coração com raio |
| `dancing_blade` | Lâmina Dançante | 4 | 3 | adrenaline, heavy_hand | 3 | R | Críticos reduzem a recarga da esquiva em 0,15 s [G] | lâmina em espiral |
| `tempo` | Compasso | 5 | 3 | dancing_blade | 3 | C | +6% redução de recarga da skill | metrônomo |
| `flurry` | Saraivada | 6 | 3 | tempo | 2 | E | 10% de chance de um crítico se repetir com 50% do dano [G] | três lâminas fantasmas |
| `hunters_cadence` | Cadência do Caçador | 7 | 3 | flurry | 4 | C | +5% velocidade de ataque, +3% chance crítica | tambor com flechas |
| `chain_reaction` | Reação em Cadeia | 8 | 3 | hunters_cadence, savage_momentum | 3 | R | Abate com crítico reduz todas as recargas (skill e apoio) em 0,5 s [G] | elos faiscando |
| `bullet_time` | Instante Eterno | 9 | 3 | chain_reaction | 1 | E | 10 críticos em 4 s: +30% vel. de ataque e movimento por 3 s (recarga 15 s) [G] | relógio derretendo |
| `resource_echo` | Eco do Recurso | 10 | 3 | bullet_time | 3 | R | Críticos devolvem 1,5% do recurso máximo | onda sonora azul |
| `perpetual_motion` | Moto-Perpétuo | 11 | 3 | resource_echo | 2 | R | +0,2 s em `crit_cooldown_refund`; +5% redução de recarga | engrenagem em forma de ∞ |
| `deaths_metronome` | Metrônomo da Morte | 12 | 3 | perpetual_motion, annihilation | 1 | E | Todo 5º ataque é crítico garantido e reinicia a esquiva; +10% velocidade de ataque [G] | pêndulo com lâmina de foice |

★ = carta que já existe hoje.

### 5.3 Caminho do Dano Mágico (48 nós)
Ramos: **0 Chama** (amplificação e queimadura) · **1 Mente** (recarga e gelo/controle) · **2 Fluxo** (recurso) · **3 Tempestade** (raios encadeados).

| id | Nome | T | R | Pais | Máx | Rar. | Efeito | Ícone |
|---|---|---:|---:|---|---:|---|---|---|
| `elemental_amp` ★ | Amplificação Elemental | 1 | 0 | — | 5 | C | +8% dano elemental | chama dentro de um prisma |
| `arcane_power` ★ | Poder Arcano | 2 | 0 | elemental_amp | 3 | R | Dano ×1,08, +5% dano elemental | olho arcano com chama |
| `kindling` | Brasa Viva | 3 | 0 | arcane_power | 4 | R | +6% dano de área [N] | brasa sendo soprada |
| `ignite` | Ignição | 4 | 0 | kindling, clarity | 3 | R | Acertos da skill **queimam**: 30% do dano em 4 s [S] | fósforo aceso |
| `wider_blast` | Explosão Ampla | 5 | 0 | ignite | 3 | C | Raio de área ×1,12 | círculos concêntricos |
| `inferno` | Inferno | 6 | 0 | wider_blast | 2 | E | +15% dano elemental; queimadura acumula 2× [S] | boca de fornalha |
| `scorched_earth` | Terra Calcinada | 7 | 0 | inferno | 3 | C | Skills em área deixam chão em chamas por 3 s (15% do dano/s) [G] | solo rachado em brasa |
| `fire_and_ice` | Fogo e Gelo | 8 | 0 | scorched_earth, deep_freeze | 3 | R | +20% dano contra alvos lentos ou queimando [S] | metade chama, metade cristal |
| `meteor_heart` | Coração de Meteoro | 9 | 0 | fire_and_ice | 1 | E | A cada 12 s, o próximo ataque básico explode (2,5 m, ×2 dano) [G] | rocha incandescente |
| `combustion` | Combustão | 10 | 0 | meteor_heart | 3 | R | Quem morre queimando explode: 25% da vida máx. do morto em 2 m (teto 4× seu dano) [S] | barril explodindo |
| `primal_heat` | Calor Primordial | 11 | 0 | combustion | 3 | R | +10% dano elemental, +5% dano | sol negro com coroa |
| `avatar_of_flame` | Avatar da Chama | 12 | 0 | primal_heat, spell_weaving | 1 | E | +30% dano elemental; após a skill, vira chama por 4 s: +20% movimento e queima quem toca (40% do dano/s) [G] | silhueta em chamas |
| `quick_mind` ★ | Mente Acelerada | 1 | 1 | — | 4 | C | +5% redução de recarga | ampulheta com asas |
| `frost_touch` | Toque Gélido | 2 | 1 | quick_mind | 3 | C | Skills deixam inimigos 15% mais lentos por 2 s [S] | floco de neve na palma |
| `clarity` | Clareza | 3 | 1 | frost_touch | 4 | R | +4% redução de recarga, +5% recurso máximo | olho de cristal |
| `shatter_frost` | Geada Estilhaçante | 4 | 1 | clarity, kindling | 3 | R | Alvos lentos recebem +10% dano da skill [S] | gelo partido |
| `echo_mind` | Mente em Eco | 5 | 1 | shatter_frost | 3 | C | 8% de chance da skill não entrar em recarga [G] | duas cabeças em perfil |
| `overload` ★ | Sobrecarga | 6 | 1 | echo_mind | 1 | E | +25% dano elemental e +10% redução de recarga | núcleo rachado vazando luz |
| `deep_freeze` | Congelamento | 7 | 1 | overload | 2 | C | 3ª skill seguida no mesmo alvo congela 1 s (chefes: 30% lentidão) [S] | bloco de gelo com olho |
| `time_dilation` | Dilatação Temporal | 8 | 1 | deep_freeze, efficient_casting | 3 | R | Esquivar reduz a recarga da skill em 0,5 s [G] | ampulheta deitada |
| `winter_crown` | Coroa do Inverno | 9 | 1 | time_dilation | 1 | E | Aura de 3 m: inimigos −20% movimento e −10% dano [S] | coroa de pingentes |
| `focus_matrix` | Matriz de Foco | 10 | 1 | winter_crown | 3 | R | +3% redução de recarga por skill usada nos últimos 6 s (máx. 3) [G] | hexágonos encaixados |
| `mental_fortress` | Fortaleza Mental | 11 | 1 | focus_matrix | 2 | R | +15% resistência a controle, +5% redução de recarga | torre com olho |
| `absolute_zero` | Zero Absoluto | 12 | 1 | mental_fortress, primal_heat | 1 | E | Teto de redução de recarga 60% → 70% [N]; skills congelam comuns por 0,8 s (1×/6 s por alvo) [S] | esfera de gelo perfeita |
| `energy_flow` ★ | Fluxo de Energia | 1 | 2 | — | 4 | C | Regeneração de recurso ×1,2 | rio azul em espiral |
| `swift_channel` ★ | Canalização Rápida | 2 | 2 | quick_mind, energy_flow | 3 | R | Velocidade de ataque ×1,1 | mão com fios de energia |
| `deep_well` | Poço Profundo | 3 | 2 | swift_channel | 4 | R | +10% recurso máximo | poço com estrelas no fundo |
| `mana_shield` | Escudo de Mana | 4 | 2 | deep_well, static_charge | 2 | R | 15% do dano recebido é pago com recurso (1 recurso = 2 de vida) [G] | bolha com runas |
| `siphon` | Sifão | 5 | 2 | mana_shield | 3 | C | Acertos da skill devolvem 4% do recurso máximo [G] | funil de luz |
| `arcane_surge` | Surto Arcano | 6 | 2 | siphon | 2 | E | Com recurso acima de 80%: +15% dano [G] | barra transbordando |
| `efficient_casting` | Conjuração Eficiente | 7 | 2 | arcane_surge | 4 | C | −6% custo de recurso da skill [N] | pergaminho e tesoura |
| `leyline` | Linha de Ley | 8 | 2 | efficient_casting, arc_flash | 3 | R | Parado por 1 s cria círculo de 2 m: +30% regeneração de recurso e +10% vel. de ataque [G] | linhas luminosas no chão |
| `infinite_reservoir` | Reservatório Infinito | 9 | 2 | leyline | 1 | E | A cada 20 s, a próxima skill é grátis e causa +25% dano [G] | jarro sem fundo |
| `overflow` | Transbordar | 10 | 2 | infinite_reservoir | 3 | R | Recurso acima do máximo vira escudo (teto 6% da vida máx.) [G] | cálice derramando luz |
| `spell_weaving` | Tecer Feitiços | 11 | 2 | overflow | 3 | R | +8% vel. de ataque, regeneração de recurso ×1,08 | fios entrelaçados |
| `archmage_soul` | Alma de Arquimago | 12 | 2 | spell_weaving, mental_fortress | 1 | E | Recurso máx. ×1,25; cada 1% de recurso gasto dá +0,5% dano por 3 s (máx. +25%) [G] | estrela de sete pontas com olho |
| `static_charge` | Carga Estática | 1 | 3 | — | 5 | C | 5% de chance do acerto soltar raio num inimigo próximo (40% do dano) [G] | faísca |
| `conductor` | Condutor | 2 | 3 | static_charge | 3 | C | Raios saltam para +1 alvo [G] | para-raios |
| `storm_call` | Chamado da Tempestade | 3 | 3 | conductor | 3 | R | +6% dano elemental, +4% chance de raio [N] | nuvem negra |
| `thunderclap` | Trovoada | 4 | 3 | storm_call, deep_well | 3 | R | Raios atordoam comuns por 0,2 s [S] | mãos batendo com raio |
| `ozone` | Ozônio | 5 | 3 | thunderclap | 3 | C | +5% movimento; raios devolvem 2% do recurso | ar ondulado |
| `ball_lightning` | Relâmpago Globular | 6 | 3 | ozone | 2 | E | A cada 10 s, esfera orbita você por 5 s (30% do dano/s ao tocar) [G] | esfera crepitante |
| `arc_flash` | Arco Voltaico | 7 | 3 | ball_lightning | 4 | C | +15% dano de raio [N] | arco entre dois pontos |
| `supercell` | Supercélula | 8 | 3 | arc_flash, scorched_earth | 3 | R | Skills em área: 25% de chance de raio do céu (×1,5 dano) [G] | nuvem em funil |
| `eye_of_the_tempest` | Olho do Temporal | 9 | 3 | supercell | 1 | E | Raios encadeiam até 5 alvos sem perder dano [G] | olho dentro de um raio |
| `magnetize` | Magnetizar | 10 | 3 | eye_of_the_tempest | 3 | R | Acertos puxam comuns 0,5 m para o alvo principal [S] | ímã em ferradura |
| `grounding` | Aterramento | 11 | 3 | magnetize | 2 | R | +10% dano elemental, +10% chance de raio | estaca de cobre |
| `storm_god` | Deus da Tempestade | 12 | 3 | grounding, spell_weaving | 1 | E | Todo crítico de skill/área solta raio garantido; raios ×2 em chefes [G] | coroa de nuvem com raios |

### 5.4 Caminho da Defesa (48 nós)
Ramos: **0 Couraça** (armadura) · **1 Vitalidade** (vida e regeneração) · **2 Égide** (escudos) · **3 Inabalável** (controle, espinhos e contra-ataque).

| id | Nome | T | R | Pais | Máx | Rar. | Efeito | Ícone |
|---|---|---:|---:|---|---:|---|---|---|
| `plating` ★ | Couraça | 1 | 0 | — | 5 | C | +10 de armadura | placa rebitada |
| `riveted_mail` | Malha Rebitada | 2 | 0 | plating | 4 | C | +8 de armadura, +3% vida máxima | malha de anéis |
| `shield_wall` | Parede de Escudos | 3 | 0 | riveted_mail | 3 | R | +20 de armadura com 3+ inimigos a até 4 m [G] | escudos lado a lado |
| `deflection` | Deflexão | 4 | 0 | shield_wall, second_wind | 3 | R | 8% de chance de bloquear projéteis (−75% dano) [G] | flecha partindo no escudo |
| `heavy_plates` | Placas Pesadas | 5 | 0 | deflection | 3 | C | +25 de armadura, −3% movimento | bigorna |
| `bulwark` ★ | Baluarte | 6 | 0 | heavy_plates | 1 | E | Vida ×1,2 e +20 de armadura | muralha com escudo |
| `iron_hide` | Couro de Ferro | 7 | 0 | bulwark | 4 | C | −6% dano de contato recebido [N] | pele com escamas metálicas |
| `unbreakable_line` | Linha Inquebrável | 8 | 0 | iron_hide, overshield | 3 | R | Aura: aliados a até 5 m ganham +10 de armadura [G] | bandeira fincada |
| `fortress` | Fortaleza | 9 | 0 | unbreakable_line | 1 | E | Armadura ×1,25; quem te ataca corpo a corpo fica 10% mais lento [S] | castelo de uma torre |
| `reactive_plating` | Blindagem Reativa | 10 | 0 | fortress | 3 | R | Golpe acima de 10% da vida máx.: +40 de armadura por 4 s (recarga 8 s) [G] | placa com olho aberto |
| `tempered` | Temperado | 11 | 0 | reactive_plating | 3 | R | +20 de armadura, +5% vida máxima | lâmina entre brasa e água |
| `living_citadel` | Cidadela Viva | 12 | 0 | tempered, vigor | 1 | E | 1% da armadura vira redução de dano (máx. +15%); −10% dano de chefes [G] | gigante de pedra ajoelhado |
| `regeneration` ★ | Regeneração | 1 | 1 | — | 5 | C | +4 de vida por segundo | folha com gota |
| `vitality` ★ | Vitalidade | 2 | 1 | regeneration | 4 | C | Vida máxima ×1,08 | coração robusto |
| `second_wind` | Segundo Fôlego | 3 | 1 | vitality | 3 | R | Abaixo de 40% de vida: regeneração ×3 por 5 s (recarga 30 s) [G] | pulmões em espiral |
| `troll_blood` | Sangue de Troll | 4 | 1 | second_wind, shield_wall | 3 | R | +0,4% da vida máx. por segundo [N] | veia verde pulsando |
| `hearty` | Robusto | 5 | 1 | troll_blood | 4 | C | +6% vida máxima | caneca de cerveja |
| `undying` | Imorredouro | 6 | 1 | hearty | 1 | E | 1× por onda (ou a cada 120 s no mundo): golpe fatal te deixa com 1 de vida e 1,5 s invulnerável [G] | caveira com coração batendo |
| `herbal_lore` | Saber das Ervas | 7 | 1 | undying | 4 | C | +25% de cura recebida [N] | ramo de ervas |
| `rally_heart` | Coração Firme | 8 | 1 | herbal_lore, overshield | 3 | R | Cura acima do máximo vira escudo de 30% do excedente [G] | coração com escudo |
| `phoenix_ember` | Brasa da Fênix | 9 | 1 | rally_heart | 1 | E | Ao cair, revive após 4 s com 40% de vida (1× por run / 10 min) [G] | pena em chamas |
| `vigor` | Vigor | 10 | 1 | phoenix_ember | 3 | R | +8% vida máxima, +3 de regeneração | punho erguido |
| `blood_pump` | Bomba de Sangue | 11 | 1 | vigor | 3 | R | Regeneração dobrada por 3 s após esquiva [G] | coração mecânico |
| `eternal_oak` | Carvalho Eterno | 12 | 1 | blood_pump, runic_mastery | 1 | E | Vida máx. ×1,15; regeneração +1% para cada 1% de vida perdida (máx. +100%) [G] | árvore com tronco de coração |
| `ward_rune` | Runa de Proteção | 1 | 2 | — | 5 | C | Escudo de 40 no início da onda (a cada 30 s no mundo) | runa circular |
| `barrier` ★ | Barreira | 2 | 2 | ward_rune, regeneration | 3 | R | Escudo de 120 no início de cada onda | domo translúcido |
| `shield_battery` | Bateria de Escudo | 3 | 2 | barrier | 4 | R | Escudos recebidos ×1,15 (`shield_amount`) | cilindro com raio |
| `reflective_ward` | Égide Refletora | 4 | 2 | shield_battery, thorns | 3 | R | Com escudo ativo, devolve 15% do dano corpo a corpo [G] | espelho com olho |
| `aegis_pulse` | Pulso da Égide | 5 | 2 | reflective_ward | 3 | C | Escudo que quebra explode: 50% do valor como dano em 2,5 m [G] | escudo rachando com onda |
| `bastion_core` | Núcleo do Bastião | 6 | 2 | aegis_pulse | 2 | E | Sem escudo: ganha 8% da vida máx. em escudo a cada 12 s [G] | cristal hexagonal |
| `overshield` | Sobrescudo | 7 | 2 | bastion_core | 4 | C | +20% no teto de escudo acumulado [N] | dois escudos sobrepostos |
| `guardian_angel` | Anjo da Guarda | 8 | 2 | overshield, herbal_lore | 3 | R | Abaixo de 25% de vida: escudo de 15% da vida máx. (recarga 40 s) [G] | asa sobre escudo |
| `prismatic_ward` | Égide Prismática | 9 | 2 | guardian_angel | 1 | E | Com escudo ativo: −20% dano elemental e de projéteis [N] | prisma |
| `kinetic_ward` | Égide Cinética | 10 | 2 | prismatic_ward | 3 | R | Esquivar dá escudo de 3% da vida máx. (acumula 3) [G] | pegadas com faíscas |
| `runic_mastery` | Maestria Rúnica | 11 | 2 | kinetic_ward | 3 | R | Escudos ×1,15, +8 de armadura | runas em anel |
| `eternal_aegis` | Égide Eterna | 12 | 2 | runic_mastery, last_stand | 1 | E | Escudos não expiram; 20% do escudo absorvido volta como vida [G] | escudo com olho de sol |
| `iron_will` | Vontade de Ferro | 1 | 3 | — | 5 | C | +6% resistência a controle | punho rompendo corrente |
| `tenacity` ★ | Tenacidade | 2 | 3 | plating, iron_will | 3 | R | +15% resistência a controle, +5 de armadura | rocha com raízes |
| `thorns` | Espinhos | 3 | 3 | tenacity | 4 | R | Devolve 10% do dano de contato recebido [G] | rosa com espinhos de ferro |
| `retaliation` | Retaliação | 4 | 3 | thorns, shield_battery | 3 | R | Ao ser atingido, 15% de chance do próximo ataque causar +50% [G] | punho em contragolpe |
| `steadfast` | Firme | 5 | 3 | retaliation | 3 | C | Imune a empurrões leves; −5% dano de projéteis [N] | bota cravada no chão |
| `vengeance` | Vingança | 6 | 3 | steadfast | 2 | E | +2% dano por golpe recebido (máx. 10, 5 s) [G] | olho flamejante |
| `juggernaut` | Rolo Compressor | 7 | 3 | vengeance | 4 | C | +8% resistência a controle; empurra comuns ao atravessá-los [G] | aríete |
| `taunt` | Provocação | 8 | 3 | juggernaut, iron_hide | 3 | R | A cada 10 s, atrai inimigos a até 6 m por 3 s e ganha +20 de armadura [S] | boca gritando |
| `immovable` | Imóvel | 9 | 3 | taunt | 1 | E | Imune a atordoamento e lentidão; −10% movimento [S] | montanha |
| `spiked_carapace` | Carapaça Espinhosa | 10 | 3 | immovable | 3 | R | Espinhos +10% e aplicam sangramento [S] | carapaça de besouro |
| `last_stand` | Última Resistência | 11 | 3 | spiked_carapace | 2 | R | Abaixo de 30% de vida: −25% dano recebido [G] | soldado ajoelhado |
| `titan_heart` | Coração de Titã | 12 | 3 | last_stand, tempered | 1 | E | 50% do dano recebido é adiado por 4 s (dá para curar antes); +20% resistência a controle [G] | coração de pedra com veios dourados |

### 5.5 Caminho do Apoio (48 nós)
Ramos: **0 Comando** (auras de dano) · **1 Cura** · **2 Proteção** (escudos em área) · **3 Tática** (recarga, velocidade, coordenação).
Habilidades ativas (`ability`): 4 atuais + 4 novas (marcadas ◆).

| id | Nome | T | R | Pais | Máx | Rar. | Efeito | Ícone |
|---|---|---:|---:|---|---:|---|---|---|
| `support_power` ★ | Potência de Apoio | 1 | 0 | — | 5 | C | +15% efeito das habilidades de apoio | punho com auréola |
| `lost_warrior_aura` ★ | Aura do Guerreiro Perdido | 2 | 0 | support_power, rallying_pulse | 1 | R | Habilidade: +30% de dano aos aliados próximos por 6 s | elmo fantasma em chamas |
| `inspiring_presence` | Presença Inspiradora | 3 | 0 | lost_warrior_aura | 4 | R | Aura passiva: aliados a até 6 m +4% dano [G] | elmo com plumas |
| `aura_mastery` | Maestria de Aura | 4 | 0 | inspiring_presence, lifebloom | 3 | R | Buffs de apoio duram +1 s [N] | anel de luz em volta do elmo |
| `command_voice` | Voz de Comando | 5 | 0 | aura_mastery | 3 | C | +10% efeito de apoio, raio ×1,05 | corneta |
| `commander` ★ | Comandante | 6 | 0 | command_voice | 1 | E | +30% de efeito e raio ×1,25 das habilidades de apoio | bastão com estrela |
| `war_banner` ◆ | Estandarte de Guerra | 7 | 0 | commander | 1 | R | Habilidade: estandarte por 10 s, raio 5 m: aliados +15% dano e +15 de armadura. Recarga 24 s | estandarte rasgado |
| `vanguard` | Vanguarda | 8 | 0 | war_banner, field_tactics | 3 | R | Aliados sob seus buffs: +8% movimento [G] | botas aladas |
| `heroic_legacy` | Legado Heroico | 9 | 0 | vanguard | 1 | E | Aura do Guerreiro Perdido também dá +10% chance crítica [G] | espada fincada numa lápide |
| `shared_glory` | Glória Compartilhada | 10 | 0 | heroic_legacy | 3 | R | Abates de aliados no raio reduzem suas recargas de apoio em 0,3 s [G] | coroa de louros |
| `marshal` | Marechal | 11 | 0 | shared_glory | 3 | R | +15% efeito de apoio | bastão de marechal |
| `living_legend` | Lenda Viva | 12 | 0 | marshal, renewal | 1 | E | Cada buff de apoio ativo dá +5% dano ao esquadrão (acumula); raio ×1,2 [G] | estátua de herói com olhos acesos |
| `rallying_pulse` ★ | Pulso Revigorante | 1 | 1 | — | 1 | C | Habilidade: cura 18% da vida máx. dos aliados próximos | coração com ondas |
| `mending` | Remendo | 2 | 1 | rallying_pulse | 4 | C | +10% cura das habilidades de apoio [N] | agulha e linha de luz |
| `lifebloom` | Flor da Vida | 3 | 1 | mending | 3 | R | Pulso Revigorante deixa cura contínua de 6% em 4 s [G] | flor desabrochando |
| `triage` | Triagem | 4 | 1 | lifebloom, bulwark_oath | 3 | R | Curas em aliados abaixo de 35% são 30% maiores [G] | cruz com relógio |
| `field_medic` | Médico de Campo | 5 | 1 | triage | 3 | C | Reerguer aliados caídos 30% mais rápido [G] | maleta de couro |
| `sanctuary` ◆ | Santuário | 6 | 1 | field_medic | 1 | E | Habilidade: círculo de 4 m por 5 s: aliados curam 5% da vida máx./s e sofrem −30% dano. Recarga 30 s | círculo com pilares |
| `purify` | Purificar | 7 | 1 | sanctuary | 3 | C | Curas removem 1 efeito negativo (lentidão, sangramento, veneno) [S] | gota cristalina |
| `vital_link` | Elo Vital | 8 | 1 | purify, resilient_squad | 3 | R | 10% do seu dano cura o aliado mais ferido no raio [G] | corações ligados |
| `miracle` | Milagre | 9 | 1 | vital_link | 1 | E | Pulso Revigorante reergue caídos no raio com 20% de vida (recarga própria 90 s) [G] | mão com raio de luz |
| `overflowing_grace` | Graça Transbordante | 10 | 1 | miracle | 3 | R | Cura excedente vira escudo (teto 10% da vida máx. do aliado) [G] | ânfora com luz |
| `renewal` | Renovação | 11 | 1 | overflowing_grace | 3 | R | −10% recarga das curas, +10% cura | serpente verde mordendo a cauda |
| `saint_of_the_lost` | Santo dos Desesperados | 12 | 1 | renewal, iron_vow | 1 | E | Aliado abaixo de 50%: sua regeneração passa para ele ×4; curas de apoio ×1,3 [G] | auréola sobre mão aberta |
| `guardian_bond` ★ | Vínculo do Guardião | 1 | 2 | — | 3 | C | +8 de armadura e +2 de vida por segundo | duas mãos entrelaçadas |
| `shared_bastion` ★ | Bastião Compartilhado | 2 | 2 | guardian_bond | 1 | R | Habilidade: escudo de 160 em área para o esquadrão | escudo com quatro pontas |
| `bulwark_oath` | Juramento do Baluarte | 3 | 2 | shared_bastion | 4 | R | Escudos em área ×1,15 | mão sobre escudo |
| `interpose` | Interpor | 4 | 2 | bulwark_oath, drill_sergeant | 3 | R | 15% do dano de aliados a até 4 m vem para você, com −50% [G] | silhueta à frente de outra |
| `stone_skin` | Pele de Pedra | 5 | 2 | interpose | 3 | C | Bastião também dá +15 de armadura por 6 s [G] | punho de pedra |
| `ward_totem` ◆ | Totem Guardião | 6 | 2 | stone_skin | 1 | E | Habilidade: totem por 12 s, raio 4 m: aliados −20% dano recebido; inimigos preferem atacá-lo. Recarga 26 s | totem entalhado com olhos |
| `resilient_squad` | Esquadrão Resiliente | 7 | 2 | ward_totem | 4 | C | Aliados no raio: +8% resistência a controle [G] | quatro escudos em cruz |
| `sentinel` | Sentinela | 8 | 2 | resilient_squad, purify | 3 | R | Totem e Bastião também curam 2% da vida máx./s [G] | farol |
| `aegis_of_many` | Égide de Muitos | 9 | 2 | sentinel | 1 | E | Escudos de apoio: −1% dano por aliado no raio (máx. 4) e +3 s de duração [G] | escudo com quatro rostos |
| `rebound` | Ricochete | 10 | 2 | aegis_of_many | 3 | R | Escudo de apoio que quebra devolve 30% a quem o deu [G] | bumerangue de luz |
| `iron_vow` | Voto de Ferro | 11 | 2 | rebound | 3 | R | Escudos de apoio ×1,15, raio ×1,1 | anel de ferro |
| `wall_of_the_lost` | Muralha dos Perdidos | 12 | 2 | iron_vow, strategist | 1 | E | 1× a cada 60 s: aliado no raio que levaria golpe fatal fica 2 s invulnerável e você ganha escudo de 20% [G] | muralha com soldados fantasmas |
| `tactical_recharge` ★ | Recarga Tática | 1 | 3 | — | 4 | C | −8% recarga das habilidades de apoio | engrenagem com seta |
| `battle_cry` ★ | Grito de Batalha | 2 | 3 | tactical_recharge | 1 | R | Habilidade: +25% vel. de ataque aos aliados próximos por 6 s | boca rugindo |
| `drill_sergeant` | Sargento | 3 | 3 | battle_cry | 4 | R | Aura: aliados próximos +5% vel. de ataque [G] | apito |
| `coordinated_strike` | Golpe Coordenado | 4 | 3 | drill_sergeant, bulwark_oath | 3 | R | Inimigo atingido por 2+ aliados em 2 s recebe +10% dano [S] | alvo com duas setas |
| `field_tactics` | Táticas de Campo | 5 | 3 | coordinated_strike | 3 | C | −6% recarga de apoio, +6% movimento | mapa enrolado |
| `tactical_call` ◆ | Chamado Tático | 6 | 3 | field_tactics | 1 | E | Habilidade: reinicia a esquiva dos aliados no raio e dá +30% movimento por 4 s. Recarga 20 s | trombeta com setas |
| `logistics` | Logística | 7 | 3 | tactical_call | 4 | C | Consumíveis do Cinto também afetam 1 aliado próximo com 50% do efeito [G] | carroça de suprimentos |
| `flanking_orders` | Ordens de Flanco | 8 | 3 | logistics, war_banner | 3 | R | Aliados +10% chance crítica contra inimigos que focam outro jogador [G] | setas contornando um alvo |
| `momentum_doctrine` | Doutrina do Ímpeto | 9 | 3 | flanking_orders | 1 | E | Cada habilidade de apoio usada reduz as outras em 1,5 s [G] | engrenagem com asas |
| `quartermaster` | Intendente | 10 | 3 | momentum_doctrine | 3 | R | Fim de onda / a cada 60 s: 20% de chance de devolver 1 consumível usado [G] | balança com sacos |
| `strategist` | Estrategista | 11 | 3 | quartermaster | 3 | R | −8% recarga de apoio, +10% duração de apoio | torre de xadrez |
| `grand_tactician` | Grande Tático | 12 | 3 | strategist, marshal | 1 | E | Habilidades de apoio com 2 cargas; teto de redução de recarga de apoio 50% → 65% [G] | tabuleiro com bandeiras |

### 5.6 Exemplo de definição (compatível com `CardDefinition`)
```ts
{ id: 'weak_spot', name: 'Ponto Fraco',
  description: 'Críticos marcam o alvo por 4 s: +6% de dano recebido de todo o esquadrão.',
  tree: 'critical', rarity: 'rare', weight: 5, max_stacks: 3,
  modifiers: [mod('crit_mark_vuln', 'add', 0.06)],
  branch: 0, tier: 4, parents: ['steady_hand', 'blood_scent'] },
```
Para nós de gatilho [G] o `modifiers` guarda o número e o servidor/cliente lê o stat no ponto do gatilho (como já acontece com `crit_heal_pct` e `crit_cooldown_refund`).

### 5.7 Interface da árvore
- A aba de cada caminho vira um **grafo rolável** (4 colunas × 12 linhas), com linhas entre pais e filhos e brilho nos nós adquiridos. Nós sem pai adquirido aparecem dessaturados com cadeado.
- Filtro por ramo (os 4 nomes no topo da aba) e um mini-mapa lateral.
- Os ícones seguem o conceito da coluna "Ícone" (silhueta única, 1 cor de destaque por caminho: Crítico vermelho-sangue, Dano Mágico azul-violeta, Defesa aço-dourado, Apoio verde-esmeralda).


---

## 6. Expansão MMORPG: Crônicas do Império (mundo aberto)

> **Status:** expansão futura. Só entra em desenvolvimento depois do lançamento do Dungeon Stance, se houver aceitação da comunidade (Fase 4 de [`PLANO_DUNGEON_STANCE_UE5.md`](PLANO_DUNGEON_STANCE_UE5.md)). As seções 6 a 16 descrevem o conteúdo dessa expansão.

> **Nome provisório:** *Crônicas do Império*. Alternativas: *Império*, *Jornada*, *Terras do Império* (ver Q2).

### 6.1 Pilares
1. **Mapa muito maior** que a arena da Debandada (cerca de **50×** a área, ver §9.1) e **duração indefinida**: o mundo persiste, você entra e sai quando quiser.
2. **Habilidades no estilo RuneScape** (todas, menos Invenção e Divinação), com nível 1–50, separadas em **3 ramos: Coleta, Combate e Suporte** (§7). O **Painel de Habilidades** só existe neste modo (tecla **H** / botão Select segurado).
3. **Horror / dark fantasy:** o Império é decadente. Ciclo dia/noite (24 min reais = 1 dia), com noites mais perigosas (monstros +20% vida e dano, criaturas exclusivas da noite, névoa), luz de tocha importando em cavernas.
4. **Economia de jogadores:** tudo o que se equipa ou consome é **coletado, processado e fabricado** por jogadores, e negociado na **Bolsa Imperial** (§13).
5. **Herança da Debandada:** os mesmos 9 heróis, os mesmos caminhos (agora permanentes), os mesmos monstros, e **Incursões**: eventos de mundo em que uma horda ataca uma fazenda ou um portão e o grupo defende em ondas (uma Debandada dentro do mundo aberto).

### 6.2 Ciclo principal (core loop)
```
 explorar ─► coletar (Mineração, Lenhador, Pesca, Caça, Agricultura, Arqueologia)
    ▲                         │
    │                         ▼
 missões / conquistas ◄─ processar e fabricar (Metalurgia, Culinária, Arcoaria, Artesanato...)
    ▲                         │
    │                         ▼
 masmorras e chefes ◄── equipar / vender na Bolsa ──► ouro ──► comprar / taxas / construção
```

### 6.3 Estrutura de sessão
- **Entrar:** escolhe a classe → aparece no último ponto seguro (banco/acampamento/portão).
- **Sair:** a qualquer momento fora de combate (10 s de "acampar"); em combate, o personagem fica 30 s no mundo (evita fuga de morte).
- **Morte:** volta ao último **Santuário** visitado; Mochila fica num túmulo por 10 min (§4.2).
- **Escala do grupo:** solo, dupla, esquadrão de até 4 (online ou offline com companheiros IA, como na Debandada). Vida dos monstros escala com o grupo: `vida × (1 + 0,6·(n − 1))`.

### 6.4 Controles extras do modo
| Ação | Teclado/mouse | Gamepad |
|---|---|---|
| Interagir / coletar (segurar para repetir) | **E** — passa as habilidades de apoio para **Q/R/F/G** | A (fora de combate) |
| Painel de Habilidades | **H** | segurar Select |
| Inventário (Mochila + Cofre) | **I** / Tab | Start → Inventário |
| Mapa do reino | **M** | Start → Mapa |
| Diário (missões e conquistas) | **J** | Start → Diário |

---

## 7. Habilidades de mundo (estilo RuneScape) em 3 ramos

### 7.1 As 28 habilidades
Todas as habilidades do RuneScape (OSRS + RS3) **menos Invenção e Divinação**. Necromancia e Navegação foram incluídas: Necromancia combina com a classe Necromante, e Navegação fica como habilidade **futura** (destravada com o 2º reino).

| Ramo | id | Nome | Referência RS | Como treina | Para que serve |
|---|---|---|---|---|---|
| **Combate** | `attack` | Ataque | Attack | lutando corpo a corpo (postura Precisa) | +0,2% chance crítica e +0,4% vel. de ataque por nível; requisito de armas corpo a corpo |
| Combate | `strength` | Força | Strength | corpo a corpo (postura Agressiva) | +3% de dano corpo a corpo por nível; requisito de armas pesadas |
| Combate | `defence` | Defesa | Defence | dano **mitigado** pela armadura e golpes bloqueados | +1,5 de armadura por nível; requisito de armaduras |
| Combate | `ranged` | Pontaria | Ranged | ataques à distância físicos (Caçador, Arqueiro) | +3% de dano à distância por nível; requisito de arcos, bestas e munição |
| Combate | `magic` | Magia | Magic | dano mágico (Arcanista, Mago) e feitiços utilitários | +3% de dano mágico por nível; teleportes, Transmutação Áurea, encantamentos |
| Combate | `necromancy` | Necromancia | Necromancy (RS3) | dano de invocações e maldições (Necromante) e rituais | +3% de dano de invocações/maldições por nível; +1 invocação nos níveis 25 e 50 |
| Combate | `constitution` | Constituição | Hitpoints/Constitution | 30% de todo XP de combate | +4% de vida máxima por nível |
| Combate | `devotion` | Devoção | Prayer | ossos e cinzas oferecidos em altares | orações (bônus temporários que gastam pontos de Devoção) |
| Combate | `slaying` | Exterminação | Slayer | tarefas dos Mestres Exterminadores | ferir monstros especiais, +15% dano em alvos da tarefa, loja de recompensas |
| Combate | `delving` | Masmorrismo | Dungeoneering | andares das masmorras instanciadas | chaves, portas de atalho, "fichas" para recompensas de masmorra |
| **Coleta** | `mining` | Mineração | Mining | veios de minério e pedreiras | minérios, pedras, gemas, essência |
| Coleta | `woodcutting` | Lenhador | Woodcutting | árvores | toras + **casca** (subproduto) |
| Coleta | `fishing` | Pesca | Fishing | pontos de pesca (rede, vara, gaiola, arpão) | peixes crus |
| Coleta | `hunter` | Caça | Hunter | rastros e armadilhas | peles, penas, carne, ingredientes raros |
| Coleta | `farming` | Agricultura | Farming | canteiros (plantar, regar, colher em tempo real) | grãos, legumes, ervas, frutas, linho, madeira de pomar |
| Coleta | `archaeology` | Arqueologia | Archaeology (RS3) | sítios de escavação | artefatos, materiais antigos, relíquias |
| **Suporte** | `smithing` | Metalurgia | Smithing | fundir na fornalha e forjar na bigorna | barras, armas, armaduras, ferramentas, pontas de flecha |
| Suporte | `cooking` | Culinária | Cooking | fogões e fogueiras | comida (cura) e banquetes (bônus do grupo) |
| Suporte | `firemaking` | Fogaréu | Firemaking | acender toras, tochas, lanternas, braseiros | fogueiras com bônus, luz em cavernas, carvão vegetal |
| Suporte | `fletching` | Arcoaria | Fletching | entalhar madeira, emplumar flechas | arcos, bestas, flechas, virotes, cabos de cajado |
| Suporte | `crafting` | Artesanato | Crafting | couro, tecido, vidro, cerâmica, joias, restauração | armaduras leves/médias, joias, frascos, artefatos restaurados |
| Suporte | `runecrafting` | Runografia | Runecrafting | essência em altares rúnicos | runas para feitiços, encantamentos e rituais |
| Suporte | `herblore` | Herbologia | Herblore | limpar ervas e misturar poções | poções, antídotos, venenos de arma |
| Suporte | `construction` | Construção | Construction | tábuas, pregos e pedra na sua **Fortaleza** | acampamento com bancadas, altar, portais, baú do Cofre |
| Suporte | `agility` | Agilidade | Agility | circuitos e atalhos | atalhos no mapa; −1% de recarga de esquiva a cada 5 níveis (máx. −10%) |
| Suporte | `thieving` | Ladinagem | Thieving | bater carteira, barracas, baús, fechaduras | ouro, itens raros, acesso a áreas trancadas |
| Suporte | `summoning` | Invocação | Summoning (RS3) | pactos com amuletos de criatura + runas | familiares: mula de carga (+espaços), lutadores, coletores |
| Suporte | `sailing` | Navegação *(futura)* | Sailing (OSRS) | barcos e rotas | viajar entre reinos; desligada até o 2º reino |

**Por que os ofícios (artesanato) ficam em Suporte?** Você pediu 3 ramos (coleta, combate, suporte). O que separa os ramos é **a origem do valor**:
- **Coleta** tira coisas brutas do mundo.
- **Combate** usa o corpo do herói contra monstros.
- **Suporte** transforma, abastece e viabiliza os outros dois: sem Metalurgia não há arma, sem Culinária não há cura, sem Agilidade e Construção não há atalho nem base.

Agilidade e Ladinagem são "suporte" no próprio RuneScape, o que reforça a escolha. Se preferirem, a alternativa é um sub-ramo **Produção** dentro de Suporte (só visual). Ver Q6.

### 7.2 Progressão por ramo
- Cada habilidade tem o **próprio nível (1–50)** e o próprio XP, como no RuneScape.
- Cada ramo tem um **Nível de Ramo** = `piso(média dos níveis das habilidades do ramo)`, com marcos a cada 5 níveis:

| Nível de Ramo | Coleta | Combate | Suporte |
|---:|---|---|---|
| 5 | +5% de chance de recurso dobrado | +2% de vida | −5% tempo de fabricação |
| 10 | Ferramentas gastam 10% menos durabilidade | Pontos de Caminho +1 (bônus único) | +5% de chance de economizar 1 material |
| 15 | Ver recursos raros no minimapa | +3% de dano | Receitas em lote (×5) |
| 20 | +5% de velocidade de coleta | Santuários curam 100% ao passar | Bancada portátil (1 uso/dia) |
| 25 | Saco de minério / cesto de peixes (+14 espaços só p/ recursos) | +5% de resistência a controle | +10% de qualidade (chance de item "Fino") |
| 30 | +10% de chance de recurso dobrado | Pontos de Caminho +1 | Taxa da Bolsa −0,25 ponto |
| 35 | Coleta automática enquanto parado (máx. 60 s) | +5% de vida | −10% tempo de fabricação |
| 40 | Encontrar ninhos/geodos/pérolas (raros) | +5% de dano | +5% de chance de economizar material |
| 45 | +10% de velocidade de coleta | Chefes dão 1 rolagem extra de saque por semana | Fabricar 1 item "Obra-Prima" por dia |
| 50 | Capa do Coletor-Mestre (cosmético + teleporte para recursos) | Capa do Campeão | Capa do Artífice |

- **Nível de Combate** (exibido sobre o personagem e usado para avisos de perigo das zonas):
  `NC = piso(0,20·Defesa + 0,20·Constituição + 0,05·Devoção + 0,55·Ofensivo)`, onde `Ofensivo = máx((Ataque + Força)/2, Pontaria, Magia, Necromancia)`. Tudo em 1 → NC 1; tudo em 50 → NC 50.
- **Nível Total** = soma dos 27 níveis ativos (máx. 1.350 sem Navegação). Usado em conquistas e portas especiais.

### 7.3 Curva de XP (até o nível 50)
Usamos a **mesma fórmula do RuneScape**, limitada ao nível 50 (a curva é conhecida, testada e fácil de explicar):

```
XP_total(L) = piso( Σ_{l=1}^{L−1} piso( l + 300 · 2^(l/7) ) / 4 )
```
Nível 50 = **101.333 XP**. A tabela completa está no [Apêndice A.1](#a1-xp-por-nível-1-50).

**Metas de tempo** (uma habilidade, jogando focado): nível 10 em ~15 min, 20 em ~45 min, 30 em ~1h40, 40 em ~3h, **50 em ~6h** (Coleta e Suporte) e ~7h (Combate). Com 27 habilidades, "maximizar" o primeiro reino leva ~170 h, mas cada sessão dá vários níveis.

**XP por ação (Coleta e Suporte)**, por faixa de recurso (T1 = nível 1, T2 = 10, T3 = 20, T4 = 30, T5 = 40, T6 = 50):

| Faixa | XP por ação | Tempo base da ação | XP/h teórico (sem viagens) |
|---|---:|---:|---:|
| T1 (1–9) | 10 | 6,0 s | 6.000 |
| T2 (10–19) | 20 | 6,5 s | 11.000 |
| T3 (20–29) | 35 | 7,0 s | 18.000 |
| T4 (30–39) | 55 | 7,5 s | 26.400 |
| T5 (40–49) | 80 | 8,0 s | 36.000 |
| T6 (50) | 110 | 8,5 s | 46.600 |

`tempo_real = tempo_base × (1 − 0,01·(nível − nível_requerido)) × mult_ferramenta`, com mínimo de 55% do tempo base. `mult_ferramenta` por metal: Bronze 1,00 · Ferro 0,92 · Aço 0,85 · Auralita 0,78 · Vermécio 0,72 · Sombraço 0,66. Recursos intermediários (níveis 5, 15, 25...) usam XP e tempo interpolados entre as faixas.

**XP de combate:** `XP = arred(4 + vida_máx_do_monstro × 0,05)`, multiplicado pela ameaça (lacaio 1, elite 2,5, chefe 10). Distribuição: **70%** para a habilidade ofensiva da classe (melee divide entre Ataque e Força pela postura) e **30%** para Constituição. Defesa ganha XP à parte: **1 XP a cada 8 de dano mitigado** pela armadura. Em grupo, cada participante (deu dano ou cura nos últimos 10 s, a até 30 m) recebe `XP × (0,5 + 0,5 × fração da contribuição)`, para incentivar o co-op sem punir o suporte.

### 7.4 O que cada habilidade destrava (1–50)
Os números entre parênteses são o nível requerido. Nomes de recursos originais; o equivalente de referência do RS aparece só no Apêndice B.

#### Combate
| Habilidade | 1 | 5 | 10 | 15 | 20 | 25 | 30 | 35 | 40 | 45 | 50 |
|---|---|---|---|---|---|---|---|---|---|---|---|
| Ataque | armas T1 (bronze) | postura Precisa | armas T2 (ferro) | — | armas T3 (aço) | Golpe Certeiro (passivo: +2% crítico) | armas T4 (auralita) | — | armas T5 (vermécio) | Mestre de Armas (troca de arma instantânea) | armas T6 (sombraço) |
| Força | armas pesadas T1 | postura Agressiva | pesadas T2 | — | pesadas T3 | Esmagar (passivo: derruba comuns 0,2 s em 5%) | pesadas T4 | — | pesadas T5 | — | pesadas T6 + Capa |
| Defesa | armaduras T1 | postura Defensiva | armaduras T2 | escudos de torre | armaduras T3 | Aparar (bloqueio no dash +0,1 s) | armaduras T4 | — | armaduras T5 | — | armaduras T6 |
| Pontaria | arcos/bestas T1 | flechas de ferro | arcos T2 | flechas de fogo (com Fogaréu 15) | arcos T3 | Tiro Duplo (passivo 5%) | arcos T4 | — | arcos T5 | Aljava Sem Fim (economiza 20% munição) | arcos T6 |
| Magia | Faísca (feitiço base) | Teleporte: Portão da Capital | cajados T2 | Encantar joia I | cajados T3 | Transmutação Menor (item → 40% valor) | cajados T4; Teleporte: Forja | Encantar joia II | cajados T5; Transmutação Áurea (60%) | Teleporte: Fenda (perigo) | cajados T6 |
| Necromancia | foices/cajados de osso T1 | ritual Erguer Ossos | T2 | — | T3 | +1 invocação | T4 | Ritual da Lápide (orbe de almas) | T5 | — | T6, +1 invocação |
| Constituição | — | +4%/nível de vida | — | — | Regeneração fora de combate ×2 | — | — | — | — | — | Capa da Vitalidade |
| Devoção | Pele de Pedra (+10 armadura) | Força Interior (+5% dano) | Olhar Aguçado (+5% alcance) | Proteção contra Projéteis (−40%) | Proteção contra Magia (−40%) | Proteção Corpo a Corpo (−40%) | Redenção (cura 25% ao cair abaixo de 10%) | Fervor (+10% dano) | Piedade (+10% vida, +10% dano) | Comunhão (orações afetam o grupo a 50%) | Ascese (orações gastam 30% menos) |
| Exterminação | tarefas Fáceis | — | Luvas de Capturar (morcegos de cripta) | tarefas Médias | Martelo de Gárgula | Bolsa de Sal (lesmas-de-poço) | tarefas Difíceis | Espelho (Olho-de-Pedra) | Respirador (gás dos esgotos fundos) | tarefas de Chefe | Elmo do Exterminador (+15% → +20%) |
| Masmorrismo | andar 1–5 | — | andares 6–10 | chave de bronze | andares 11–15 | — | andares 16–20 | chave de prata | andares 21–25 | — | andares 26–30, Coroa do Abismo (cosmético) |

#### Coleta
| Habilidade | 1 | 5 | 10 | 15 | 20 | 25 | 30 | 35 | 40 | 45 | 50 |
|---|---|---|---|---|---|---|---|---|---|---|---|
| Mineração | Cobre, Estanho, Pedra-Essência | Argila Vermelha | Ferro-Negro | Prata | Carvão | Veio de Gemas | Auralita | Ouro | Vermécio | Granito-Sangue | Minério Sombrio (só na Fenda) |
| Lenhador | Pinheiro-Comum | Bétula-Pálida (casca p/ pergaminho) | Carvalho-Cinzento (casca de tanino) | Freixo | Salgueiro-Chorão (casca medicinal) | Cedro-Vermelho | Ácer-Rubro | Mogno-do-Brejo | Teixo-Negro | Ébano-Cinzento | Fantasmeira (Bosque dos Enforcados, à noite) |
| Pesca | Camarão-de-Rio (rede) | Sardinha (vara + isca) | Truta-Parda (vara de mosca + penas) | Lúcio | Salmão-Prateado | Enguia-de-Lodo (esgotos) | Lagosta-de-Rocha (gaiola) | Bacalhau-Negro | Peixe-Lâmina (arpão) | Arraia-Elétrica | Enguia-Abissal (Lago Afogado) |
| Caça | Coelho (laço) | Perdiz (laço) | Raposa-Cinza (caixa) | Marta | Javali (fosso) | Salamandra-de-Brasa (rede) | Cervo-de-Chifre-Branco | Lince-das-Sombras | Mariposa-Lunar (rede, noite) | Urso-Cinzento (fosso) | Cervo-Espectral (noite) |
| Agricultura | Batata, Trigo | Repolho, Cevada | Linho, Ervamarga | Macieira (pomar) | Lúpulo, Folha-de-Guarda | Carvalho plantado | Raiz-de-Ferro | Pereira-Sangue | Flor-de-Cinza, Musgo-Lunar | Teixo plantado | Lírio-Negro |
| Arqueologia | Ruínas do Mercado Antigo | cacos → Museu | Acampamento Legionário | — | Necrópole do Rei Antigo | — | Templo Afundado | — | Observatório Caído | — | Câmara do Primeiro Imperador |

#### Suporte
| Habilidade | 1 | 5 | 10 | 15 | 20 | 25 | 30 | 35 | 40 | 45 | 50 |
|---|---|---|---|---|---|---|---|---|---|---|---|
| Metalurgia | Barra de Bronze, itens T1 | ferramentas de bronze | Barra de Ferro, itens T2 | Barra de Prata | Barra de Aço, itens T3 | pontas de flecha de aço | Barra de Auralita, itens T4 | Barra de Ouro | Barra de Vermécio, itens T5 | Grande Fornalha (2 barras por carga) | Barra de Sombraço, itens T6 |
| Culinária | Camarão, Pão | Sardinha, Torta de Batata | Truta, Ensopado | Lúcio, Cerveja de Cevada | Salmão, Torta de Maçã | Enguia, Pão de Ervas | Lagosta | Bacalhau, Torta de Javali | Peixe-Lâmina | Banquete do Caçador (grupo) | Enguia-Abissal, Banquete Imperial |
| Fogaréu | Pinheiro, fogueira | Tocha | Carvalho (fogueira +regen) | Lanterna de óleo | Salgueiro, carvão vegetal | Braseiro de grupo | Ácer (fogueira +XP de Culinária) | Lanterna Selada (água) | Teixo, Fogo Sagrado (afasta mortos-vivos) | — | Fantasmeira: Chama Fria (revela fantasmas) |
| Arcoaria | arco curto de pinheiro, flechas de bronze | cabo de cajado | arco de carvalho, besta T2 | flechas de ferro | arco de salgueiro, besta T3 | — | arco de ácer, besta T4 | — | arco de teixo, besta T5 | flechas incendiárias | arco de fantasmeira, besta T6 |
| Artesanato | couro macio, cerâmica | anel de prata | couro curtido, vidro | amuleto de prata | linho → tecido; couro reforçado | joia com gema | couro de cervo, mantos T4 | joia de ouro | couro de lince, mantos T5 | restauração avançada | couro espectral, mantos T6, Diadema |
| Runografia | Runa de Brisa | Runa de Maré | Runa de Terra | Runa de Brasa | Runa da Mente | Runa do Corpo | Runa do Caos | Runa da Lei (teleportes) | Runa da Morte (Necromancia) | Bolsa Abissal | Runa de Sangue |
| Herbologia | Tônico de Vigor | Poção de Força | Antídoto | Poção de Defesa | Poção de Recurso | Veneno de Arma | Elixir de Pontaria | Elixir de Magia | Poção de Coragem (controle) | Antídoto Superior | Elixir Supremo do Império |
| Construção | Barraca, Fogueira de acampamento | Bancada de Carpinteiro | Oficina (bigorna portátil) | — | Baú do Acampamento (acesso ao Cofre) | Jardim (canteiros extras) | Altar (recarrega Devoção, +XP) | Despensa (comida não estraga) | Portal (teleportes do reino) | — | Salão dos Troféus (bônus por chefe exposto) |
| Agilidade | Circuito dos Telhados (Bairro Baixo) | atalho: muro da Forja | atalho: tronco sobre o rio | — | Circuito das Muralhas | atalho: corda do penhasco | Circuito da Cidadela | — | atalho: Fenda Estreita (masmorra) | — | Circuito do Campanário |
| Ladinagem | bater carteira: Cidadão | barraca de pão | Mercador | barraca de seda | Guarda | arrombar baús do Bairro Baixo | barraca de prata | portas trancadas (Catacumbas) | Nobre | barraca de gemas | Cofre do Tesoureiro Imperial (missão) |
| Invocação | Rato de Carga (+3 espaços) | — | Lobo-Cinzento (luta) | — | Mula de Carga (+8 espaços) | Coruja (coleta penas/ervas) | Javali de Guerra | — | Golem de Carga (+14 espaços) | — | Corvo do Abismo (busca saque automaticamente) |
| Navegação | *futura (2º reino)* | | | | | | | | | | |

### 7.5 Painel de Habilidades (UI)
- Três colunas — **Coleta**, **Combate**, **Suporte** — cada uma com o **Nível de Ramo** no topo e a barra dos marcos.
- Cada habilidade: ícone, nível, barra de XP até o próximo nível, XP que falta, e um botão "Guia" que abre a tabela do que ela destrava (§7.4), com o próximo desbloqueio em destaque.
- Rodapé: **Nível de Combate**, **Nível Total**, tempo jogado no modo.
- Notificação de nível: banner curto + som; no nível 50, fogos com a cor do ramo e anúncio no chat do reino.


---

## 8. Bestiário: monstros do nível 1 ao 50

Um único bestiário serve aos dois modos. **Crônicas do Império** usa os monstros pelo nível da zona. **Debandada** monta as ondas a partir dos mesmos monstros, filtrados pela arena e pelo Tormento (§2.2). Os 6 monstros atuais são herdados (★) e ganham nível e zona fixos.

### 8.1 Fórmulas de atributos
Atributos base por nível `L` (lacaio padrão):

| Atributo | Fórmula | L1 | L10 | L25 | L50 |
|---|---|---:|---:|---:|---:|
| Vida | `50 + 16·L + 0,5·L²` | 67 | 260 | 763 | 2.100 |
| DPS de contato | `12 + 4,5·L` | 17 | 57 | 125 | 237 |
| Dano de projétil | `2,2 × DPS` por disparo (recarga 2,6 s) | 37 | 125 | 275 | 521 |
| Armadura | `2·L` (mitigação para o jogador: `200/(200 + armadura)`) | 2 | 20 | 50 | 100 |
| XP de combate | `arred(4 + vida × 0,05)` | 7 | 17 | 42 | 109 |
| Ouro médio | `2 + 1,6·L + 0,05·L²` | 4 | 23 | 73 | 207 |

Tabela completa em [A.2](#a2-atributos-de-monstro-por-nível). Conferência com o atual: o Goblin Soldado (120 de vida, 25 DPS) bate com um lacaio de **nível 3** (103 de vida, 26 DPS), então os valores atuais da Debandada continuam válidos.

**Multiplicadores por arquétipo** (aplicados sobre a base):

| Arquétipo | Vida | DPS | Vel. (px/s) | Raio | Comportamento | Exemplo atual |
|---|---:|---:|---:|---:|---|---|
| Lacaio | ×1,0 | ×1,0 | 70 | 14 | persegue e bate | Goblin Soldado |
| Corredor | ×0,6 | ×0,7 | 130 | 11 | flanqueia, ataca em investida | Demônio Corredor |
| Brutamontes | ×3,7 | ×2,3 | 45 | 22 | lento, golpe em área telegrafado | Goblin Brutamontes |
| Atirador | ×0,75 | ×0,4 (contato) | 60 | 13 | mantém 340 px e dispara projétil esquivável | Esqueleto-Zumbi |
| Conjurador | ×0,8 | ×0,5 | 55 | 13 | área telegrafada no chão (×3 DPS, a cada 4 s), foge de perto | (novo) |
| Enxame | ×0,3 | ×0,35 | 110 | 9 | aparece em grupos de 4–8 | (novo) |
| Passivo | ×0,5 | ×0,3 | 60 | 12 | só revida; foge com pouca vida | (novo) |
| Elite | ×2,5 | ×1,5 | arquétipo | +4 | 1 habilidade especial, aura visível | (novo) |
| Chefe | ×30 | ×3,5 | 52 | 34 | fases (100/60/30%), ataques telegrafados, invoca | Senhor da Clareira |

**Tempo para matar (referência):** um herói de nível de combate 50 com equipamento T6 causa ~320 DPS (com críticos). Um lacaio de nível 50 (2.100 de vida, ×0,67 pela armadura) cai em ~10 s solo e ~3 s em esquadrão. No nível 1, ~2 s.

### 8.2 Famílias de rig (reaproveitando o que já existe)
| Família | Base atual reaproveitada | Monstros |
|---|---|---|
| **R1 Goblinóide** | sprite do goblin + animação procedural do `SpriteActor3D` (e o truque de escala/cor do brutamontes) | goblins, hobgoblin |
| **R2 Humanoide-Herói** | `ModularCharacter.gd` + `CharacterParts.gd` (o corpo de anão cabeçudo) + `ClassGear.gd` para armas/armaduras | Anão Renegado (reuso direto!), Andarilho, Bandido, Mago Oco, Druida, Cavaleiro e Comandante Corvo-Negro, Sentinela de Geada |
| **R3 Esquelético** | sprite do esqueleto/esqueleto-zumbi + o esqueleto procedural do lacaio do Necromante | Esqueleto Errante, Zumbi Afogado, Necrófago, Rei Sem Rosto |
| **R4 Quadrúpede** *(novo)* | esqueleto procedural novo: coluna em 3 segmentos + 4 pernas com IK de 2 ossos; malha via `MeshKit.gd` (perfis torneados) | rato, vaca, lobos, urso, javali, Cão de Cinza, Uivante (alterna bípede/quadrúpede) |
| **R5 Artrópode** *(novo)* | 6–8 pernas com IK procedural e passo alternado; corpo extrudado | aranhas, escorpiões, Rastejante, Sanguessuga (segmentos) |
| **R6 Espectral** *(novo, barato)* | sem pernas: malha de véu com shader de deslocamento e transparência; flutua | Fantasma-de-Lamparina, Carpideira, Lamentadora |
| **R7 Gigante** | R2 escalado ×2,5–3,5 com cabeça menor, braços longos; golems com blocos do `MeshKit` | gigantes, ogro, golems, demônios maiores |
| **R8 Alado/Serpentino** *(novo)* | asas com 2 ossos + curva de corpo em spline (tubos curvos do `MeshKit`) | Diabrete, Gárgula, Serpe Verde, Demônio Corredor (hoje sprite) |
| **R9 Ave pequena** | reaproveita R8 sem voo | Galinha-do-Charco |

Todos seguem o contrato atual: uma cena em `res://art/enemies/<id>.tscn` substitui o placeholder sem mudar código (frente para −Z, pés em y = 0).

### 8.3 Tabela-resumo (1–50)
RS = monstro de referência do RuneScape (só inspiração de papel e nível; nome, visual e mecânica são nossos).

| Nív | id | Nome | Arquétipo | Zona | Vida | DPS | RS (ref.) |
|---:|---|---|---|---|---:|---:|---|
| 1 | `sewer_rat` | Rato-de-Esgoto | Enxame | Galerias Afogadas | 20 | 6 | Giant rat |
| 1 | `bog_hen` | Galinha-do-Charco | Passivo | Campos do Moinho | 33 | 5 | Chicken |
| 2 | `crookhorn_cow` | Vaca-Chifre-Torto | Passivo | Campos do Moinho | 42 | 6 | Cow |
| 2 | `vagrant` | Andarilho Perdido | Lacaio | Estrada do Rei | 84 | 21 | Man |
| 3 | `goblin_grunt` ★ | Goblin Soldado | Lacaio | Clareira, Acampamento Goblin | 103 | 26 | Goblin |
| 4 | `grey_imp` | Diabrete Cinzento | Corredor | Campos (noite) | 73 | 21 | Imp |
| 5 | `well_spider` | Aranha-de-Poço | Enxame | Galerias Afogadas | 43 | 12 | Giant spider |
| 6 | `ford_bandit` | Bandido do Vau | Lacaio | Vau dos Bandidos | 164 | 39 | Bandit |
| 7 | `demon_runner` ★ | Demônio Corredor | Corredor | Clareira, Fenda (borda) | 112 | 30 | — |
| 8 | `zombie_shooter` ★ | Esqueleto-Zumbi Atirador | Atirador | Clareira, Cemitério | 158 | 19 (106/disparo) | — |
| 9 | `goblin_brute` ★ | Goblin Brutamontes | Brutamontes | Clareira, Acampamento Goblin | 868 | 121 | Hobgoblin (papel) |
| 10 | `stone_scorpion` | Escorpião-de-Pedra | Lacaio | Colinas de Ferrugem | 260 | 57 | Scorpion |
| 11 | `hollow_wizard` | Mago Oco | Conjurador | Torre Tombada | 229 | 31 (área 185) | Dark wizard |
| **12** | `glade_lord` ★ | **Senhor da Clareira** | **Chefe** | Clareira da Floresta | 9.420 | 231 | — |
| 13 | `drowned_zombie` | Zumbi Afogado | Lacaio | Galerias, Charco Cinzento | 343 | 71 | Zombie |
| 14 | `wandering_skeleton` | Esqueleto Errante | Lacaio | Cemitério dos Silenciosos | 372 | 75 | Skeleton |
| **15** | `mother_mange` | **Mãe Sarnenta** | **Chefe** | Galerias Afogadas (fundo) | 12.090 | 280 | Scurrius |
| 16 | `renegade_dwarf` | Anão Renegado | Lacaio | Mina do Velho Anão | 434 | 84 | Dwarf |
| 17 | `lantern_ghost` | Fantasma-de-Lamparina | Corredor (espectral) | Cemitério (noite) | 280 | 62 | Ghost |
| 18 | `scar_bear` | Urso-Cicatriz | Brutamontes | Mata dos Enforcados | 1.850 | 214 | Grizzly bear |
| 19 | `rot_druid` | Druida da Podridão | Conjurador | Círculo de Pedras | 428 | 49 (área 293) | Chaos druid |
| 20 | `hobgoblin_lancer` | Hobgoblin Lanceiro | Lacaio (alcance 2,5 m) | Colinas de Ferrugem | 570 | 102 | Hobgoblin |
| 21 | `flesh_crawler` | Rastejante-de-Carne | Enxame | Catacumbas da Coroa | 182 | 37 | Flesh crawler |
| 22 | `pale_wolf` | Lobo-Pálido | Corredor | Dentes do Norte | 386 | 78 | White wolf |
| 23 | `rubble_golem` | Golem de Entulho | Brutamontes | Mina do Velho Anão | 2.525 | 266 | Rock golem |
| 24 | `bog_leech` | Sanguessuga-do-Charco | Enxame | Charco Cinzento | 217 | 42 | Swamp leech |
| **25** | `stone_foreman` | **Capataz de Pedra** | **Chefe** | Mina do Velho Anão (fundo) | 22.890 | 438 | Giant Mole (papel) |
| 26 | `hill_giant` | Gigante-do-Outeiro | Brutamontes | Colinas de Ferrugem | 2.975 | 297 | Hill giant |
| 27 | `scarlet_weaver` | Tecelã Escarlate | Corredor | Caverna dos Ecos | 508 | 93 | Deadly red spider |
| 28 | `crowblack_knight` | Cavaleiro Corvo-Negro | Elite (lacaio) | Fortaleza Corvo-Negro | 2.225 | 207 | Black knight |
| 29 | `marsh_ogre` | Ogro do Charco | Brutamontes | Charco Cinzento | 3.458 | 328 | Ogre |
| **30** | `breu_widow` | **Viúva do Breu** | **Chefe** | Caverna dos Ecos (ninho) | 29.400 | 515 | Sarachnis |
| **30** | `obrum` | **Obrum, o Outeiro Vivo** | **Chefe de mundo** | Colinas (arena aberta) | 29.400 | 515 | Obor |
| 31 | `frost_sentinel` | Sentinela de Geada | Lacaio | Dentes do Norte | 1.027 | 152 | Ice warrior |
| 32 | `king_scorpion` | Rei-Escorpião | Elite (lacaio) | Fosso dos Escorpiões | 2.685 | 234 | King scorpion |
| 33 | `clay_warrior` | Guerreiro de Argila | Lacaio | Charco Cinzento | 1.123 | 161 | Earth warrior |
| 34 | `moss_giant` | Gigante Musgoso | Brutamontes | Mata dos Enforcados | 4.336 | 379 | Moss giant |
| **35** | `faceless_king` | **Rei Sem Rosto** | **Chefe** | Catacumbas da Coroa (cripta) | 36.690 | 595 | Skeletal king (papel) |
| 36 | `mourner` | Carpideira | Conjurador (espectral) | Catacumbas da Coroa | 1.019 | 87 (área 522) | Ankou |
| 37 | `rotmoon_howler` | Uivante da Lua Podre | Elite (corredor) | Mata (noite) | 1.990 | 187 | Werewolf |
| 38 | `gutter_gargoyle` | Gárgula de Calha | Lacaio (Exterminação 20) | Telhados da Cidadela (noite) | 1.380 | 183 | Gargoyle |
| **38** | `moss_mother` | **Mãe-Musgo** | **Chefe de mundo** | Bosque Velho | 41.400 | 641 | Bryophyta |
| 39 | `lamenter` | Lamentadora | Conjurador (espectral) | Catacumbas (fundo) | 1.148 | 94 (cone 563) | Banshee |
| 40 | `ember_demon` | Demônio Menor das Brasas | Brutamontes | Fenda Infernal (borda) | 5.513 | 442 | Lesser demon |
| 41 | `frost_giant` | Gigante de Geada | Brutamontes | Dentes do Norte | 5.722 | 452 | Ice giant |
| **42** | `crow_commander` | **Comandante Corvo-Negro** | **Chefe** | Fortaleza Corvo-Negro (torre) | 48.120 | 704 | Black Knight Titan (papel) |
| 43 | `ash_hound` | Cão de Cinza | Corredor (matilha 3) | Fenda Infernal | 998 | 144 | Hellhound |
| 44 | `crypt_ghoul` | Necrófago | Lacaio | Catacumbas profundas | 1.722 | 210 | Ghoul |
| 45 | `stone_eye` | Olho-de-Pedra | Atirador (Exterminação 35) | Caverna dos Ecos (fundo) | 1.337 | 86 (472/disparo) | Cockatrice/Basilisk |
| 46 | `green_wyrm` | Serpe Verde | Elite (brutamontes) | Marca Cinzenta | 17.057 | 756 | Green dragon |
| 47 | `obsidian_golem` | Golem de Obsidiana | Brutamontes | Fenda Infernal | 7.054 | 514 | — |
| 48 | `chain_demon` | Demônio Maior das Correntes | Elite (brutamontes) | Fenda Infernal | 18.223 | 787 | Greater demon |
| 49 | `rift_herald` | Arauto da Fenda | Elite (conjurador) | Fenda Infernal (portais) | 4.069 | 174 (área 1046) | — |
| **50** | `archduke_deadember` | **Arquiduque Brasa-Morta** | **Chefe final** | Coração da Fenda | 63.000 | 830 | — |

**Total:** 47 monstros comuns/elite + 9 chefes = **56 entradas**. Monstros de elite aplicam `vida ×2,5, DPS ×1,5` sobre o arquétipo entre parênteses; elites brutamontes (Serpe Verde, Demônio Maior) funcionam como **mini-chefes** pensados para grupo. Os números desta tabela saem direto das fórmulas de §8.1.

### 8.4 Fichas dos monstros
Formato: **Conceito · Silhueta · Comportamento e ataques · Saque · Arte (horror) · Rig.** Saque: *sempre* / *comum (1/4)* / *incomum (1/16)* / *raro (1/128)* / *único (1/512)*. Todo monstro também solta ouro (tabela 8.1) e, a partir do nível 15, **amuletos de pacto** para Invocação (1/20).

#### Faixa 1–10 (Campos do Moinho, Estrada do Rei, Galerias Afogadas, Clareira)
- **Rato-de-Esgoto (1)** · Ratos do tamanho de um gato, pelagem caindo em tufos, olhos leitosos · Corpo baixo e alongado, cauda pelada longa · Grupos de 6; mordida rápida; fogem da luz de tocha · Sempre: Osso Pequeno; comum: Rabo de Rato (Herbologia); raro: Anel Roído (prata) · Pele com feridas, brilho úmido; guinchos que ecoam · R4.
- **Galinha-do-Charco (1)** · Galinhas magras de penas cinza e crista escura · Ave pequena, pescoço comprido · Passiva; bica quem a ataca · Sempre: Penas ×5–15, Ossos; comum: Ovo · Olhos vermelhos à noite (só um detalhe inquietante) · R9.
- **Vaca-Chifre-Torto (2)** · Gado de chifres assimétricos e costelas aparentes · Quadrúpede pesado · Passiva; coice ao ser atacada · Sempre: Couro Cru, Carne Crua, Ossos · Mancha de mofo no couro, mugido grave distorcido · R4.
- **Andarilho Perdido (2)** · Ex-camponês enlouquecido pela fome · Humanoide magro com trapos e cajado · Golpes lentos; às vezes implora antes de atacar (falas) · Sempre: Ossos; comum: moedas, Pão Velho; incomum: Erva Ervamarga · Rosto sombreado, olhos fundos · R2.
- **Goblin Soldado ★ (3)** · Infantaria goblin das tribos da Clareira · Pequeno, orelhas largas, escudo de madeira · Persegue em grupo e cerca · Sempre: Ossos; comum: Lança de Bronze Quebrada, moedas; incomum: Cabeça de Flecha de Bronze ×10; raro: Totem Goblin (missão) · Sprite atual; nova versão 3D com pintura de guerra de lama · R1.
- **Diabrete Cinzento (4)** · Diabrete pequeno que rouba e some · Alado, cauda em gancho · Corre, morde e **teleporta** 6 m quando fica com pouca vida; à noite rouba 1 item comum da Mochila se não for morto em 20 s (devolve ao morrer) · Comum: Cinzas Infernais (Devoção), Conta Negra; raro: Anel do Diabrete (+2% movimento) · Riso infantil distorcido · R8.
- **Aranha-de-Poço (5)** · Aranhas de patas finas que descem do teto · 8 patas longas · Enxame de 5; cai do teto em emboscada · Comum: Olho de Aranha (Herbologia), Seda Crua (Artesanato) · Pelos que brilham na luz da tocha · R5.
- **Bandido do Vau (6)** · Desertores do exército imperial · Humanoide com capuz e besta leve · Metade corpo a corpo, metade atirador; foge para chamar reforço · Comum: moedas extras, Capuz Gasto; incomum: Besta T1; raro: Mapa do Esconderijo (missão "Pedágio de Sangue") · Bandana vermelha manchada · R2.
- **Demônio Corredor ★ (7)** · Demônios menores que escapam da Fenda · Corpo vermelho esguio, membros longos · Investida em zigue-zague, ataca pelas costas · Comum: Cinzas Infernais; incomum: Chifre Pequeno (Arcoaria: ponta de flecha infernal) · Sprite atual; 3D com pele rachada mostrando brasa · R8.
- **Esqueleto-Zumbi Atirador ★ (8)** · Mortos-vivos arqueiros do cemitério · Esqueleto com carne pendurada, arco · Mantém distância, dispara projétil esquivável · Sempre: Ossos; comum: Flechas de Ferro ×5; incomum: Arco Curto Podre · Sprite atual · R3.
- **Goblin Brutamontes ★ (9)** · Campeão goblin inchado · Grande, corcunda, clava de tronco · Golpe em área telegrafado (anel vermelho 1,2 s) · Comum: Pele Grossa, moedas; incomum: Clava T2; raro: Dente de Ogro-Goblin (amuleto) · Hoje é o goblin escalado; 3D com cicatrizes e argola no nariz · R1/R7.
- **Escorpião-de-Pedra (10)** · Escorpiões com carapaça de rocha das colinas · Artrópode baixo, cauda alta · Ferroada com **veneno** (5% vida/3 s) [S] · Comum: Placa de Carapaça (Artesanato); incomum: Ferrão (Herbologia: antídoto) · Carapaça com veios de ferrugem · R5.

#### Chefe — Senhor da Clareira ★ (12)
- **Conceito:** ancião da floresta corrompido, metade árvore, metade carne, que manda nos goblins.
- **Silhueta:** enorme, ombros de raiz, chifres de galho, lanterna de fungos no peito.
- **Fases:** 100%: pancadas + projétil lento (como hoje). 60%: invoca 4 Goblins Soldados e brota raízes que prendem (círculos telegrafados). 30%: enraíza-se, fica imóvel e dispara 3 projéteis em leque a cada 2 s até ser derrubado.
- **Saque:** sempre: Coração de Seiva (Herbologia/Construção), moedas ×15; comum: Arco de Carvalho Retorcido (T2 raro); raro: **Coroa de Galhos** (elmo cosmético); único: Semente da Clareira (planta árvore exclusiva na Fortaleza).
- **Arte/rig:** hoje sprite; versão 3D com R7 + cascas e cogumelos bioluminescentes.

#### Faixa 11–20 (Torre Tombada, Galerias, Cemitério, Mina, Mata)
- **Mago Oco (11)** · Aprendizes do Colégio que trocaram a alma por poder · Robe rasgado, rosto de porcelana rachada · Conjura círculos de fogo frio no chão (área ×3 a cada 4 s), recua quando você chega perto · Comum: Runas de Brisa/Maré ×5–10; incomum: Cajado T2; raro: Página do Grimório (missão) · Máscara sem boca · R2.
- **Zumbi Afogado (13)** · Afogados dos esgotos e do charco · Inchado, pele azulada, algas · Lento, agarra (lentidão 30%) [S] · Sempre: Ossos; comum: Enguia Podre (isca); incomum: Anel de Ferro Enferrujado · Pinga água; som de borbulhas · R3.
- **Esqueleto Errante (14)** · Soldados mortos do antigo exército · Esqueleto com elmo e escudo · Bloqueia de frente (−50% dano frontal), vulnerável pelas costas · Sempre: Ossos; comum: Espada T2 Lascada; incomum: Escudo Redondo T2 · Reaproveita o esqueleto do Necromante, com tinta verde nos olhos · R3.
- **Mãe Sarnenta — chefe (15)** · Rata-matriarca do tamanho de um boi, que mora no fundo das Galerias · Corpo inchado, ninhada agarrada nas costas · Fase 1: mordidas e investidas; fase 2: solta enxames de ratos; fase 3: água do esgoto sobe (zonas seguras em plataformas) · Sempre: Ossos Grandes, moedas ×15; comum: Pele Sarnenta (couro); raro: **Coroa de Ratos** (cosmético); único: Rato de Carga Dourado (familiar de Invocação) · Ninho de lixo, sons de milhares de patas · R4 escalado.
- **Anão Renegado (16)** · Mineiros expulsos da Mina do Velho Anão · O **mesmo corpo do herói** com barba trançada e picareta · Joga bombas de pólvora (área telegrafada) e ataca com picareta · Comum: Minério de Ferro ×3, Carvão; incomum: Picareta T2; raro: Projeto da Fornalha (destrava Grande Fornalha) · Óculos de solda quebrados · R2 (reuso direto).
- **Fantasma-de-Lamparina (17)** · Almas que carregam lamparinas apagadas · Véu flutuante, lamparina na mão · Atravessa paredes, some quando você não está olhando; só pode ser atingido com a lamparina acesa (Fogaréu: acender com tocha) · Comum: Ectoplasma (Herbologia, Devoção) · Rosto aparece só no reflexo da água · R6.
- **Urso-Cicatriz (18)** · Ursos marcados por armadilhas · Grande, pelagem com cicatrizes brancas · Investida + patada em cone · Sempre: Pele de Urso, Carne de Urso; incomum: Garra (Artesanato) · Armadilha ainda presa à pata · R4.
- **Druida da Podridão (19)** · Seita que venera a decomposição · Robe de musgo, máscara de crânio de cervo · Raízes que prendem (área), cura outros monstros · Comum: ervas Folha-de-Guarda e Raiz-de-Ferro (não limpas); incomum: Foice de Druida; raro: Semente de Flor-de-Cinza · Moscas ao redor · R2.
- **Hobgoblin Lanceiro (20)** · Hobgoblins de armadura pesada · Alto, lança longa · Estocada de 2,5 m em linha telegrafada · Comum: Lança T3; incomum: Armadura de Placas T2 · Elmo com chifres de bode · R1 escalado.

#### Faixa 21–30
- **Rastejante-de-Carne (21)** · Massas de carne que rastejam nas catacumbas · Bolha com membros humanos · Enxame; ao morrer deixa poça que deixa lento · Comum: Ossos, Carne Estranha (não comestível; isca de Caça) · Horror corporal leve · R5.
- **Lobo-Pálido (22)** · Matilhas brancas dos picos · Lobo esguio · Matilha de 4; uivo que chama +2 · Sempre: Pele Pálida, Ossos de Lobo · Respiração congelada visível · R4.
- **Golem de Entulho (23)** · Pedras animadas por runas mineiras esquecidas · Bloco humanoide · Golpe que racha o chão (linha) · Comum: Carvão ×5, Minério de Auralita (raro); incomum: Pedra Rúnica (essência pura) · Runas pulsando · R7.
- **Sanguessuga-do-Charco (24)** · Sanguessugas do tamanho de um braço · Segmentada · Gruda e drena vida (cura a si mesma) · Comum: Sangue de Sanguessuga (Herbologia: Poção de Recurso) · Brilho viscoso · R5.
- **Capataz de Pedra — chefe (25)** · O antigo chefe da Mina, transformado em golem pela própria ganância · Golem enorme com lanterna de mineiro cravada no peito · Fase 1: socos e tremores; fase 2: escava e reaparece (sombra no chão); fase 3: carrinhos de mina descem trilhos (desviar) · Sempre: moedas ×15, Minério de Auralita ×10; comum: Picareta de Auralita; raro: **Lanterna do Capataz** (luz dupla em cavernas); único: Bigorna Ancestral (Construção) · R7.
- **Gigante-do-Outeiro (26)** · Gigantes sujos e famintos das colinas · Gigante barrigudo com clava · Pisão em área + arremesso de pedra · Sempre: Ossos Grandes; comum: Clava T3; incomum: Chave Gigante (porta do Obrum) · R7.
- **Tecelã Escarlate (27)** · Aranhas vermelhas venenosas · Patas longas · Disparo de teia (lentidão 50% por 2 s) + mordida venenosa · Comum: Olho Vermelho, Seda Escarlate · Teias cobrindo cadáveres · R5.
- **Cavaleiro Corvo-Negro (28, elite)** · Ordem de cavaleiros que traiu o Imperador · Armadura negra com penas de corvo · Escudo que bloqueia de frente; investida com espada; **aura:** aliados +10% armadura · Comum: Peça de Armadura T3; incomum: Elmo Corvo (T3 raro); raro: Insígnia Corvo-Negro (missão) · R2.
- **Ogro do Charco (29)** · Ogros barrigudos que vivem na lama · Gigante com braços enormes · Arremessa troncos (linha telegrafada) · Comum: Ossos Grandes, Couro de Ogro · R7.
- **Viúva do Breu — chefe (30)** · Aranha-rainha na escuridão total da caverna · Abdômen gigante com "rostos" desenhados · A luz da tocha é essencial: sem luz, ela fica invisível; fase 2: ovos que eclodem; fase 3: teia cobre a arena (corte as teias para abrir caminho) · Sempre: Seda do Breu ×10; raro: **Arco Tecido do Breu** (T4 épico); único: Ovo da Viúva (familiar) · R5 escalado.
- **Obrum, o Outeiro Vivo — chefe de mundo (30)** · O maior dos gigantes das colinas, com uma vila nas costas · Fases com pedregulhos e chamado de gigantes · Arena aberta acessível com a Chave Gigante; renasce a cada 1 h por instância · Saque: Clava de Obrum (T4), Ossos Colossais (Devoção) · R7.

#### Faixa 31–40
- **Sentinela de Geada (31)** · Guerreiros congelados que guardam passagens · Armadura de gelo · Golpes que deixam lento [S] · Comum: Cristal de Geada (Runografia) · R2.
- **Rei-Escorpião (32, elite)** · Escorpião gigante dourado · Pinças que agarram (prende 1 s) · Comum: Carapaça Dourada; raro: Coroa de Pinças (cosmético) · R5.
- **Guerreiro de Argila (33)** · Soldados moldados em barro mágico · Regenera parado na lama · Comum: Argila Encantada · R2.
- **Gigante Musgoso (34)** · Gigantes cobertos de musgo · Regenera 1%/s se não sofrer dano por 3 s · Comum: Musgo-Lunar, Ossos Grandes; incomum: Chave Musgosa (Mãe-Musgo) · R7.
- **Rei Sem Rosto — chefe (35)** · Antigo rei enterrado sob a Cidadela, que arrancou o próprio rosto para não ser lembrado · Esqueleto de armadura com máscara de ouro vazia · Fase 1: espada e escudo; fase 2: ergue esqueletos (como o Necromante); fase 3: máscara voa e lança raios enquanto o corpo luta · Saque: **Máscara Vazia** (elmo T5 épico), Coroa Enferrujada (missão), moedas ×15 · R3.
- **Carpideira (36)** · Espírito de luto que chora sangue · Véu negro, mãos compridas · Áreas de lamento no chão; foge · Comum: Ectoplasma, Runa da Morte ×3 · R6.
- **Uivante da Lua Podre (37, elite)** · Lobisomens da Mata nas noites de lua · Bípede/quadrúpede · Investida e uivo que dá medo (lentidão) · Comum: Pele Lunar; raro: Presa Lunar (amuleto) · R4.
- **Gárgula de Calha (38)** · Estátuas vivas dos telhados da Cidadela · Alada de pedra · Só morre se finalizada com **Martelo de Gárgula** (Exterminação 20) · Comum: Pó de Granito; raro: Asas de Pedra (cosmético) · R8.
- **Mãe-Musgo — chefe de mundo (38)** · Gigante ancestral coberta de musgo e flores · Invoca brotos, áreas de esporos · Saque: Clava Florida (T5), Semente de Lírio-Negro · R7.
- **Lamentadora (39)** · Grito em cone que atordoa sem **Protetores de Ouvido** (Exterminação 15) · Comum: Cabelo Fantasma (Artesanato) · R6.
- **Demônio Menor das Brasas (40)** · Demônios brutos da borda da Fenda · Chifres, asas atrofiadas · Golpe flamejante em área · Comum: Cinzas Infernais ×3, Chifre de Demônio; raro: Cimitarra de Brasa (T5) · R7/R8.

#### Faixa 41–50 (Dentes do Norte, Fortaleza, Fenda Infernal, Marca Cinzenta)
- **Gigante de Geada (41)** · Pisões que congelam · Comum: Cristal de Geada ×5, Ossos Colossais · R7.
- **Comandante Corvo-Negro — chefe (42)** · O cavaleiro que matou o último Imperador · Armadura negra, capa de penas, montaria esquelética na fase 2 · Fase 1: duelo; fase 2: a cavalo (investidas em linha); fase 3: chama a ordem inteira (ondas) · Saque: **Espada do Regicida** (T5 épica), Armadura Corvo-Negro (set), moedas · R2 + R4 (montaria).
- **Cão de Cinza (43)** · Cães da Fenda · Matilha de 3, mordida flamejante, rastro de fogo · Comum: Cinzas Infernais, Ossos Queimados · R4.
- **Necrófago (44)** · Comedores de cadáveres · Devora corpos de monstros mortos para curar 20% · Comum: Ossos, Carne Estranha; incomum: Unha de Necrófago (veneno) · R3.
- **Olho-de-Pedra (45)** · Lagarto de um olho só · Olhar petrificante (sem Espelho, Exterminação 35, você fica preso 2 s) · Comum: Escama de Pedra; raro: Olho Petrificado (amuleto) · R5.
- **Serpe Verde (46, elite)** · Dragão menor da Marca Cinzenta · Sopro venenoso em cone (precisa de Escudo Antissopro ou Poção) · Sempre: Ossos de Serpe (melhor oferenda de Devoção), Couro de Serpe (Artesanato T5) · R8.
- **Golem de Obsidiana (47)** · Reflete 30% do dano mágico · Comum: Obsidiana (Construção), Minério Sombrio (raro) · R7.
- **Demônio Maior das Correntes (48, elite)** · Arrasta jogadores com correntes · Comum: Correntes Infernais, Minério Sombrio · R7.
- **Arauto da Fenda (49, elite)** · Abre portais que soltam Diabretes · Comum: Essência Infernal (necessária no Sombraço), Runa de Sangue ×5 · R2/R6.
- **Arquiduque Brasa-Morta — chefe final (50)** · Nobre do Inferno que tenta transformar a Capital em brasa · Gigante de armadura derretida com coroa de chamas negras · Fase 1: espada flamejante; fase 2: arena se divide em ilhas de rocha sobre lava; fase 3: invoca Demônios Maiores e cai uma "chuva de cinza" (áreas no chão) · Saque: **Essência do Arquiduque** (arma lendária de classe à escolha), Coroa de Brasa (cosmético), Título "Apagador de Brasas" · R7 com VFX de fogo do `CharacterVfx.gd`.

### 8.5 Debandada usando o bestiário
| Arena | Ondas comuns | Elites/atiradores | Chefe onda 5 | Chefe onda 10 |
|---|---|---|---|---|
| Clareira da Floresta (atual) | Goblin Soldado, Demônio Corredor | Esqueleto-Zumbi, Goblin Brutamontes | Goblin Brutamontes Elite | Senhor da Clareira |
| Galerias Afogadas | Rato-de-Esgoto, Aranha-de-Poço, Zumbi Afogado | Diabrete Cinzento | Zumbi Afogado Elite | Mãe Sarnenta |
| Cemitério | Esqueleto Errante, Rastejante-de-Carne | Fantasma, Carpideira | Necrófago Elite | Rei Sem Rosto |
| Mina | Anão Renegado, Escorpião | Golem de Entulho | Hobgoblin Elite | Capataz de Pedra |
| Fenda Infernal | Cão de Cinza, Demônio Corredor | Demônio Menor, Arauto | Demônio Maior | Arquiduque Brasa-Morta |

Nível efetivo da arena na Debandada = `3 + 4·(índice da arena) + 2·Tormento`, combinado com a escala por onda que já existe.


---

## 9. A Capital do Império (primeiro mapa)

> Primeiro e, por enquanto, **único** mapa do Crônicas do Império. Os outros 24 reinos (§10) ficam só como ideia até a Capital estar pronta.

### 9.1 Escala: de onde vêm os números
- **Referência:** no OSRS o mapa é dividido em **regiões de 64 × 64 tiles** (chunks de 8 × 8). As muralhas de Varrock ocupam aproximadamente as coordenadas **x 3175–3290 e y 3380–3505**, cerca de **120 × 125 tiles** (uma região e meia em cada eixo). Varrock com os arredores (Grande Câmbio a noroeste, Vila Bárbara a oeste, serraria a nordeste, Guilda dos Campeões ao sul, borda da Wilderness ao norte) cabe em algo como **4 × 3 regiões ≈ 256 × 192 tiles**. São estimativas a partir das coordenadas do mapa do wiki: conferir antes de fechar o tamanho final.
- **Conversão tile → metro:** no RS, correr cobre ~3,3 tiles/s. Nosso herói anda a ~4,75 m/s (190 px/s). Manter o "tempo de travessia" daria 1 tile ≈ 1,4 m, mas isso fica apertado para combate de ação com a câmera atual. **Proposta: 1 tile = 2 m** (cidade 20–40% mais "larga" para caber combate e multidão).
- **Resultado:**

| Medida | Tiles (ref. RS) | Metros | Pixels de arena (40 px/m) |
|---|---:|---:|---:|
| Cidade murada | ~120 × 125 | **240 × 250 m** | 9.600 × 10.000 |
| Reino completo (cidade + arredores) | 384 × 288 (6 × 4,5 regiões) | **768 × 576 m** | 30.720 × 23.040 |
| Comparação: arena da Debandada | — | 120 × 75 m | 4.800 × 3.000 |

O reino completo tem **~49×** a área da arena atual. Travessia de ponta a ponta: ~2 min 40 s andando (o RS leva tempo parecido para cruzar Varrock e arredores). Masmorras são mapas **instanciados à parte** e não entram nessa conta.

- **Divisão técnica:** o reino é dividido em **chunks de 64 × 64 m** (12 × 9 = 108 chunks). O servidor só simula monstros nos chunks a até 2 de distância de algum jogador ("sono" de chunk), e o cliente carrega geometria e vegetação por chunk (§17.4).

### 9.2 Esboço do mapa (1 caractere = 16 m; norte para cima)
```
:::::::::::::::::::::::::::::≈::::::^^^^^^^^^^^^
:::::::::::::::::::::::::::::≈::::::^^^^^^^^^^^^
::FFFFFFFF:::::::::::::::::::≈≈:::::^^^^^^^^^^^^
::FFFFFFFF:P::::::::::::::::::≈:::::^^^^^^^6^^^^
..FFFFFFFF....................≈.....^^^^^^^^^^^^
..FFFFFFFF.....a..............≈.....^^^^^^^^^^^^
..FFFFFFFF........4...........≈≈....^^^^^^^^^^^^
.nFFFFFFFFnnnnnn.............5.≈....^^^^a^^^^^^^
.nnnnnnnnnnn3nnn...............≈....^^^^^^^^^^^^
.nnnnnnnnnnnnnnn...............≈....^^^^^^^^^^^^
.nnnnnnnnnnnnnnn#######^########≈...^^^^^^^^^^^^
.nnnnnnnnnnnnnnn#              #≈...^^^^^^^^^^^^
.nnnnn1nnnnnnnnn#              #≈W..^^^^^^^^^^^^
.nnnnnnnnnnnnnnn# A    S C     #≈..TTTTTTTTTTTTT
.nnnnnnnnnnnnnnn#              #≈..TTTTTTTTTTTTT
.nnnnnnnnnnnnnnn#              #≈..TTTTTTTT9TTTT
.nn2nnnnnnnnnnnn#              #≈..TTTTTTTTTTTTT
.nnnnnnnnnnnnnnn#              #≈..TTTTTTTTTTTTT
.nn"""""""""""""> B         M  =≈..TTTTT8TTTTTTT
..."""""""""""""#      $       #≈..TTTTTTTTTTTTT
..."""""""""""""#              #≈..TTTTTTTTTTTTT
..."""""""""""""#              #≈..TTTTTTTTTTTTT
..."""""""""""""#              #≈..TTTTTTTTTTTTT
..."""""""""""""#              #≈..TTTTTTTTTT7TT
..."""""""""""""# L     G    D #≈..TTTTTTTTTTTTT
..."""""""V"""""#              #≈..0TTTTTTTTTTTT
..."""""""""""""########v#######≈..TTTTTTTTTTTTT
...""""""""""c""............≈≈≈....%%%%%%%%%%%%%
..."""""""""""""....a..≈≈≈≈≈≈......%%%%%%%%%%%%%
...""""""""""""≈≈≈≈≈≈≈≈≈...........%%%%%%%%%%%%%
..tttttttt.≈X≈≈≈.......|...........%%%%%%%%%%%%%
≈≈≈≈≈≈≈≈≈≈≈≈.......................%%%%%%%%%%%%%
≈.tttt*ttt.............|...........%%%%%%K%%%%%%
..tttttttt.........................%%%a%%%%%%%%%
..tttttttt.........................%%%%%%%%%%%%%
.......................|...........%%%%%%%%%%%%%
```
**Legenda:** `:` Marca Cinzenta (ermo do norte) · `^` Dentes do Norte (montanhas geladas) · `n` Colinas de Ferrugem · `F` Fortaleza Corvo-Negro · `T` Mata dos Enforcados · `%` Charco Cinzento · `"` Campos do Moinho (fazendas) · `t` Clareira da Floresta (o mapa atual da Debandada, reaproveitado como zona) · `≈` Rio Lacrimal · `#` muralhas · `|` Estrada do Rei.
**Dentro da muralha:** `C` Cidadela Imperial · `A` Colégio Arcano · `S` Catedral do Sol Poente · `B` Bairro da Forja · `$` Praça das Balanças e Bolsa Imperial · `M` Quarteirão dos Mercadores · `L` Bairro Baixo (Becos) · `G` Arena dos Campeões e Quartel · `D` Cais do Rio.
**Portões:** `>` Portão Oeste · `^` Portão Norte · `v` Portão Sul · `=` Ponte Leste.
**Pontos:** `1` Mina do Velho Anão (masmorra) · `2` Fosso dos Escorpiões · `3` Arena de Obrum · `4` Torre Tombada · `5` Acampamento Goblin · `6` Fenda Infernal · `7` Caverna dos Ecos (masmorra) · `8` Círculo de Pedras · `9` Bosque Velho (Mãe-Musgo) · `0` Cemitério dos Silenciosos e entrada das Catacumbas da Coroa · `K` Lago Afogado · `V` Vila de Trigal · `c` Pastos Chifre-Torto · `X` Vau dos Bandidos · `W` Serraria do Rio · `P` Mosteiro da Vigília · `a` sítios de Arqueologia · `*` covil do Senhor da Clareira.

### 9.3 A cidade murada (240 × 250 m)
O Império está em decadência: o último Imperador foi assassinado por sua própria guarda (a Ordem Corvo-Negro), o trono está vazio e a cidade é governada por um **Conselho de Regência** dividido. A cidade é bonita de dia e assustadora à noite: sinos tocam à meia-noite, as gárgulas se mexem nos telhados e os esgotos sussurram.

| Distrito | Posição (m) | Função de jogo | Pontos e NPCs |
|---|---|---|---|
| **Cidadela Imperial** | x 400–490, y 180–250 (penhasco sobre o rio, no canto nordeste) | Sede do Conselho, missões principais, entrada nobre das Catacumbas | Salão do Trono Vazio, Torre do Relógio Quebrado, Regente Severa Altamar, Capitã Iolanda da Guarda Cinzenta |
| **Catedral do Sol Poente** | x 340–400, y 190–260 | Devoção (altar principal), Santuário de renascimento | Arcebispo Teodoro, Irmãs da Vigília, cripta lateral |
| **Colégio Arcano** | x 270–340, y 180–250 | Magia e Runografia; pedreira de Pedra-Essência no pátio | Arquimaga Liora Vesp, loja de runas, altar da Runa da Mente no subsolo |
| **Bairro da Forja** | x 270–340, y 260–340 | Metalurgia: **Grande Fornalha** e 6 bigornas; Arcoaria (oficina) | Mestre-Ferreiro Brom Martelo-Fundo (anão), Guilda dos Metalurgistas (entrada: Metalurgia 30) |
| **Praça das Balanças** | x 340–420, y 270–335 | **Bolsa Imperial** (§13), banco central, quadro de missões | Cambista-Mor Otávio Ruela, 2 bancos (oeste e leste), pregoeiro |
| **Quarteirão dos Mercadores** | x 410–490, y 260–335 | Lojas NPC (armas, armaduras, comida, ferramentas, runas) e barracas (Ladinagem) | Guilda dos Mercadores, Casa de Leilões (futuro) |
| **Bairro Baixo** | x 270–350, y 345–410 | Ladinagem, circuito de Agilidade nos telhados, taverna, entrada dos Esgotos | Taverna do Corvo Bêbado, Mestre dos Ladrões "Sombra", Mestre Exterminador Velha Garra |
| **Arena dos Campeões** | x 360–430, y 345–410 | Treino de combate, desafios contra monstros, **Guilda dos Campeões** (Nível de Combate 32) | Mestre de Armas Doran, arena de duelo (PvP opcional, Q9) |
| **Cais do Rio** | x 440–500, y 340–410 | Pesca (rede e vara), Culinária (fogão grande), barcos (Navegação futura), canoas | Velho Pescador Tião, Guilda dos Pescadores (Pesca 35) |
| **Muralhas** | perímetro | Circuito de Agilidade das Muralhas (nível 20), torres de vigia | Guardas, sentinelas (eventos de Incursão) |

### 9.4 Arredores (zonas abertas)
| Zona | Área aprox. (m) | Níveis | Coleta | Monstros | Destaques |
|---|---|---|---|---|---|
| Campos do Moinho | x 60–250, y 300–480 | 1–6 | Agricultura (canteiros públicos), Caça (coelho, perdiz), Pesca no rio | Galinha, Vaca, Andarilho, Diabrete (noite) | Moinho (farinha), Vila de Trigal (tutorial), Pastos Chifre-Torto |
| Clareira da Floresta | x 40–160, y 480–560 | 3–12 | Lenhador (pinheiro, carvalho), Pesca (truta) | Goblins, Demônio Corredor, Esqueleto-Zumbi | **o mapa atual da Debandada**, com o covil do Senhor da Clareira |
| Estrada do Rei e Vau dos Bandidos | sul e sudoeste | 4–12 | — | Bandido do Vau | Pedágio ilegal (missão), carroças |
| Colinas de Ferrugem | x 20–250, y 120–290 | 10–26 | Mineração (cobre a auralita), Caça (raposa, javali) | Escorpião, Hobgoblin, Gigante-do-Outeiro | Mina do Velho Anão, Fosso dos Escorpiões, Arena de Obrum |
| Torre Tombada | x 280–330, y 90–130 | 11–20 | Arqueologia (Acampamento Legionário perto) | Mago Oco | Biblioteca queimada (missão do Colégio) |
| Acampamento Goblin | x 440–500, y 100–140 | 3–12 | — | Goblins | Totem goblin, eventos de Incursão |
| Mata dos Enforcados | x 560–768, y 208–432 | 15–38 | Lenhador (salgueiro a fantasmeira), Caça (cervo, lince, mariposa), ervas silvestres | Urso, Druida, Uivante (noite), Gigante Musgoso | Círculo de Pedras, Bosque Velho, Caverna dos Ecos |
| Charco Cinzento | x 560–768, y 432–576 | 18–33 | Pesca (enguia), Caça (salamandra), Arqueologia (Templo Afundado) | Zumbi Afogado, Sanguessuga, Ogro, Guerreiro de Argila | Lago Afogado (Enguia-Abissal) |
| Dentes do Norte | x 576–768, y 0–208 | 22–41 | Mineração (ouro, vermécio, granito-sangue), Arqueologia (Observatório Caído) | Lobo-Pálido, Sentinela, Gigante de Geada | trilhas de Agilidade, passagens congeladas |
| Fortaleza Corvo-Negro | x 40–160, y 32–128 | 28–42 | Ladinagem (baús de guerra) | Cavaleiro Corvo-Negro | **castelo arruinado** com masmorra, chefe Comandante |
| Marca Cinzenta | faixa norte, y 0–64 | 30–50 | recursos raros, mais rápidos | Serpe Verde, Demônios | zona de alto risco (morte perde 1 item a mais, Q9) |
| Fenda Infernal | x 680–740, y 30–90 (entrada) | 40–50 | Mineração (Minério Sombrio), Essência Infernal | Cão de Cinza, Golem de Obsidiana, Demônio Maior, Arauto | masmorra final, Arquiduque |
| Cemitério dos Silenciosos | x 520–600, y 380–460 | 8–20 (superfície) | Arqueologia (Necrópole) | Esqueletos, Fantasmas (noite) | entrada das Catacumbas da Coroa |
| Mosteiro da Vigília | x 160–200, y 40–80 | — | Agricultura (jardim de ervas) | — | altar de Devoção alternativo, loja de poções |
| Serraria do Rio | x 530–560, y 190–215 | — | troca toras por tábuas (Construção) | — | canoas |

### 9.5 Masmorras, cavernas e castelos
Todas são **instâncias** (um mapa separado por grupo, 1–4 jogadores). Entradas físicas no mundo.

| Masmorra | Entrada | Níveis | Andares/salas | Mecânica-tema | Chefe | Requisito |
|---|---|---|---|---|---|---|
| **Galerias Afogadas** (esgotos) | Bairro Baixo (bueiro) | 3–15 | 3 andares | nível da água sobe e desce; atalhos de Agilidade | Mãe Sarnenta (15) | — |
| **Mina do Velho Anão** | Colinas (1) | 10–25 | 4 galerias | escuridão (tocha), carrinhos de mina, veios ricos | Capataz de Pedra (25) | Missão "A Última Lanterna" |
| **Caverna dos Ecos** | Mata (7) | 15–45 | 3 câmaras + fosso | **luz é vida**: sem tocha os monstros ficam invisíveis; ecos revelam emboscadas | Viúva do Breu (30); fosso: Olho-de-Pedra | Fogaréu 10 (tochas) |
| **Catacumbas da Coroa** | Cemitério (0) e Cidadela | 20–44 | 5 criptas | portas de Ladinagem, armadilhas, esqueletos que se reerguem se não forem esmagados | Rei Sem Rosto (35) | Missão "O Rei Que Esqueceu" (parte 1) |
| **Fortaleza Corvo-Negro** (castelo arruinado) | Noroeste (F) | 28–42 | pátio, salão, masmorra, torre | castelo original: muralha dupla em estrela, ponte levadiça quebrada, torre de menagem inclinada; cerco em ondas no pátio | Comandante Corvo-Negro (42) | Exterminação 25 ou missão |
| **Fenda Infernal** | Nordeste (6) | 40–50 | 3 círculos | lava, plataformas que afundam, portais | Arquiduque Brasa-Morta (50) | Nível de Combate 40 e missão "Brasa Sob a Coroa" |
| **Profundezas** (Masmorrismo) | porão da Taverna do Corvo Bêbado | 1–50 | andares procedurais (1–30) | salas geradas por semente, chefe aleatório do bestiário | rotativo | Masmorrismo por andar |

**Castelo do Império (Cidadela Imperial) — design original, sem copiar Varrock:** fica num **penhasco sobre o rio**, com acesso por uma **rampa em zigue-zague** coberta (o "Caminho dos Sete Arcos"); o corpo principal é uma **torre de relógio** quebrada parada às 12h03 (a hora do regicídio), com uma ala de vidro colorido destruída e um **jardim suspenso** morto. Por baixo, um poço vertical leva às Catacumbas (atalho de Agilidade 40).

### 9.6 NPCs principais (primeiro reino)
| NPC | Onde | Papel |
|---|---|---|
| Regente Severa Altamar | Cidadela | linha de missões principal, título de Cavaleiro do Império |
| Capitã Iolanda | Cidadela / muralhas | Incursões, missões de defesa |
| Arquimaga Liora Vesp | Colégio Arcano | Magia, Runografia, missões arcanas |
| Brom Martelo-Fundo | Bairro da Forja | Metalurgia, tutoriais de forja, Guilda dos Metalurgistas |
| Cambista-Mor Otávio Ruela | Praça das Balanças | Bolsa Imperial |
| Velha Garra | Bairro Baixo | Mestra Exterminadora (tarefas) |
| "Sombra" | Bairro Baixo | Ladinagem |
| Tião | Cais | Pesca, Culinária |
| Irmã Celene | Catedral / Mosteiro | Devoção, Herbologia |
| Doran | Arena | combate, Guilda dos Campeões |
| Professora Ermelinda Pó | Museu (dentro do Colégio) | Arqueologia, doações ao museu |
| Mestre Jacó | Vila de Trigal | Agricultura, tutorial |
| Ruiva, a Caçadora | Campos / Mata | Caça |
| Carpinteiro Benedito | Serraria | Construção, Arcoaria |

---

## 10. Os 25 reinos

Ideia para o futuro (só a Capital será construída agora). Faixas de nível acima de 50 assumem que o teto vai subir depois (ver Q10).

| # | Reino | Raça principal | Bioma | Governo | Capital | Tema | Níveis | Recursos e monstros marcantes |
|---:|---|---|---|---|---|---|---|---|
| 1 | **Capital do Império** | humanos | planície fluvial temperada | monarquia em regência (trono vazio) | Capital do Império | império decadente, regicídio | 1–50 | ferro, auralita, ervas; goblins, mortos-vivos, Fenda |
| 2 | Clãs de Pedra-Funda | anões | montanhas e cidades subterrâneas | conselho de clãs (federação) | Karak-Ondur | forjas, dívidas de sangue | 40–70 | vermécio, gemas, runas de terra; golems, trolls |
| 3 | Bosque de Lúmen | elfos | floresta ancestral | monarquia eletiva (rainha escolhida pelas árvores) | Ithelmar | natureza que se defende | 45–75 | madeiras raras, seda; dríades corrompidas |
| 4 | Horda de Brasa-Rubra | orcs | estepe vulcânica | chefia militar (quem vence lidera) | Gor-Mahk | honra e conquista | 50–80 | obsidiana, couro de lagarto; wargs, xamãs |
| 5 | Tribos do Musgo | goblins e hobgoblins | pântanos e cavernas | anarquia de tribos com rei-totem | Fossa Grande | humor sombrio, caos | 10–40 | cogumelos, lodo; goblins de todas as castas |
| 6 | Ilhas Salgadas | tritões | arquipélago e recifes | república marítima | Coral-Alto | comércio e pirataria | 50–85 | pérolas, peixes abissais; serpentes marinhas |
| 7 | Califado das Areias | humanos nômades | deserto | califado (teocracia mercantil) | Sahr-Ennan | tumbas, miragens | 55–85 | ouro, especiarias; múmias, escorpiões gigantes |
| 8 | Necrópole de Vel'Mora | mortos-vivos conscientes | terras cinzentas sem sol | necrocracia (conselho de liches) | Vel'Mora | a morte como cidadania | 70–100 | ossos antigos, runas da morte; liches, carniçais |
| 9 | Corte das Fadas Pálidas | feéricos | floresta crepuscular onírica | corte de duas rainhas (verão/inverno) | Trono de Orvalho | contratos e ilusões | 60–90 | pó de fada, flores lunares; fadas sombrias |
| 10 | Planície dos Cascos | centauros | pradarias abertas | democracia de manadas (assembleia) | Círculo dos Ventos | caça e liberdade | 35–60 | cavalos, couro; grifos, bisões |
| 11 | Ninhos de Corvália | homens-corvo (aviários) | penhascos e torres de vento | oligarquia de famílias | Ninho-Mor | segredos e espionagem | 55–80 | penas raras, mensagens; harpias |
| 12 | Colmeia de Quitina | insectoides | savana de cupinzeiros gigantes | mente coletiva (rainha-colmeia) | O Ninho | horror de colmeia | 75–105 | quitina, mel âmbar; operárias, soldados |
| 13 | Reino Fúngico de Esporânia | miconídeos (cogumelos) | cavernas bioluminescentes | consenso por esporos (democracia química) | Micélia | decomposição e renascimento | 40–65 | fungos alquímicos; parasitas |
| 14 | Engenho de Latão | gnomos | colinas industriais | meritocracia de inventores | Cidade-Engrenagem | máquinas (sem a habilidade Invenção) | 45–70 | latão, vidro, carvão; autômatos defeituosos |
| 15 | Jarlado do Gelo Eterno | meio-gigantes do gelo | tundra e geleiras | jarlado (reis guerreiros) | Hrimvald | sobrevivência e sagas | 60–90 | ferro-gélido, peles; mamutes, yetis |
| 16 | Domínio Escamado | homens-lagarto | selva quente e rios | dinastia sacerdotal | Ssath-Kar | sacrifícios e templos-pirâmide | 65–95 | jade, venenos; basiliscos, serpes |
| 17 | Principado Carmesim | vampiros e servos humanos | colinas de vinhedos sob neblina | aristocracia de sangue | Sanguinária | nobreza parasita | 80–110 | vinho de sangue, prata; vampiros, morcegos gigantes |
| 18 | Ermos dos Uivantes | homens-fera (lupinos) | taiga e florestas de pinheiro | alcateias com Alfa | Toca do Primeiro Uivo | maldição e lua | 50–80 | peles, presas; lobisomens |
| 19 | República de Porto-Livre | humanos, meio-elfos, anões | costa e cidade-porto | república mercantil (senado de guildas) | Porto-Livre | dinheiro, contrabando | 30–60 | todo o comércio; contrabandistas |
| 20 | Teocracia da Chama Branca | humanos fanáticos | planalto de pedra calcária | teocracia (Sumo Inquisidor) | Alvorada | fé que queima | 70–100 | incenso, prata sagrada; inquisidores, anjos de pedra |
| 21 | Cidadela Dracônica | draconatos | vulcões e picos | dinastia dracônica (dragão-rei) | Ninho de Cinza | tesouros e orgulho | 90–120 | escamas, ouro rubro; dragões adultos |
| 22 | Concílio dos Gigantes de Pedra | gigantes de pedra | cânions de arenito | gerontocracia (os mais velhos e lentos mandam) | Monólito | tempo geológico | 85–115 | granito rúnico; golems titânicos |
| 23 | Fendas do Abismo | demônios | planícies de lava e cinza | tirania infernal (arquiduques em guerra) | Trono de Brasa | o inferno que vaza para o mundo | 100–130 | essência infernal, sombraço puro; demônios maiores |
| 24 | Pântano das Bruxas Velhas | bruxas (coven humano e não humano) | brejo de mangue negro | matriarcado de coven | Caldeirão | maldições e pactos | 55–85 | ervas proibidas; espantalhos, sapos gigantes |
| 25 | Ruínas de Aethra | constructos e espíritos antigos | cidade flutuante em ruínas | sem governo (a IA ancestral dorme) | Aethra | civilização perdida, Arqueologia | 110–150 | relíquias, cristais celestes; sentinelas arcanas |

**Raças jogáveis futuras** (quando quisermos): anões (já é a base visual dos heróis!), elfos, orcs, homens-corvo, tritões.


---

## 11. Itens, raridade e progressão de equipamento

### 11.1 Raridade e qualidade
| Raridade | Cor da moldura | Origem típica | Afixos aleatórios | Negociável |
|---|---|---|---|---|
| Comum | cinza-osso | fabricado, lojas NPC, saque | 0 | sim |
| Incomum | verde-musgo | fabricado com qualidade "Fino", saque | 1 | sim |
| Raro | azul-aço | saque de elite, Obra-Prima | 2 | sim |
| Épico | roxo-hematoma | chefes, masmorras | 3 + 1 efeito único | sim (com taxa maior, §13) |
| Lendário | âmbar-brasa | chefe final, missões épicas | efeito de classe | **vinculado** à conta |

**Qualidade de fabricação** (só itens fabricados): Normal → **Fino** (+10% nos atributos base; chance `5% + 0,5%·(nível − requisito)`, máx. 25%) → **Obra-Prima** (+20% e 1 afixo; 1×/dia com Suporte 45 ou 1% de chance). Isso dá valor de mercado a artesãos de nível alto mesmo fabricando itens baixos.

**Afixos** (exemplos, com faixa por tier T): +`(2–4)·T`% dano crítico · +`(1–2)·T`% chance crítica · +`(8–15)·T` vida · +`(1–2)·T`% vel. de ataque · +`(2–3)·T` armadura · +`(1–2)·T`% velocidade de coleta (ferramentas) · +`T`% XP de uma habilidade (só em amuletos).

### 11.2 Tiers de equipamento (amarrados ao nível)
| Tier | Metal / material | Requisito de uso (Ataque/Força/Defesa/Pontaria/Magia/Necromancia) | Mult. de dano da arma | Armadura por conjunto pesado (6 peças) | Cor-assinatura |
|---|---|---:|---:|---:|---|
| T1 | Bronze / Pinheiro / Couro Macio / Lã | 1 | ×1,00 | 24 | bronze opaco |
| T2 | Ferro-Negro / Carvalho / Couro Curtido / Linho | 10 | ×1,25 | 48 | grafite |
| T3 | Aço-Cinzento / Salgueiro / Couro Reforçado / Linho Tingido | 20 | ×1,55 | 78 | aço azulado |
| T4 | Auralita / Ácer / Couro de Cervo / Seda Escarlate | 30 | ×1,90 | 108 | azul-celeste perolado |
| T5 | Vermécio / Teixo / Couro de Lince / Seda do Breu | 40 | ×2,30 | 138 | vermelho-vinho |
| T6 | Sombraço / Fantasmeira / Couro Espectral / Véu Lunar | 50 | ×2,75 | 168 | preto com brilho de brasa |

**Dano do herói no mundo aberto:** `dano = dano_base_da_classe × mult_rank × (1 + 0,03·(nível_ofensivo − 1)) × mult_arma × Π(caminhos, afixos)`.
Exemplo Bárbaro (38 de base), Força 50, machado T6, rank 1: `38 × 2,47 × 2,75 ≈ 258` por golpe.

**Vida do herói:** `vida = vida_base × mult_rank × (1 + 0,04·(Constituição − 1)) + Σ vida dos itens`. Bárbaro com Constituição 50: `1400 × 2,96 ≈ 4.144`.

**Mitigação:** `dano_recebido = dano × 100 / (100 + armadura)` (a fórmula do jogador; monstros usam `200/(200 + armadura)` para que o tempo de luta não exploda no nível 50).

### 11.3 Slots e famílias de arma por classe
13 slots: Cabeça, Ombros, Peito, Mãos, Pernas, Pés, Capa, Amuleto, Anel I, Anel II, Arma principal, Mão secundária, Cinto de Batalha (4 consumíveis).

| Classe | Arma principal (família) | Mão secundária | Peso de armadura | Habilidade ofensiva |
|---|---|---|---|---|
| Bárbaro | machados de duas mãos | — | pesada | Força (+Ataque) |
| Cruzado | maças | escudo | pesada | Força (+Ataque), Devoção |
| Caçador de Demônios | bestas de mão (par) | aljava de virotes | média | Pontaria |
| Monge | bastões / manoplas | — | leve-média | Ataque (+Força) |
| Necromante | foices / cajados de osso | grimório / relicário | leve | Necromancia |
| Arcanista | cajados com cristal | orbe | leve | Magia |
| Cavaleiro Sangrento | glaives | — | pesada-média | Força (+Necromancia como bônus de roubo de vida) |
| Mago | cajados de chama | tomo | leve | Magia |
| Arqueiro | arcos longos | aljava de flechas | média | Pontaria |

Pesada exige Defesa e é feita na **Metalurgia**; média é **couro** (Artesanato) com reforços de metal; leve é **tecido** (Artesanato) com runas (Runografia).

### 11.4 Durabilidade e reparo
- Armas e armaduras: 1.000 pontos de durabilidade; −1 por golpe dado/recebido (×0,5 com Couro de Ferro). A 0, o item dá 50% dos atributos até ser reparado.
- Reparo: na bigorna (Metalurgia ≥ requisito − 10, custa 1 barra a cada 25%) **ou** no NPC por ouro (`valor_guia × 0,08` por 100% — sumidouro de ouro, §13.6).

---

## 12. Metalurgia e cadeias de produção

### 12.1 Minérios e barras
| Barra | Minérios (Mineração) | Nível de Metalurgia | XP por barra | Usos |
|---|---|---:|---:|---|
| Bronze | 1 Cobre (1) + 1 Estanho (1) | 1 | 6 | T1 armas, armaduras, ferramentas |
| Ferro-Negro | 1 Ferro-Negro (10) — 50% de falha sem anel de forja | 10 | 12 | T2 |
| Prata | 1 Prata (15) | 15 | 14 | joias, flechas contra mortos-vivos |
| Aço-Cinzento | 1 Ferro-Negro + 2 Carvão (20) | 20 | 18 | T3 |
| Auralita | 1 Auralita (30) + 3 Carvão | 30 | 30 | T4 |
| Ouro | 1 Ouro (35) | 35 | 23 (×2,5 com Luvas de Ourives) | joias |
| Vermécio | 1 Vermécio (40) + 4 Carvão | 40 | 38 | T5 |
| Sombraço | 1 Minério Sombrio (50) + 6 Carvão + 1 Essência Infernal | 50 | 50 | T6 |

A **Grande Fornalha** (Metalurgia 45) funde 2 barras por carga com metade do carvão, mas exige 2 jogadores (ou 1 jogador + familiar Golem) alimentando o fogo. É uma atividade co-op de propósito.

### 12.2 Forja (bigorna): itens por tier
`nível_do_item = nível_base_do_tier + deslocamento`. XP de forja = `XP_da_barra × barras_usadas`.

| Item | Desloc. | Barras | Item | Desloc. | Barras |
|---|---:|---:|---|---:|---:|
| Adaga | +0 | 1 | Elmo médio | +3 | 1 |
| Machadinha (ferramenta de Lenhador) | +1 | 1 | Espada curta | +4 | 1 |
| Picareta (ferramenta de Mineração) | +1 | 2 | Cimitarra / glaive leve | +5 | 2 |
| Maça | +2 | 1 | Espada longa | +6 | 2 |
| Pontas de flecha ×15 | +2 | 1 | Elmo completo | +7 | 2 |
| Virotes sem pena ×10 | +3 | 1 | Escudo quadrado | +8 | 3 |
| Cravos de armadura (couro reforçado) | +3 | 1 | Perneiras de placas | +9 | 3 |
| Botas de placas | +4 | 1 | Peitoral de placas | +9 | 5 |
| Manoplas | +4 | 1 | Machado de duas mãos | +9 | 3 |
| Ombreiras | +5 | 2 | Escudo de torre (Defesa 15+) | +9 | 4 |

Exemplos: Peitoral de Aço-Cinzento = Metalurgia 29, 5 barras, 90 XP. Machado de duas mãos de Vermécio = Metalurgia 49, 3 barras, 114 XP. Itens de Sombraço exigem **Metalurgia 50** independente do deslocamento.

### 12.3 Outras cadeias (coleta → processamento → produto)
| Cadeia | Coleta | Processamento | Produto final | Destino |
|---|---|---|---|---|
| **Madeira** | Lenhador: toras + casca | Serraria (tábuas, ouro) / Arcoaria (entalhar) | arcos, bestas, cabos de cajado, flechas, tábuas | Pontaria, Magia, Construção |
| **Casca** (subproduto do Lenhador, 20% por tora) | Lenhador | Curtume (casca de Carvalho → tanino) / Herbologia (casca de Salgueiro → analgésico) / Artesanato (casca de Bétula → pergaminho) | couro curtido, Tônico de Vigor+, pergaminhos de teleporte | Artesanato, Herbologia, Magia |
| **Couro** | Caça (peles), vacas, ursos, serpes | curtume (casca + sal) → Artesanato | armaduras médias, luvas, botas, aljavas | Pontaria, todas as classes |
| **Tecido** | Agricultura (linho, algodão-do-vale), seda de aranha (saque) | roca (fio) → tear (tecido) → Artesanato | mantos, capuzes, robes, bolsas de runas | Magia, Necromancia |
| **Pesca/Culinária** | Pesca, Caça (carne), Agricultura (trigo, batata, maçã) | Culinária (fogão > fogueira: −10% de queimar) | comida, banquetes | cura de todos |
| **Ervas/Herbologia** | Agricultura (ervas), coleta silvestre na Mata, saque | limpar erva → poção sem acabar (+ frasco de vidro) → secundário | poções, antídotos, venenos | combate, coleta (Poção de Foco: +3 níveis temporários) |
| **Runas** | Mineração (Pedra-Essência) | Runografia nos altares (Brisa, Maré, Terra, Brasa, Mente, Corpo, Caos, Lei, Morte, Sangue) | runas | feitiços, encantar joias, teleportes, rituais da Necromancia |
| **Joias** | Mineração (prata, ouro, gemas) | Metalurgia (barra) → Artesanato (molde) → Magia (encantar com runas) | anéis, amuletos, colares encantados | atributos e utilidades (Anel de Forja, Amuleto do Pescador) |
| **Vidro/Cerâmica** | areia (Cais), soda (alga queimada), argila | Fogaréu + Artesanato (forno) | frascos (Herbologia), lanternas (Fogaréu), vasos | suporte |
| **Construção** | tábuas, pedra, pregos (Metalurgia), tecido | bancada na Fortaleza | móveis, salas, altar, portal | bônus permanentes |
| **Devoção** | ossos e cinzas (saque) | oferenda no altar (×3,5 XP com altar da Fortaleza 30) | XP de Devoção | orações |
| **Arqueologia** | escavação | restauração no Museu (Artesanato + material) | artefatos restaurados | XP, reputação do Museu, itens decorativos e relíquias (bônus passivos pequenos) |
| **Invocação** | amuletos de pacto (saque) + ingrediente | obelisco de Invocação + runas | bolsas de pacto (familiar por 30 min) | carga, combate, coleta |

### 12.4 Consumíveis (referência)
| Comida | Culinária | Cura | Poção | Herbologia | Efeito |
|---|---:|---:|---|---:|---|
| Camarão | 1 | 78 | Tônico de Vigor | 1 | cura 120 + 2% vida/s por 5 s |
| Pão | 1 | 78 | Poção de Força | 5 | +5% dano por 5 min |
| Truta | 10 | 240 | Antídoto | 12 | cura veneno, imune por 90 s |
| Torta de Maçã (2 porções) | 20 | 2 × 210 | Poção de Defesa | 20 | +20 de armadura por 5 min |
| Salmão | 20 | 420 | Poção de Recurso | 26 | restaura 40% do recurso |
| Lagosta | 30 | 600 | Elixir de Pontaria / Magia | 32 / 38 | +8% dano do estilo por 5 min |
| Bacalhau-Negro | 35 | 690 | Poção de Coragem | 40 | +30% resistência a controle por 3 min |
| Peixe-Lâmina | 40 | 780 | Antídoto Superior | 45 | imune a veneno e sopro de serpe por 3 min |
| Enguia-Abissal | 50 | 960 | Elixir Supremo do Império | 50 | +10% dano, +10% vida, +10% vel. de coleta por 10 min |

Cura da comida = `60 + 18·nível_requerido`. Comer tem recarga global de 1,8 s, poções de 3 s, e comida não funciona durante a animação de esquiva.


---

## 13. Mercado: a Bolsa Imperial

Inspirada no Grand Exchange do RuneScape (livro de ofertas de compra e venda, preço guia, limite de compra a cada 4 h, taxa de 2% como sumidouro de ouro), com regras próprias.

### 13.1 Regras de jogo
| Regra | Valor proposto | Por quê |
|---|---|---|
| Espaços de oferta | **6** por conta (+1 em Suporte 30, +1 na conquista "Magnata") | limita especulação e carga no servidor |
| Tipos de oferta | **Compra** (preço máximo) e **Venda** (preço mínimo), quantidade 1–1.000.000 | livro de ofertas clássico |
| Casamento | **prioridade preço → tempo**. O negócio sai **ao preço da oferta que estava esperando no livro** (quem chega depois "aceita" o preço de quem já estava lá). | igual ao RS, e intuitivo |
| Execução parcial | permitida; o restante continua no livro | |
| Garantia (escrow) | ao criar a oferta, o ouro (compra) ou os itens (venda) **saem do Cofre na hora** e ficam presos até executar ou cancelar | impede duplicação e oferta sem fundos |
| Taxa | **2% sobre o valor de cada venda**, arredondada para baixo, teto de **500.000** por negócio; itens épicos pagam **4%**; Suporte 30 tira 0,25 ponto | sumidouro de ouro |
| Limite de compra | por item, a cada **4 h** por conta (ex.: minério 10.000, poção 2.000, arma T5 10, item épico 2) | dificulta monopólio e robôs |
| Faixa de preço | ofertas entre **20% e 500%** do preço guia (fora disso, recusa) | evita lavagem de ouro com preços absurdos |
| Coleta | o que for executado vai para a **Caixa de Coleta** (até 30 dias), depois para o Cofre | |
| Cancelamento | grátis, devolve o que estava em garantia | |
| Itens vinculados | Lendários, itens de missão e cosméticos de conquista **não** entram na Bolsa | |
| Onde usar | balcões na Praça das Balanças e em qualquer banco; do menu principal, **só consulta e cancelamento** (criar oferta exige estar num balcão, para valorizar a cidade) — ver Q8 | |

### 13.2 Preço guia
- `preço_guia = mediana ponderada por volume dos negócios das últimas 24 h`, recalculada **a cada 1 h**. Sem negócios: mantém o valor anterior, com queda de 1% por dia sem negociação (até o piso).
- **Piso** = valor de **Transmutação Áurea** (feitiço de Magia 40 que converte o item em `60% do valor base` em ouro). Esse piso garante que nenhum item feito valha zero, igual à "alquimia" do RS.
- **Valor base** (fixo, nos dados do item) = custo dos materiais × 1,15 + XP × 0,5 (calibrável).

### 13.3 Lojas NPC
- Vendem **itens de entrada** (ferramentas de bronze/ferro, frascos vazios, iscas, sementes básicas, runas de Brisa/Maré, comida T1), com **estoque que repõe** 1 unidade a cada N minutos.
- Preço dinâmico: `preço = valor_base × (1,3 − 0,6 × estoque/estoque_máx)` na compra; na venda ao NPC o jogador recebe `valor_base × 0,4 × (estoque_máx − estoque)/estoque_máx` (cai quando a loja está cheia). Vender para NPC é sempre pior que a Bolsa, de propósito.
- Lojas de guilda (Metalurgistas, Pescadores, Campeões) vendem receitas e ferramentas melhores, exigindo nível.

### 13.4 Fontes e sumidouros de ouro
| Fontes (entra ouro) | Sumidouros (sai ouro) |
|---|---|
| ouro de monstros (tabela 8.1) | taxa da Bolsa (2–4%) |
| vender para NPC | reparo por NPC (§11.4) |
| recompensas de missões e Diário | teleportes por ouro (barqueiro, carroças) |
| Transmutação Áurea (gasta runas) | compras em lojas NPC |
| Espólio da Debandada | construção (pregos, materiais de NPC, licenças de sala) |
| tarefas de Exterminação | respec dos caminhos, expansão do Cofre, espaços de oferta |
| Ladinagem | multa de morte na Marca Cinzenta, taxa do Túmulo (resgate antes de 10 min) |

**Meta de equilíbrio:** sumidouros removem **70–90%** do ouro que entra por semana. Monitoramento: o servidor grava a soma diária de cada fonte/sumidouro (leaderboard interno `econ_daily`) para ajustarmos as taxas.

### 13.5 Implementação no Nakama (autoritativa)
- **Carteira:** ouro na **wallet** nativa do Nakama (`nk.walletUpdate`, com ledger e motivo em `metadata`), que já é atômica e auditável.
- **Ordens:** coleção `market_orders` (dono = usuário do sistema, chave = `order_id`), índice por item em `market_book` (chave = `item_id`, guarda as melhores ofertas resumidas). Histórico em `market_trades` (rolante 7 dias) e `market_price` (preço guia por item).
- **Serialização sem corrida:** um **match autoritativo "singleton"** com label `{"mode":"market"}` criado no `InitModule`, é o **único** que casa ofertas. Os RPCs `market_place_order` e `market_cancel` validam e colocam em garantia (escrita com versão otimista no Cofre + `walletUpdate`) e depois avisam o match via `nk.matchSignal(matchId, payload)`. O match processa o sinal **em fila**, casa as ofertas e grava os resultados num único `storageWrite` em lote. Isso evita duas ofertas casando contra o mesmo lote ao mesmo tempo.
- **Alternativa** (se o volume crescer): tabelas SQL próprias via `nk.sqlExec` em transação. Começar pelo singleton, que é mais simples e cabe com folga para milhares de ofertas por minuto.
- **RPCs:** `market_place_order`, `market_cancel`, `market_collect`, `market_my_orders`, `market_book(item_id)` (5 melhores de cada lado, agregado por preço), `market_history(item_id)` (gráfico 7 dias), `market_search(texto, filtros)`.
- **Limites:** no máximo 1 oferta nova por segundo por usuário, 60 por hora; o limite de 4 h fica em `market_limits` (chave = usuário).
- **Offline:** não há Bolsa. Existe o **Mercador Ambulante**, NPC com preço fixo = valor base (compra a 40%, vende a 130%) e estoque gerado por semente diária.

### 13.6 Interface
- Tela dividida: à esquerda os 6 espaços de oferta (barra de progresso, quantidade feita, cancelar); à direita, a busca de item com ícone, **preço guia**, gráfico de 7 dias, as 5 melhores ofertas de cada lado e o limite de 4 h restante.
- Criar oferta: quantidade (atalhos 1/10/100/Tudo), preço (atalhos −5%/guia/+5%), custo/ganho estimado **já com a taxa**.
- Notificação in-game quando uma oferta completa.

---

## 14. Missões (quests)

Missões ficam no **Diário** (tecla J). Cada uma tem requisitos (níveis e missões anteriores), passos com marcadores no mapa e recompensas: XP em habilidades (lâmpadas), ouro, itens e **desbloqueios** (§16). **Pontos de Missão (PM)** somados liberam conteúdo (ex.: Guilda dos Campeões exige 12 PM).

| # | Missão | Requisitos | Resumo | Recompensas |
|---:|---|---|---|---|
| 1 | **Os Sinos Calados** (tutorial) | — | Chegar à Vila de Trigal, aprender a coletar, fabricar e lutar; os sinos da Capital não tocam desde o regicídio | 1 PM; kit de bronze; acesso à Capital |
| 2 | **Pão para os Vivos** | — | Moer trigo, assar pão e alimentar o abrigo da Catedral | 1 PM; 300 XP Culinária, 150 Agricultura |
| 3 | **Pedágio de Sangue** | Combate 8 | Acabar com o pedágio do Vau dos Bandidos | 1 PM; 500 XP Ataque/Pontaria/Magia (à escolha); Besta de Ferro |
| 4 | **Ratos no Porão do Rei** | — | Investigar os esgotos sob a Taverna; descobrir a Mãe Sarnenta | 2 PM; acesso às Galerias Afogadas; Lanterna de Óleo |
| 5 | **A Fornalha Fria** | Metalurgia 10, Mineração 10 | Reacender a Grande Fornalha com o anão Brom | 1 PM; 1.000 XP Metalurgia; Anel de Forja |
| 6 | **Linhas no Rio** | Pesca 15 | Ajudar Tião a descobrir por que os peixes do cais morrem | 1 PM; 1.200 XP Pesca; acesso ao Cais Fundo |
| 7 | **Totens de Lama** | Combate 10 | Destruir os 3 totens do Acampamento Goblin | 2 PM; Machadinha de Aço; título "Quebra-Totens" |
| 8 | **A Última Lanterna** | Mineração 15, Fogaréu 10 | Entrar na Mina do Velho Anão e salvar mineiros presos | 2 PM; acesso à Mina; Picareta de Ferro Fina |
| 9 | **O Colégio Calado** | Magia 15, Runografia 10 | Descobrir por que os aprendizes viram Magos Ocos | 2 PM; Teleporte: Portão da Capital; Cajado T2 Raro |
| 10 | **Sementes de Cinza** | Agricultura 20, Herbologia 15 | Recuperar a semente sagrada roubada pelos Druidas da Podridão | 2 PM; Lírio-Negro (semente); 2.000 XP Herbologia |
| 11 | **Telhados à Meia-Noite** | Agilidade 20, Ladinagem 20 | Perseguir um ladrão pelos telhados enquanto as gárgulas acordam | 2 PM; Circuito das Muralhas; Martelo de Gárgula (receita) |
| 12 | **O Senhor da Clareira** | Combate 12 | Matar o Senhor da Clareira e libertar a floresta | 2 PM; acesso à Mata dos Enforcados pela trilha sul; Coroa de Galhos (chance aumentada) |
| 13 | **Debaixo da Pedra** | Combate 25, missão 8 | Enfrentar o Capataz de Pedra | 3 PM; Bigorna Ancestral (Construção); 5.000 XP Mineração |
| 14 | **Lume na Escuridão** | Fogaréu 25, Combate 25 | Guiar uma expedição na Caverna dos Ecos | 2 PM; Lanterna Selada; acesso ao fosso |
| 15 | **Juramento Quebrado** | Combate 28, Exterminação 15 | Infiltrar a Fortaleza Corvo-Negro e achar a insígnia do regicida | 3 PM; Escudo Corvo (T3 épico); pista da linha principal |
| 16 | **O Rei Que Esqueceu** (parte 1) | Combate 30, Ladinagem 30, Arqueologia 20 | Descer às Catacumbas da Coroa atrás do primeiro rei | 3 PM; acesso às Catacumbas; Máscara Vazia (chance) |
| 17 | **O Rei Que Esqueceu** (parte 2) | missão 16, Arqueologia 30 | Restaurar o rosto do Rei Sem Rosto com fragmentos do Templo Afundado | 3 PM; Relíquia: Coroa Restaurada (+5% XP de Arqueologia) |
| 18 | **Fogo nos Campos** (evento repetível) | Combate 20 | Defender a Vila de Trigal numa Incursão (ondas — uma Debandada no mundo) | 1 PM (primeira vez); espólio de Incursão |
| 19 | **O Pacto do Corvo** | Invocação 30, Necromancia 25 | Fazer pacto com o Corvo do Abismo | 2 PM; familiar Corvo (Invocação 50 para usar) |
| 20 | **A Casa Própria** | Construção 20 | Ganhar terreno na encosta e erguer a Fortaleza pessoal | 1 PM; Baú do Acampamento; teleporte para a Fortaleza |
| 21 | **Ouro de Tolo** | Metalurgia 35, Artesanato 35 | Descobrir quem falsifica moedas na Bolsa | 2 PM; Luvas de Ourives; −0,25 ponto na taxa da Bolsa |
| 22 | **A Torre do Relógio** | Construção 35, Metalurgia 30, Agilidade 35 | Consertar o relógio da Cidadela, parado na hora do regicídio | 3 PM; os sinos voltam a tocar (evento global), Circuito da Cidadela |
| 23 | **Regicida** | Combate 42, missões 15 e 22 | Encarar o Comandante Corvo-Negro e descobrir quem mandou matar o Imperador | 4 PM; título "Vingador da Coroa"; Espada do Regicida (chance) |
| 24 | **Brasa Sob a Coroa** (final do reino) | Combate 45, missão 23, 30 PM | O Conselho abriu a Fenda para chamar o Arquiduque; fechar a Fenda | 5 PM; acesso ao Coração da Fenda; Capa do Império; desbloqueia o 2º reino (futuro) |
| 25 | **Diário da Professora Pó** (repetível semanal) | Arqueologia 10 | Doar artefatos ao Museu | reputação do Museu, lâmpadas de XP |

**Total:** 23 missões únicas + 2 repetíveis (18 e 25), **51 PM** disponíveis.

---

## 15. Conquistas e o Diário da Capital

Dois sistemas:
- **Diário da Capital** (inspirado nos Achievement Diaries): tarefas por **dificuldade** (Fácil, Médio, Difícil, Elite) ligadas ao reino. Completar um nível inteiro dá a **Armadura da Capital** do nível, com bônus de profissão (ex.: chance de minerar 2 minérios, fundir 2 barras, como a Armadura de Varrock).
- **Conquistas gerais:** marcos de conta com pontos, títulos e cosméticos.

### 15.1 Diário da Capital
| Nível | Tarefas (exemplos) | Recompensa |
|---|---|---|
| **Fácil** (10 tarefas) | minerar Cobre e Estanho no mesmo minuto · forjar uma Adaga de Bronze · pescar um Camarão no Cais · assar Pão no moinho · bater carteira de um Cidadão · terminar o Circuito dos Telhados · comprar algo na Bolsa · rezar no altar da Catedral · matar um Goblin Soldado · plantar batatas em Trigal | Armadura da Capital I (10% de chance de minério duplo até Ferro) + 2 lâmpadas de 500 XP |
| **Médio** (12 tarefas) | fundir uma Barra de Aço · pescar Salmão · matar a Mãe Sarnenta · criar uma Runa da Mente · fazer uma Poção de Defesa · terminar o Circuito das Muralhas · acender fogueira de Salgueiro na Mata · escavar no Acampamento Legionário · domar uma Raposa-Cinza · vender 1.000 itens na Bolsa · teleportar ao Portão · completar uma tarefa de Exterminação média | Armadura II (até Auralita; 10% de barra dupla até Aço) + Teleporte diário para a Forja |
| **Difícil** (12 tarefas) | forjar um Peitoral de Auralita · matar a Viúva do Breu sem tocha (desafio) · cozinhar Lagosta sem queimar 28 seguidas · fazer Elixir de Magia · Circuito da Cidadela · cortar Teixo-Negro · matar o Rei Sem Rosto · restaurar artefato do Templo Afundado · subir a Construção ao Altar · completar 5 Incursões · Fogo Sagrado nas Catacumbas · andar 20 das Profundezas | Armadura III (até Vermécio) + espaço extra na Bolsa |
| **Elite** (8 tarefas) | forjar um item de Sombraço · pescar Enguia-Abissal · matar o Arquiduque · Runa de Sangue · Elixir Supremo · Circuito do Campanário · andar 30 das Profundezas · Nível Total 1.000 | Armadura IV (qualquer minério/barra) + Capa do Diário (teleporte ilimitado à Capital) |

### 15.2 Conquistas gerais (40+)
| # | Conquista | Condição | Recompensa |
|---:|---|---|---|
| 1 | Primeiro Sangue | matar o primeiro monstro no Crônicas | 5 pts |
| 2 | Mãos Calejadas | chegar ao nível 10 em qualquer habilidade de Coleta | 5 pts |
| 3 | Ferreiro de Aldeia | Metalurgia 20 | 10 pts, título "Ferreiro" |
| 4 | Mestre da Forja | Metalurgia 50 | 25 pts, Capa do Artífice (variante) |
| 5 | Lenhador de Ferro | 1.000 toras cortadas | 10 pts |
| 6 | Pescador de Histórias | pescar todas as espécies da Capital | 15 pts, título "Contador de Peixes" |
| 7 | Cozinheiro Sem Fumaça | 500 comidas sem queimar seguidas | 15 pts |
| 8 | Alquimista | fazer todas as poções até 50 | 20 pts |
| 9 | Voz das Runas | criar todas as runas | 15 pts |
| 10 | Mão Leve | 1.000 roubos bem-sucedidos | 10 pts, título "Mão Leve" |
| 11 | Corredor de Telhados | 100 voltas de Agilidade | 10 pts |
| 12 | Senhor do Feudo | Construção 50 | 25 pts |
| 13 | Domador | invocar 10 familiares diferentes | 15 pts |
| 14 | Arqueólogo Imperial | completar a coleção do Museu | 25 pts, relíquia Busto do Primeiro Imperador |
| 15 | Rato de Esgoto | matar 500 Ratos-de-Esgoto | 5 pts, cosmético "Cauda de Rato" |
| 16 | Matador de Gigantes | matar os 4 tipos de gigante | 15 pts |
| 17 | Caçador de Chefes | matar todos os 9 chefes | 30 pts, título "Caçador de Chefes" |
| 18 | Sem Luz, Sem Medo | Viúva do Breu sem tocha | 20 pts |
| 19 | Mão Única | matar o Rei Sem Rosto solo | 20 pts |
| 20 | Regicida do Regicida | matar o Comandante sem morrer | 20 pts |
| 21 | Apagador de Brasas | matar o Arquiduque | 30 pts, título |
| 22 | Esquadrão de Ferro | matar um chefe com 4 jogadores sem ninguém cair | 15 pts |
| 23 | Mestre Exterminador | 100 tarefas de Exterminação | 25 pts |
| 24 | Profundo | andar 30 das Profundezas | 25 pts |
| 25 | Magnata | 10 milhões de ouro negociados na Bolsa | 20 pts, +1 espaço de oferta |
| 26 | Comerciante Honesto | 1.000 ofertas completas | 10 pts |
| 27 | Mecenas | gastar 1 milhão em taxas (o jogo agradece) | 10 pts, título "Mecenas" |
| 28 | Total 500 | Nível Total 500 | 15 pts |
| 29 | Total 1.000 | Nível Total 1.000 | 25 pts |
| 30 | Total Máximo | todas as habilidades ativas no 50 | 50 pts, Capa do Império Completo |
| 31 | Coletor-Mestre | Nível de Ramo Coleta 50 | 30 pts |
| 32 | Campeão | Nível de Ramo Combate 50 | 30 pts |
| 33 | Artífice | Nível de Ramo Suporte 50 | 30 pts |
| 34 | Missionário | todas as missões da Capital | 30 pts, Capa de Missões |
| 35 | Guardião de Trigal | 25 Incursões defendidas | 15 pts |
| 36 | Noite Eterna | sobreviver a uma noite inteira na Mata dos Enforcados | 10 pts |
| 37 | Sinos Novamente | concluir "A Torre do Relógio" | 10 pts |
| 38 | Debandada I–V | vencer a Debandada em cada Tormento | 5–25 pts, molduras de retrato |
| 39 | Ponte Entre Mundos | usar no Crônicas um item obtido no Espólio da Debandada | 5 pts |
| 40 | Nove Vidas | levar as 9 classes ao rank 12 | 25 pts |
| 41 | Caminho Completo | adquirir os 48 nós de um caminho (Crônicas) | 20 pts por caminho |
| 42 | Obra-Prima | fabricar 10 Obras-Primas | 15 pts |
| 43 | Fortuna do Diabrete | recuperar um item roubado por um Diabrete | 5 pts |
| 44 | Inquilino da Fenda | passar 1 h seguida na Fenda Infernal | 15 pts |

Pontos de conquista liberam molduras, poses de vitória para a Ficha do Herói e **espaços extras no Cofre** (+40 a cada 100 pts).

---

## 16. Sistema de desbloqueios

Todo desbloqueio é uma regra de dados avaliada **no servidor**:

```ts
type UnlockRequirement =
  | { kind: 'skill'; skill: SkillId; level: number }
  | { kind: 'branch'; branch: 'gathering' | 'combat' | 'support'; level: number }
  | { kind: 'combat_level'; level: number }
  | { kind: 'total_level'; level: number }
  | { kind: 'quest'; quest_id: string; stage?: number }
  | { kind: 'quest_points'; points: number }
  | { kind: 'achievement'; achievement_id: string }
  | { kind: 'diary'; tier: 'easy' | 'medium' | 'hard' | 'elite' }
  | { kind: 'class_rank'; class_id: ClassId; rank: number }
  | { kind: 'item_owned'; item_id: string };
```

| O que é desbloqueado | Exemplos | Checado quando |
|---|---|---|
| **Uso de item** | arma T4 exige Ataque 30; armadura T5 exige Defesa 40 | equipar (RPC `inventory_equip`) e ao entrar no mundo |
| **Receita** | Peitoral de Auralita (Metalurgia 39) | `craft` |
| **Recurso** | veio de Vermécio (Mineração 40) | `gather_start` |
| **Área** | Guilda dos Metalurgistas (Metalurgia 30), Catacumbas (missão 16) | portas/gatilhos de zona (o servidor rejeita a entrada) |
| **Atalho** | tronco sobre o rio (Agilidade 10) | `use_shortcut` |
| **Feitiço / oração** | Teleporte: Forja (Magia 30 + missão 9) | ao conjurar |
| **Familiar** | Mula de Carga (Invocação 20) | ao invocar |
| **Conteúdo de outro modo** | Tormento III da Debandada (rank 20 da classe); cosmético da Debandada usável no mundo | ao entrar na fila |
| **Interface** | 7º espaço da Bolsa, abas do Cofre | ao abrir a tela |

O cliente recebe a lista de desbloqueios já resolvida no `load_world_profile` (só para mostrar cadeados e tooltips); a autoridade continua no servidor.


---

## 17. Arquitetura técnica: dados, servidor, cliente

### 17.1 Princípios (mantidos do projeto atual)
1. **Servidor é a fonte única** dos dados (`nakama/src/*.ts`), exportados para o cliente por `npm run export-data`.
2. **Regras puras e testáveis** separadas das chamadas Nakama (como em `squad_logic.ts` e `progression.ts`), com cobertura ≥ 80% (vitest).
3. **Storage só escrito pelo servidor** (`permissionWrite: 0`) com **versão otimista** e nova tentativa (padrão de `grantMatchXp`).
4. **Offline** espelha as regras em GDScript (`LocalMatchHost`) com o mesmo protocolo de op codes.

### 17.2 Novos módulos do servidor
```
nakama/src/
  world/
    skills.ts          # SkillDefinition, curva de XP, nível de ramo, nível de combate
    items.ts           # ItemDefinition (catálogo), raridade, afixos
    recipes.ts         # RecipeDefinition (fundir, forjar, cozinhar...)
    resources.ts       # ResourceNodeDefinition (veios, árvores, pontos de pesca)
    monsters.ts        # MonsterDefinition + fórmulas (substitui/estende ENEMY_ARCHETYPES)
    drops.ts           # DropTable, rolagem com RNG do servidor
    quests.ts          # QuestDefinition, estados, validação de passos
    achievements.ts    # AchievementDefinition, contadores
    unlocks.ts         # avaliação de UnlockRequirement
    zones.ts           # ZoneDefinition, chunks, masmorras (generaliza level.ts)
    world_match.ts     # match autoritativo do mundo aberto (instância)
    world_logic.ts     # regras puras: coleta, combate PvE no mundo, respawn
  economy/
    inventory.ts       # Cofre, Mochila, equipamento (RPCs + regras puras)
    market.ts          # RPCs da Bolsa
    market_match.ts    # match singleton que casa ofertas
    shops.ts           # lojas NPC
  tools/export-data    # gera game_data.json (Debandada) + world_data.json (Crônicas)
```
Os dados do mundo serão grandes (centenas de itens e receitas). Exportar em **`godot/data/world_data.json`** separado, para não inchar o `game_data.json` da Debandada.

### 17.3 Esquemas de dados (TypeScript)
```ts
// ---------- Habilidades ----------
export type SkillBranch = 'gathering' | 'combat' | 'support';
export type SkillId =
  | 'attack' | 'strength' | 'defence' | 'ranged' | 'magic' | 'necromancy' | 'constitution'
  | 'devotion' | 'slaying' | 'delving'
  | 'mining' | 'woodcutting' | 'fishing' | 'hunter' | 'farming' | 'archaeology'
  | 'smithing' | 'cooking' | 'firemaking' | 'fletching' | 'crafting' | 'runecrafting'
  | 'herblore' | 'construction' | 'agility' | 'thieving' | 'summoning' | 'sailing';

export interface SkillDefinition {
  id: SkillId;
  name: string;                 // pt-BR
  branch: SkillBranch;
  enabled: boolean;             // sailing = false até o 2º reino
  max_level: number;            // 50
  unlocks: { level: number; description: string; unlock_ids: string[] }[];
}

export interface SkillState { xp: number; level: number }   // level derivado de xp (sanitize)
export interface WorldProfile {                            // storage: world_profile / 'profile'
  skills: { [id in SkillId]?: SkillState };
  quest_points: number;
  quests: { [questId: string]: { stage: number; completed: boolean } };
  achievements: { [id: string]: { progress: number; done_at?: number } };
  diary: { easy: number; medium: number; hard: number; elite: number }; // bitmask de tarefas
  path_points: { [tree in TreeId]?: { [cardId: string]: number } };     // caminhos permanentes
  last_position: { zone_id: string; x: number; y: number };
  updated_at: number;
}

// ---------- Itens ----------
export type ItemCategory = 'weapon' | 'armor' | 'tool' | 'jewelry' | 'resource' | 'material'
  | 'food' | 'potion' | 'rune' | 'ammo' | 'quest' | 'cosmetic' | 'pact' | 'artifact';
export type EquipSlot = 'head' | 'shoulders' | 'chest' | 'hands' | 'legs' | 'feet' | 'cape'
  | 'amulet' | 'ring' | 'main_hand' | 'off_hand';
export type ItemRarity = 'common' | 'uncommon' | 'rare' | 'epic' | 'legendary';

export interface ItemDefinition {
  id: string;                   // 'auralite_platebody'
  name: string;                 // 'Peitoral de Auralita'
  category: ItemCategory;
  tier: number;                 // 1..6 (0 = sem tier)
  rarity: ItemRarity;           // raridade base (instâncias podem subir com afixos)
  stackable: boolean;
  max_stack: number;            // 1 para equipamento, 1e6 para recursos
  base_value: number;           // valor base em ouro (Transmutação = 60%)
  tradeable: boolean;
  slot?: EquipSlot;
  weapon_family?: string;       // 'two_handed_axe', 'mace', 'longbow'...
  classes?: ClassId[];          // classes que podem equipar (vazio = todas)
  requirements: UnlockRequirement[];
  modifiers: Omit<StatModifier, 'target'>[];   // mesmo modelo de runas e cartas
  consumable?: { heal?: number; effect_id?: string; duration_s?: number; cooldown_group: string };
  durability?: number;
  buy_limit_4h: number;
  icon: string;                 // 'res://art/icons/items/auralite_platebody.png'
}

/** Instância única (só para itens com afixos/durabilidade). Recursos ficam só como pilha. */
export interface ItemInstance {
  uid: string;                  // gerado no servidor (anti-duplicação)
  item_id: string;
  quality: 'normal' | 'fine' | 'masterwork';
  affixes: Omit<StatModifier, 'target'>[];
  durability: number;
  crafted_by?: string;          // nome do jogador (orgulho de artesão)
  bound: boolean;
}

export interface InventorySlot { item_id: string; qty: number; instance?: ItemInstance; locked?: boolean }
export interface InventoryContainer {           // storage: inventory / 'vault' | 'bag_<class>' | 'equip_<class>'
  capacity: number;
  slots: (InventorySlot | null)[];
}

// ---------- Receitas e recursos ----------
export interface RecipeDefinition {
  id: string;
  skill: SkillId;
  level: number;
  station: 'furnace' | 'great_furnace' | 'anvil' | 'range' | 'fire' | 'loom' | 'spinning_wheel'
    | 'tannery' | 'workbench' | 'altar' | 'rune_altar' | 'obelisk' | 'kiln' | 'none';
  inputs: { item_id: string; qty: number }[];
  outputs: { item_id: string; qty: number }[];
  xp: number;
  action_time_s: number;
  fail_chance?: number;         // ex.: Ferro sem Anel de Forja, queimar comida
  requirements?: UnlockRequirement[];
}

export interface ResourceNodeDefinition {
  id: string;                   // 'iron_vein'
  skill: SkillId;
  level: number;
  tool_family: 'pickaxe' | 'hatchet' | 'net' | 'rod' | 'fly_rod' | 'cage' | 'harpoon' | 'trap' | 'trowel' | 'none';
  yields: { item_id: string; qty: number; chance: number }[];   // casca como subproduto
  xp: number;
  base_time_s: number;
  depletion: { min_actions: number; max_actions: number; respawn_s: number };
  night_only?: boolean;
}

// ---------- Monstros e saque ----------
export type MonsterArchetype = 'minion' | 'runner' | 'brute' | 'shooter' | 'caster' | 'swarm' | 'passive';
export interface MonsterDefinition {
  id: string;
  name: string;
  level: number;
  archetype: MonsterArchetype;
  elite: boolean;
  boss: boolean;
  rig_family: 'R1' | 'R2' | 'R3' | 'R4' | 'R5' | 'R6' | 'R7' | 'R8' | 'R9';
  zones: string[];
  night_only?: boolean;
  slayer_level?: number;        // Exterminação exigida para ferir
  required_item?: string;       // ex.: 'gargoyle_hammer'
  abilities: string[];          // ids de habilidades de monstro (veneno, teia, teleporte...)
  phases?: { hp_pct: number; abilities: string[] }[];
  drop_table_id: string;
  overrides?: Partial<{ hp: number; dps: number; speed: number; radius: number }>; // valores herdados da Debandada
}

export interface DropEntry { item_id: string; min: number; max: number; weight: number }
export interface DropTable {
  id: string;
  always: DropEntry[];
  rolls: number;                // rolagens no "main table"
  main: DropEntry[];            // peso relativo; inclui { item_id: 'nothing' }
  rare: { chance: number; entries: DropEntry[] };   // ex.: 1/128
  unique?: { chance: number; entries: DropEntry[] }; // ex.: 1/512, com proteção de azar
  gold: { min: number; max: number };
}

// ---------- Missões e conquistas ----------
export interface QuestStep {
  id: string;
  description: string;
  objective:
    | { kind: 'talk'; npc_id: string }
    | { kind: 'kill'; monster_id: string; count: number }
    | { kind: 'gather'; item_id: string; count: number }
    | { kind: 'craft'; recipe_id: string; count: number }
    | { kind: 'deliver'; npc_id: string; item_id: string; count: number }
    | { kind: 'reach'; zone_id: string; x: number; y: number; radius: number }
    | { kind: 'event'; event_id: string };
}
export interface QuestDefinition {
  id: string; name: string; description: string;
  requirements: UnlockRequirement[];
  steps: QuestStep[];
  rewards: { quest_points: number; xp: { skill: SkillId; amount: number }[]; items: { item_id: string; qty: number }[]; gold: number; unlocks: string[] };
  repeatable?: { reset: 'daily' | 'weekly' };
}
export interface AchievementDefinition {
  id: string; name: string; description: string;
  category: 'skill' | 'combat' | 'quest' | 'economy' | 'exploration' | 'debandada' | 'diary';
  diary_tier?: 'easy' | 'medium' | 'hard' | 'elite';
  counter: string;              // ex.: 'kill:sewer_rat', 'craft:any', 'gold_traded'
  target: number;
  points: number;
  rewards: { title?: string; cosmetic_id?: string; unlocks?: string[] };
}

// ---------- Mercado ----------
export interface MarketOrder {
  order_id: string;
  user_id: string;
  item_id: string;
  side: 'buy' | 'sell';
  price: number;                // por unidade
  qty: number;
  filled: number;
  escrow_gold: number;          // compras
  status: 'open' | 'filled' | 'cancelled' | 'expired';
  created_at: number;
  updated_at: number;
}
export interface MarketTrade {
  trade_id: string; item_id: string; price: number; qty: number;
  buy_order_id: string; sell_order_id: string; tax: number; at: number;
}

// ---------- Mundo ----------
export interface ZoneDefinition {
  id: string; name: string; level_min: number; level_max: number;
  bounds: Rect;                 // em px de arena (40 px/m)
  chunk_ids: string[];
  monsters: { monster_id: string; camp: Vec2; count: number; respawn_s: number }[];
  resources: { node_id: string; at: Vec2 }[];
  danger: 'safe' | 'normal' | 'high';
  dungeon?: { instance_map: string; party_max: number; requirements: UnlockRequirement[] };
}
```

### 17.4 Coleções de storage
| Coleção | Chave | Dono | Conteúdo |
|---|---|---|---|
| `player_progression` *(existe)* | `<class_id>` | usuário | rank, runas |
| `world_profile` | `profile` | usuário | habilidades, missões, conquistas, caminhos permanentes |
| `inventory` | `vault`, `bag_<class>`, `equip_<class>` | usuário | contêineres |
| `market_orders` | `order_id` | sistema | ofertas |
| `market_book` | `item_id` | sistema | resumo do livro |
| `market_price` | `item_id` | sistema | preço guia, histórico horário |
| `market_limits` | usuário | sistema | limite de 4 h |
| `farming_patches` | `patch_id` | usuário | o que foi plantado e quando |
| `fortress` | `layout` | usuário | salas da Fortaleza (Construção) |
| **Wallet** (nativa) | — | usuário | ouro |
| **Leaderboards** | `skill_<id>`, `total_level`, `boss_<id>_fastest`, `econ_daily` | — | rankings e telemetria da economia |

### 17.5 RPCs novos
`load_world_profile`, `world_join` (encontra ou cria instância), `inventory_move`, `inventory_split`, `inventory_discard`, `inventory_equip`, `craft` (receita + quantidade), `farming_plant`, `farming_harvest`, `quest_accept`, `quest_abandon`, `shop_buy`, `shop_sell`, `path_buy_node`, `path_respec`, e os da Bolsa (§13.5). **Coleta e combate no mundo** não são RPC: são **mensagens dentro do match** (op codes novos), porque dependem de posição e tempo.

### 17.6 Novos op codes (match do mundo)
| Op | Direção | Payload |
|---|---|---|
| `GATHER_START` | cliente → servidor | `{ node_uid }` |
| `GATHER_TICK` | servidor → cliente | `{ node_uid, success, item_id?, xp?, depleted? }` |
| `GATHER_STOP` | ambos | `{ node_uid, reason }` |
| `LOOT_SPAWN` / `LOOT_PICKUP` | ambos | `{ loot_uid, item_id, qty, x, y, owner? }` |
| `NODE_STATE` | servidor → cliente | lote de nós esgotados/renascidos no chunk |
| `CHUNK_ENTER` / `CHUNK_LEAVE` | cliente → servidor | assinatura de interesse (AOI) |
| `INTERACT` | cliente → servidor | `{ target_uid, action }` (NPC, porta, atalho, estação) |
| `SKILL_XP` | servidor → cliente | `{ skill, xp, level_up? }` |
| `WORLD_EVENT` | servidor → cliente | Incursão, noite, chefe de mundo |

### 17.7 Cliente Godot
- **Estados novos** no `GameStateManager`: `WORLD_LOADING → WORLD → (DUNGEON) → WORLD`, e as sobreposições `INVENTORY`, `SKILLS`, `MAP`, `JOURNAL`, `MARKET`, `HERO_SHEET`.
- **Mundo por chunks:** `WorldStreamer.gd` carrega os chunks a até 2 de distância (terreno via `LevelGeometry.gd` generalizado, vegetação por MultiMesh, props por cena) e descarrega os distantes. A Clareira atual vira **um conjunto de chunks** do mundo.
- **Interação:** `Interactable.gd` (nós de recurso, NPCs, estações) com destaque ao mirar, círculo de progresso da ação e animação do herói por ferramenta (picareta, machadinha, vara, martelo). A animação procedural do `ModularCharacter.gd` ganha as famílias "minerar", "cortar", "pescar", "forjar", "cozinhar".
- **UI:** reaproveitar o tema atual dos menus (mesma tipografia, molduras e cores) para Inventário, Habilidades, Bolsa, Diário e Ficha do Herói.

---

## 18. Multiplayer no mundo aberto

### 18.1 Opções
| Opção | Como funciona | Prós | Contras |
|---|---|---|---|
| **A. Instância por grupo** | cada grupo (1–4) recebe seu próprio match do reino | igual ao que já existe (squad match), simples, barato, offline idêntico | mundo "vazio" de outros jogadores |
| **B. Fragmentos compartilhados** | matches do reino com até **24–32** jogadores, com área de interesse por chunk | mundo vivo, encontros, Bolsa e cidade cheias | mais banda e CPU por match; grupos precisam cair no mesmo fragmento |
| **C. Mundo único sem fragmentos** | todos no mesmo match | — | não escala no modelo de match do Nakama (um match roda num nó, num loop único) |

O Nakama roda **matches autoritativos com loop de tick** (até ~30 Hz) e um nó aguenta centenas de matches pequenos. Um match único com centenas de jogadores e milhares de monstros não é o caso de uso dele.

### 18.2 Recomendação
1. **Fase 1 (M6–M11): opção A.** Cada grupo tem a sua instância do reino (o **mesmo** formato do squad match atual, 10 Hz). Recursos e monstros são por instância, e a **economia é global** (Cofre, Bolsa, habilidades no storage/RPC, fora do match). Offline funciona igual (1 jogador + IA).
2. **Fase 2 (M8+): opção B só para a cidade.** A **Capital murada** vira um "hub social" compartilhado (até 32 jogadores por fragmento, sem combate, com Bolsa, bancos, forja, cais). Os arredores e as masmorras continuam instanciados por grupo. Portões fazem a troca de match (tela de carregamento curta).
3. **Masmorras:** sempre instâncias de grupo (1–4), com escala de vida por jogador.
4. **Formação de grupo:** usar o **sistema de party** do Nakama (hoje `partyId` é ignorado no matchmaker): convite por amigo/código, o líder entra no mundo e os outros seguem para a mesma instância.

**Orçamento de servidor (estimativa para planejar):** match do reino com 4 jogadores, ~150 monstros ativos nos chunks acordados, 10 Hz → parecido com uma Debandada na onda 10. Hub com 32 jogadores sem monstros, 10 Hz, só posições num raio de 40 m → leve.

---

## 19. Antitrapaça e integridade da economia

Hoje o cliente controla o próprio movimento (validado por velocidade) e reporta dano (com teto). Com economia persistente, o risco sobe. Regras:

| Ameaça | Defesa |
|---|---|
| Coleta falsa / rápida demais | o servidor rola o sucesso; `GATHER_START` exige distância ≤ 2 m do nó, ferramenta e nível; o ritmo segue `tempo_real` do servidor, não do cliente |
| Saque inventado | todo saque nasce no servidor (`LOOT_SPAWN` com `loot_uid`), só pode ser pego uma vez e a até 3 m |
| Duplicação de itens | escrita de inventário com **versão otimista**; itens com instância têm `uid` único; a Bolsa usa garantia (escrow) e um único match casando ofertas |
| Teletransporte / speedhack | validação de velocidade (já existe) + checagem de colisão contra paredes da cidade e água no servidor |
| Dano inflado | teto por golpe que já existe (`maxDamagePerHit`), agora com o multiplicador da arma e das habilidades do servidor; chefes registram DPS por jogador e sinalizam anomalias |
| Robôs de coleta | limite diário suave de XP (acima de 12 h de ação contínua, XP cai para 50%); eventos aleatórios leves ("o Diabrete roubou sua picareta"); limites de 4 h na Bolsa |
| Lavagem de ouro | faixa de preço 20–500% do guia, taxa, histórico auditável, limite de envio direto entre contas (sem troca direta na fase 1; tudo pela Bolsa) |
| Abuso offline → online | perfil offline **nunca** sobe para o online (já é assim com o rank) |

Logs: toda alteração de ouro e item raro grava motivo (`metadata` da wallet; coleção `audit_log` com retenção de 30 dias).

---

## 20. Roteiro (roadmap) por fases

> **Substituído:** o roteiro vigente é o de [`PLANO_DUNGEON_STANCE_UE5.md`](PLANO_DUNGEON_STANCE_UE5.md) §5. As fases M0–M5 abaixo correspondem ao conteúdo do Dungeon Stance (Fases 1–3 do plano UE5); M6 em diante, à expansão MMORPG (Fase 4). A tabela fica como referência de escopo e esforço do protótipo.

Esforço em **semanas de 1 pessoa** (estimativa grossa, para priorizar).

| Fase | Entrega | Inclui | Depende de | Esforço |
|---|---|---|---|---:|
| **M0** | Debandada renomeada | textos, `game_mode`, label `debandada`, README/CLASSES | — | 0,5 |
| **M1** | Ficha do Herói 3D | submenu com SubViewport, poses, abas de atributos/skill/runas/lore | — | 2 |
| **M2** | Árvores 48 × 4 (fase 1) | layout 4×12, pais valendo, todos os nós [N] e de stats existentes; UI do grafo; nós [S]/[G] com `weight: 0` | — | 3 |
| **M3** | Sistema de status em inimigos | marca, sangramento, queimadura, veneno, lentidão, atordoamento no `squad_logic.ts` + espelho offline; libera os nós [S] | M2 | 3 |
| **M3b** | Nós de gatilho [G] | procs, auras, escudos condicionais, 4 habilidades novas do Apoio | M3 | 4 |
| **M4** | Inventário Compartilhado + Espólio | Cofre, Cinto de Batalha, RPCs, UI, Espólio da Debandada | — | 3 |
| **M5** | Bestiário fase 1 (rigs R4 e R5) | quadrúpede e artrópode procedurais; 10 monstros da faixa 1–15; arenas Galerias e Cemitério na Debandada | M3 | 5 |
| **M6** | Crônicas do Império — esqueleto | `world_match` (instância por grupo), chunks, streaming, Campos + Clareira + cidade em greybox, Painel de Habilidades, Mineração/Lenhador/Pesca/Metalurgia/Culinária/Fogaréu | M4 | 8 |
| **M7** | Cadeias de produção completas | as 27 habilidades até 50, receitas, recursos, Construção básica, Agricultura em tempo real | M6 | 10 |
| **M8** | Bolsa Imperial | ordens, match singleton, preço guia, lojas NPC, UI | M4, M6 | 4 |
| **M9** | Zonas e masmorras 1–30 | Colinas, Mata, Charco, Galerias, Mina, Caverna, Catacumbas (parte), chefes até a Viúva | M5, M6 | 10 |
| **M10** | Missões 1–14 + Diário Fácil/Médio + 30 conquistas | sistema de missões, diálogos, marcadores | M7, M9 | 6 |
| **M11** | Zonas 30–50 | Dentes do Norte, Fortaleza, Fenda, Marca Cinzenta, chefes finais, rigs R6–R8 | M9 | 10 |
| **M12** | Hub social da Capital | fragmentos compartilhados de 32 jogadores, party do Nakama | M8 | 5 |
| **M13** | Fechamento do reino | missões 15–25, Diário Difícil/Elite, balanceamento da economia, telemetria | tudo | 6 |

**Total aproximado:** ~80 semanas de 1 pessoa (com IA ajudando, bem menos; serve para ordenar). **Primeiro marco jogável do mundo aberto:** fim do M6.

**Ordem recomendada:** M0 → M1 → M2 → M4 (rápidos e visíveis) → M3/M3b → M5 → M6 → M7/M8 em paralelo → M9 → M10 → M11 → M12 → M13.

---

## 21. Perguntas em aberto para decidir

| # | Pergunta | Proposta atual |
|---:|---|---|
| Q1 | "Inventário Compartilhado" é **entre suas classes** (Cofre da Conta), **entre jogadores do grupo**, ou os dois? | os dois: Cofre da Conta + Baú do Esquadrão temporário |
| Q2 | Nome do modo novo: *Crônicas do Império*, *Império*, *Jornada* ou outro? | *Crônicas do Império* |
| Q3 | Equipamento do mundo aberto deve valer **dentro da Debandada**? | não (só Cinto de Batalha), para a Debandada continuar justa |
| Q4 | Habilidades de mundo são **por conta** (compartilhadas entre as 9 classes) ou **por classe**? | por conta; a classe só muda qual habilidade de combate ela usa |
| Q5 | No mundo aberto, os caminhos (48 nós) viram **permanentes** com Pontos de Caminho? | sim, 1 ponto por nível de habilidade de Combate |
| Q6 | Ofícios (Metalurgia, Culinária...) ficam no ramo **Suporte** ou viram um 4º ramo **Produção**? | Suporte (você pediu 3 ramos), com "Produção" só como agrupamento visual |
| Q7 | Escala do mapa: 1 tile do RS = **2 m** (reino 768 × 576 m) está bom, ou preferem maior/menor? | 2 m |
| Q8 | A Bolsa pode ser usada **pelo menu principal** ou só nos balcões da cidade? | consulta/cancelamento no menu; criar oferta só na cidade |
| Q9 | Haverá **PvP** (arena de duelo, Marca Cinzenta com perda de itens)? | só duelo consensual na Arena dos Campeões; Marca Cinzenta PvE com risco maior |
| Q10 | O teto de nível vai subir depois de 50 (para os outros reinos)? Até quanto? | sim, até 99 ou 120, com a mesma fórmula; os 25 reinos já supõem isso |
| Q11 | Mundo compartilhado já no início ou instância por grupo primeiro? | instância por grupo (§18.2) |
| Q12 | A Ficha do Herói deve mostrar o **equipamento visível** do mundo aberto no modelo 3D? | sim, já preparar os pontos de encaixe no `ClassGear.gd` |
| Q13 | Morte no mundo aberto: túmulo de 10 min está bom ou preferem algo mais leve (sem perder nada)? | túmulo de 10 min, 3 espaços protegidos |
| Q14 | Sistema de troca direta entre jogadores (trade) além da Bolsa? | não na fase 1 (reduz golpes e lavagem) |
| Q15 | Monetização/cosméticos pagos entram no plano? | fora do escopo deste documento |

---

## Apêndice A — Fórmulas e tabelas numéricas

### A.1 XP por nível (1–50)
Fórmula do RuneScape, truncada no 50: `XP_total(L) = piso( Σ_{l=1}^{L−1} piso(l + 300·2^(l/7)) / 4 )`.

| Nív | XP total | XP p/ próximo | Nív | XP total | XP p/ próximo |
|---:|---:|---:|---:|---:|---:|
| 1 | 0 | 83 | 26 | 8.740 | 990 |
| 2 | 83 | 91 | 27 | 9.730 | 1.094 |
| 3 | 174 | 102 | 28 | 10.824 | 1.207 |
| 4 | 276 | 112 | 29 | 12.031 | 1.332 |
| 5 | 388 | 124 | 30 | 13.363 | 1.470 |
| 6 | 512 | 138 | 31 | 14.833 | 1.623 |
| 7 | 650 | 151 | 32 | 16.456 | 1.791 |
| 8 | 801 | 168 | 33 | 18.247 | 1.977 |
| 9 | 969 | 185 | 34 | 20.224 | 2.182 |
| 10 | 1.154 | 204 | 35 | 22.406 | 2.409 |
| 11 | 1.358 | 226 | 36 | 24.815 | 2.658 |
| 12 | 1.584 | 249 | 37 | 27.473 | 2.935 |
| 13 | 1.833 | 274 | 38 | 30.408 | 3.240 |
| 14 | 2.107 | 304 | 39 | 33.648 | 3.576 |
| 15 | 2.411 | 335 | 40 | 37.224 | 3.947 |
| 16 | 2.746 | 369 | 41 | 41.171 | 4.358 |
| 17 | 3.115 | 408 | 42 | 45.529 | 4.810 |
| 18 | 3.523 | 450 | 43 | 50.339 | 5.310 |
| 19 | 3.973 | 497 | 44 | 55.649 | 5.863 |
| 20 | 4.470 | 548 | 45 | 61.512 | 6.471 |
| 21 | 5.018 | 606 | 46 | 67.983 | 7.144 |
| 22 | 5.624 | 667 | 47 | 75.127 | 7.887 |
| 23 | 6.291 | 737 | 48 | 83.014 | 8.707 |
| 24 | 7.028 | 814 | 49 | 91.721 | 9.612 |
| 25 | 7.842 | 898 | 50 | 101.333 | — |

### A.2 Atributos de monstro por nível
Lacaio padrão (multiplique pelo arquétipo, §8.1). Ouro médio = `2 + 1,6·L + 0,05·L²`.

| Nív | Vida | DPS contato | Proj. (dano) | Armadura | XP combate | Ouro médio |
|---:|---:|---:|---:|---:|---:|---:|
| 1 | 67 | 17 | 37 | 2 | 7 | 4 |
| 3 | 103 | 26 | 57 | 6 | 9 | 7 |
| 5 | 143 | 35 | 77 | 10 | 11 | 11 |
| 8 | 210 | 48 | 106 | 16 | 15 | 18 |
| 10 | 260 | 57 | 125 | 20 | 17 | 23 |
| 12 | 314 | 66 | 145 | 24 | 20 | 28 |
| 15 | 403 | 80 | 176 | 30 | 24 | 37 |
| 18 | 500 | 93 | 205 | 36 | 29 | 47 |
| 20 | 570 | 102 | 224 | 40 | 33 | 54 |
| 22 | 644 | 111 | 244 | 44 | 36 | 61 |
| 25 | 763 | 125 | 275 | 50 | 42 | 73 |
| 28 | 890 | 138 | 304 | 56 | 49 | 86 |
| 30 | 980 | 147 | 323 | 60 | 53 | 95 |
| 32 | 1074 | 156 | 343 | 64 | 58 | 104 |
| 35 | 1223 | 170 | 374 | 70 | 65 | 119 |
| 38 | 1380 | 183 | 403 | 76 | 73 | 135 |
| 40 | 1490 | 192 | 422 | 80 | 79 | 146 |
| 42 | 1604 | 201 | 442 | 84 | 84 | 157 |
| 45 | 1783 | 215 | 473 | 90 | 93 | 175 |
| 48 | 1970 | 228 | 502 | 96 | 103 | 194 |
| 50 | 2100 | 237 | 521 | 100 | 109 | 207 |

### A.3 Resumo das fórmulas
| Grandeza | Fórmula |
|---|---|
| Vida do monstro | `(50 + 16L + 0,5L²) × mult_arquétipo × (elite ? 2,5 : 1) × (1 + 0,6·(jogadores − 1))` |
| DPS do monstro | `(12 + 4,5L) × mult_arquétipo × (elite ? 1,5 : 1)`; noite ×1,2 |
| XP de combate | `arred(4 + vida_máx × 0,05) × ameaça` (lacaio 1, elite 2,5, chefe 10) |
| Dano do herói | `base_classe × mult_rank × (1 + 0,03·(ofensivo − 1)) × mult_arma × Π(caminhos, afixos)` |
| Vida do herói | `base_classe × mult_rank × (1 + 0,04·(Constituição − 1)) + Σ vida dos itens` |
| Mitigação | herói: `100/(100 + armadura)`; monstro: `200/(200 + armadura)` |
| Tempo de ação de coleta | `tempo_base × (1 − 0,01·(nível − requisito)) × mult_ferramenta`, mín. 55% |
| Chance de "Fino" | `5% + 0,5%·(nível − requisito)`, máx. 25% |
| Cura da comida | `60 + 18·nível_requerido` |
| Nível de Combate | `piso(0,20·Def + 0,20·Const + 0,05·Dev + 0,55·máx((Atq+For)/2, Pont, Mag, Necro))` |
| Nível de Ramo | `piso(média dos níveis das habilidades do ramo)` |
| Taxa da Bolsa | `piso(2% × valor)`, teto 500.000; épico 4% |
| Preço guia | mediana ponderada por volume em 24 h, recalculada a cada 1 h |

---

## Apêndice B — Fontes de pesquisa

Usadas só como **referência de escala, papéis e mecânicas**. Nenhum nome, mapa, castelo ou criatura foi copiado.

| Tema | Fonte |
|---|---|
| Habilidades do OSRS e categorias (combate, coleta, artesão, suporte) | https://oldschool.runescape.wiki/w/Skills |
| Habilidades do RS3 (29 habilidades, Necromancia, Arqueologia, Invenção, Divinação) | https://runescape.wiki/w/Skills |
| Tabela de XP e fórmula (nível 50 = 101.333 XP) | https://oldschool.runescape.wiki/w/Experience · https://osrstoolkit.com/calculators/experience-table/ |
| Grand Exchange (taxa de 2%, teto de 5 milhões, limite de compra a cada 4 h) | https://oldschool.runescape.wiki/w/Grand_Exchange · https://oldschool.runescape.wiki/w/Tax |
| Varrock (marcos: palácio, Grand Exchange, museu, guildas, esgotos, bancos, serraria) | https://oldschool.runescape.wiki/w/Varrock · https://runescape.wiki/w/Varrock_Palace |
| Sistema de regiões do mapa (regiões de 64 × 64 tiles, chunks de 8 × 8) | https://rswiki.moparisthebest.com/Map-Region-System.html · https://oldschool.runescape.wiki/w/RuneScape:Map |
| Barras e níveis de Metalurgia (Bronze 1, Ferro 15, Prata 20, Aço 30, Ouro 40, Mithril 50...) | https://oldschool.runescape.wiki/w/Bars · https://runescape.wiki/w/Smithing/Level_up_table |
| Diários de conquista (Fácil/Médio/Difícil/Elite; Armadura de Varrock) | https://oldschool.runescape.wiki/w/Achievement_Diary |
| Monstros de referência por nível (galinha, goblin, vaca, gigantes, demônios menores...) | https://oldschool.runescape.wiki/w/Monsters |
| Nakama: partidas autoritativas, tick rate, leaderboards, wallet | https://heroiclabs.com/docs/nakama/concepts/multiplayer/authoritative/ |
| Diablo Immortal (painel de personagem com modelo central e slots ao redor) | referência visual do jogo, sem material copiado |

> Observação: os wikis do RuneScape bloquearam a leitura automática (HTTP 403) durante a pesquisa. Os números citados vieram de buscas e resumos dessas páginas, e as dimensões de Varrock em tiles são **estimativas**. Vale conferir à mão antes de fechar o tamanho final do mapa (Q7).
