# Guia de Classes — Squad Roguelike

As 9 classes ficam definidas **no servidor** (`nakama/src/classes.ts`, fonte única). O cliente as recebe pelo RPC `get_class_definitions`, então qualquer número abaixo pode ser balanceado ali sem alterar o cliente. As runas ficam em `nakama/src/runes.ts`.

Os números abaixo valem para a **Debandada** (o modo de ondas atual: 10 ondas, chefes nas ondas 5 e 10). Cada classe pode ser vista em detalhe no jogo em **Classes e Ranks → Ficha do Herói** (modelo 3D, atributos, habilidade, runas e lore).

> **Unidades:** distâncias em pixels de arena (40 px = 1 m). Velocidade de ataque em ataques por segundo. Recarga (cooldown) em segundos.

## Resumo

| Classe | Papel | Vida | Dano | Vel. ataque | Alcance | Mov. | Recurso | Skill (botão direito / RB) |
|---|---|---:|---:|---:|---:|---:|---|---|
| Bárbaro | Melee / Tank | 1400 | 38 | 1,1 | 75 | 190 | Fúria | Redemoinho |
| Cruzado | Tank / Buff | **1500** | 30 | 0,9 | 85 | 180 | Fé | Investida Sagrada |
| Caçador de Demônios | Ranged DPS | 900 | 26 | 2,0 | 420 | **230** | Disciplina | Rajada |
| Monge | Melee / Utilidade | 1050 | 22 | **2,4** | 75 | 220 | Espírito | Onda de Luz |
| Necromante | Invocador / DoT | 950 | 24 | 1,0 | 360 | 195 | Essência | Lança de Ossos |
| Arcanista | Ranged AoE | 850 | 40 | 0,9 | 380 | 200 | Mana | Pulso Arcano |
| Cavaleiro Sangrento | Híbrido / Roubo de vida | 1150 | 30 | 1,4 | 160 | 205 | Sangue | Ceifa Sangrenta |
| Mago | Burst / Longo alcance | 820 | **42** | 0,85 | 560 | 195 | Mana | Meteoro |
| Arqueiro | Ranged DPS / Longo alcance | 900 | 30 | 1,6 | **600** | 215 | Foco | Flecha Perfurante |

Valores comuns a todas as classes, salvo indicação: chance crítica de 5%, dano crítico ×1,5, armadura 10, esquiva (dash) de 170 px com recarga de 1,2 s.

## O que toda classe tem

- **Ataque rápido** (segurar botão esquerdo / RT), mirado no mouse ou no analógico direito. A forma depende do tipo de ataque da classe:
  - **corpo a corpo:** golpe em arco de 110° à frente que acerta só o inimigo mais próximo dentro do arco;
  - **corpo a corpo em área:** o mesmo arco, mas acerta todos dentro dele;
  - **projétil:** disparo na direção da mira, com o alcance da classe;
  - **projétil em área:** disparo que explode ao acertar, ao atingir o terreno ou no fim do alcance.
- **Skill da classe** (botão direito / RB): custa recurso e tem recarga, ambas reduzidas pela redução de recarga (máximo de 60%).
- **Esquiva** (Shift / B): dash com invulnerabilidade de cerca de 0,24 s. O servidor confere a recarga.
- **Pulo** (Espaço / A): até 1,4 m de altura; dá para pular por cima de projéteis inimigos.
- **Recurso:** classes que regeneram (Mana, Fé...) começam a partida cheias. Bárbaro e Cavaleiro Sangrento começam **vazios** e enchem batendo e apanhando.
- **Progressão permanente por classe:** cada rank dá +1% de vida e +0,6% de dano, e libera runas nos ranks **5** e **12**.

---

## Bárbaro
**Melee / Tank.** Gera Fúria ao causar e sofrer dano e ataca em área de curto alcance.

- **Atributos:** 1400 de vida · 38 de dano · 1,1 ataques/s · alcance 75 · armadura **30** · movimento 190.
- **Ataque rápido:** corpo a corpo em área, acerta todos no arco à frente.
- **Fúria:** começa em 0, ganha +6 por acerto e um pouco mais ao sofrer dano, e **escoa 2 por segundo**. Fúria alta aumenta o dano em até **+30%**, de forma proporcional à barra.
- **Skill — Redemoinho:** golpe em área ao redor, raio 140, ×2,2 de dano. Custa 30 de Fúria, recarga de 6 s.
- **Runas:** Fúria Berserker I (rank 5, dano ×1,08) · Pele de Ferro I (rank 12, +15 de armadura).
- **Como jogar:** entre no meio da horda para manter a Fúria alta e use o Redemoinho quando estiver cercado. Funciona bem com o **Caminho da Defesa** (aguenta mais tempo no meio) ou do **Crítico**.

## Cruzado
**Tank / Buff.** Escudos de absorção de dano e suporte para o esquadrão.

- **Atributos:** **1500** de vida (a maior) · 30 de dano · 0,9 ataques/s · alcance 85 · armadura **40** (a maior) · movimento 180 (o mais lento).
- **Ataque rápido:** corpo a corpo em área.
- **Fé:** regenera 5 por segundo, +3 por acerto e +2 ao sofrer dano.
- **Escudo:** começa cada onda da Debandada com **150** de escudo.
- **Skill — Investida Sagrada:** golpe frontal em cone de 80°, alcance 170, ×2,4 de dano. Custa 25 de Fé, recarga de 7 s.
- **Runas:** Consagração I (rank 5, escudo ×1,2) · Aura Sagrada II (rank 12, vida ×1,1).
- **Como jogar:** linha de frente do time. É a escolha natural para o **Caminho do Apoio**: Aura do Guerreiro Perdido e Bastião Compartilhado ficam ótimos em quem já está no meio do grupo.

## Caçador de Demônios
**Ranged DPS.** Velocidade de ataque e mobilidade.

- **Atributos:** 900 de vida · 26 de dano · **2,0 ataques/s** · alcance 420 · crítico 10% · movimento **230** (o mais rápido).
- **Ataque rápido:** projétil rápido (900 px/s).
- **Disciplina:** máximo de 80, regenera 8 por segundo.
- **Esquiva aprimorada:** dash de **230 px** com recarga de **0,8 s** (a melhor do jogo). Também tem **12% de redução passiva** de todo dano recebido.
- **Skill — Rajada:** leque de 5 virotes em 40°, alcance 450, ×1,2 de dano cada. Custa 30, recarga de 5 s.
- **Runas:** Tiro Rápido I (rank 5, velocidade de ataque ×1,12) · Evasão I (rank 12, +5% de redução de dano).
- **Como jogar:** dê dano em movimento e use o dash para reposicionar e escapar dos atiradores. Combina com o **Caminho do Crítico**: a alta cadência aproveita bem a cura e a redução de recarga ao critar.

## Monge
**Melee / Utilidade.** Combos rápidos e proteção.

- **Atributos:** 1050 de vida · 22 de dano · **2,4 ataques/s** (a maior) · alcance 75 · movimento 220 · esquiva com recarga de 0,9 s.
- **Ataque rápido:** corpo a corpo de alvo único (o inimigo mais próximo no arco).
- **Combo:** a cada **3º** ataque rápido, o golpe causa **×2** de dano.
- **Espírito:** regenera 4 por segundo e +5 por acerto.
- **Escudo:** começa cada onda com **80** de escudo.
- **Skill — Onda de Luz:** explosão ao redor, raio 120, ×2,5 de dano. Custa 30, recarga de 5 s.
- **Runas:** Golpes Fluidos I (rank 5, velocidade de ataque ×1,1) · Serenidade I (rank 12, escudo ×1,25).
- **Como jogar:** cole em um alvo forte (brutamontes) para fechar combos e use a Onda de Luz para limpar os menores. Funciona com o **Crítico** (muitos acertos) ou o **Apoio**.

## Necromante
**Invocador / Dano contínuo.** Lacaios autônomos e maldição em área.

- **Atributos:** 950 de vida · 24 de dano · 1,0 ataques/s · alcance 360 · Essência máxima de 120 · movimento 195.
- **Ataque rápido:** projétil (650 px/s).
- **Esqueletos:** invoca 1 esqueleto a cada **4 s**, até **3** ao mesmo tempo. Cada um causa 14 de dano, ataca o inimigo mais próximo em até 260 px e dura 20 s. Os acertos dos esqueletos contam como seus.
- **Maldição:** causa **8 de dano por segundo** a todos os inimigos num raio de 140 ao seu redor, no mesmo nível de altura.
- **Skill — Lança de Ossos:** lança que **atravessa** todos os inimigos em linha, alcance 500, ×2,6 de dano. Custa 30, recarga de 4 s.
- **Runas:** **Maestria Esquelética I** (rank 5: esqueletos com +15% de dano e limite de 3 → **4**) · Aura de Decadência II (rank 12, maldição ×1,3).
- **Como jogar:** fique logo atrás da linha de frente, para a maldição pegar quem chega perto, e alinhe a Lança de Ossos com as filas que vêm pelo caminho. Combina com o **Apoio** (aguenta à distância) ou o **Dano Mágico** (mais Lanças).

## Arcanista
**Ranged AoE.** Mana e magia elemental em área.

- **Atributos:** 850 de vida · 40 de dano · 0,9 ataques/s · alcance 380 · Mana máxima de **150** · **5% de redução de recarga** base.
- **Ataque rápido:** projétil que **explode** em área (raio 90).
- **Mana:** regenera 10 por segundo.
- **Skill — Pulso Arcano:** onda ao redor, raio **160**, ×2,0 de dano. Custa 35, recarga de 6 s.
- **Runas:** Fluxo Arcano I (rank 5, regeneração de recurso ×1,2) · Maestria Elemental I (rank 12, +10% de dano elemental).
- **Como jogar:** acerte grupos com a explosão do ataque rápido e guarde o Pulso Arcano para quando a horda alcançar você. Combina com o **Caminho do Dano Mágico** (amplificação elemental e recarga).

## Cavaleiro Sangrento
**Híbrido / Roubo de vida.** Médio alcance e sustentação.

- **Atributos:** 1150 de vida · 30 de dano · 1,4 ataques/s · alcance **160** (o maior entre os corpo a corpo) · armadura 20 · **12% de roubo de vida**.
- **Ataque rápido:** corpo a corpo em área, com arco longo.
- **Sangue:** começa em 0, **não regenera**, +5 por acerto e um pouco ao sofrer dano. A skill exige bater antes.
- **Skill — Ceifa Sangrenta:** corte em cone de **120°**, alcance 190, ×2,0 de dano; o roubo de vida também cura com ela. Custa 30, recarga de 6 s.
- **Runas:** Sede Carmesim I (rank 5, +4% de roubo de vida) · Pacto de Sangue I (rank 12, vida ×1,08).
- **Como jogar:** lute à meia distância, que o alcance do arco permite, e se cure acertando vários de uma vez. Combina com o **Crítico** (Sede Crítica cura ao critar) ou a **Defesa**.

## Mago *(novo)*
**Burst / Longo alcance.** Bolas de fogo e meteoros.

- **Atributos:** 820 de vida (a menor) · **42** de dano (o maior) · 0,85 ataques/s · alcance **560** · Mana máxima de **160** · crítico 7%.
- **Ataque rápido:** bola de fogo (700 px/s) que **explode** em área (raio 70).
- **Mana:** regenera **12 por segundo** (a maior).
- **Skill — Meteoro:** marca uma área no chão (raio 130, a até 600 px). Depois de **0,7 s**, um meteoro cai e causa **×4,0** de dano (o maior multiplicador do jogo). Custa 40, recarga de 8 s. Em terceira pessoa, cai onde a mira central aponta.
- **Runas:** Piromancia I (rank 5, raio da explosão ×1,25) · Alcance Arcano I (rank 12, +60 de alcance).
- **Como jogar:** fique longe, de preferência **em cima de um platô** (os monstros precisam dar a volta pela rampa), e mire o Meteoro onde a horda *vai estar*. É frágil: use o dash cedo. Combina com o **Dano Mágico**.

## Arqueiro *(novo)*
**Ranged DPS / Longo alcance.** O maior alcance do esquadrão.

- **Atributos:** 900 de vida · 30 de dano · 1,6 ataques/s · alcance **600** (o maior) · crítico **12%** (o maior) · movimento 215 · esquiva de 210 px com recarga de 0,9 s.
- **Ataque rápido:** flecha muito rápida (**1000 px/s**), difícil de errar.
- **Foco:** regenera 10 por segundo.
- **Skill — Flecha Perfurante:** flecha carregada (1300 px/s) que **atravessa a fileira inteira**, alcance **750**, ×3,0 de dano. Custa 25, recarga de 5 s.
- **Runas:** Olho de Águia I (rank 5, +5% de crítico) · Aljava Veloz I (rank 12, velocidade de ataque ×1,12).
- **Como jogar:** abata os **atiradores** de longe, antes que eles disparem, e use a Flecha Perfurante nas filas do caminho. Combina com o **Caminho do Crítico**.

---

## Ainda não implementado

Estes valores existem nos dados das classes, mas ainda não têm efeito no jogo:

- **Cruzado — aura** (`aura_radius`, `aura_armor_bonus`): a aura de armadura para aliados não está implementada. O suporte em área vem das habilidades do Caminho do Apoio.
- **Monge — `ally_shield`:** apesar do nome, hoje dá escudo só ao próprio Monge, no início de cada onda.
- **Arcanista — `slow_pct`:** lentidão nos inimigos ainda não existe; a simulação dos inimigos no servidor não tem efeitos de status.
- **Bárbaro — `cleave_radius`** e **Cavaleiro Sangrento — `drain_radius`:** sem uso. A área do ataque vem do arco à frente.
- **Necromante — `summon_health`:** os esqueletos não recebem dano (só expiram após 20 s).
- **Caçador de Demônios — `dodge_chance`:** funciona como redução de dano, não como chance de evitar o golpe inteiro.

---

## Visual e animações (mockups em `arts/`)

O personagem no jogo usa a arte do próprio mockup da classe (sprites extraídos por `tools/art/extract_sprites.py`). A ficha original do mockup fica só como blueprint de arte (`tools/art/blueprints/`); no jogo, **Classes e Ranks → Ficha do Herói** mostra o modelo 3D da classe.

| Classe | Poses extraídas do mockup | Geradas por animação procedural |
|---|---|---|
| Bárbaro | parado, ataque, Redemoinho, esquiva, pulo | — |
| Cruzado | parado, ataque, Investida Sagrada, esquiva, pulo | — |
| Monge | parado, ataque, Onda de Luz, esquiva, pulo | — |
| Caçador de Demônios | parado, ataque, Rajada, esquiva, pulo | — |
| Necromante | parado, ataque, Lança de Ossos, invocação | esquiva (rastro), pulo |
| Arcanista | parado, ataque, Pulso Arcano | esquiva (rastro), pulo |
| Cavaleiro Sangrento | parado, ataque, Ceifa Sangrenta | esquiva (rastro), pulo |
| Mago | parado, ataque, Meteoro | esquiva (rastro), pulo |
| Arqueiro | parado, ataque, Flecha Perfurante | esquiva (rastro), pulo |

Todas as classes têm, por cima das poses, animação procedural:
- respiração e passo;
- avanço no golpe e pulso na skill;
- rastro no dash e flash ao levar dano;
- personagem deitado quando cai.

**Inimigos (também dos mockups):** goblin (soldado), demônio vermelho (corredor), esqueleto-zumbi (atirador) e esqueleto (lacaio do Necromante). O **brutamontes** é gerado a partir do goblin, maior e mais escuro.
