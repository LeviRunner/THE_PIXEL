# Checklist de Animações: Personagem Masculino (`Hero_Chibi_Male`)

> **Esqueleto:** `Hero_Chibi_Male_RIG_Skeleton` (AccuRIG)  
> **Estilo de Gameplay:** Dungeon Crawler Soulslike (Combate tático, sem cambalhotas, esquiva por *Quickstep/Dash*, bloqueio ativo e *Parry*).

---

## 0. Checklist de Rigging e Preparação (Corpo, Olhos/Rosto e Cabelo)
> Antes de exportar os sets autorais de animação, cada grupo anatômico precisa estar devidamente rigado (Skin / Ossos / Morph Targets):

| Grupo / Elemento | Tipo de Rigging | 4. Rig Concluído | 5. Animação Testada |
| :--- | :--- | :---: | :---: |
| **Corpo Base Completo (`Head_M`, `Torso_M`, `Pelvis_M`, Membros)** | AccuRIG (Esqueleto Base + Pesos de Skin) | [ ] | [ ] |
| **Olhos Completos (`Iris_M`, `Centro_iris_M`, `Brilho_iris_M`, `Fundo_iris_M`)** | Eye Bones (`eye_l` / `eye_r`) ou UV Offset / Aim Constraint | [ ] | [ ] |
| **Pálpebras e Cílios (`Palpebra_M`, `Cilios_M`)** | Morph Target (`Blink` / Piscar) ou Osso de Pálpebra | [ ] | [ ] |
| **Sobrancelhas (`Sobrancelhas_M`)** | Morph Targets (`Raiva`, `Surpresa`, `Foco`) ou Ossos `brow_l/r` | [ ] | [ ] |
| **Fundo de Bochechas (`Fundo_bochechas_M`)** | Skin no osso `head` + Morph de Sorriso/Expressão | [ ] | [ ] |
| **Boca Interna (`Dentes_M`, `Lingua_M`)** | Osso da Mandíbula (`jaw`) + Morph de Abertura Vocal | [ ] | [ ] |
| **Cabelo Modular (`Hair_01_M`, `Hair_Front_01_M`)** | Osso `head` + Ossos secundários de ponta/física + Morphs de Tamanho | [ ] | [ ] |

---

## 1. Locomoção e Movimentação Básica
| ID da Animação | Nome do Arquivo FBX | Descrição / Regra Chibi | Status |
| :--- | :--- | :--- | :---: |
| **Idle Padrão** | `Anim_Male_Idle.fbx` | Respiração suave, joelhos levemente flexionados, postura alerta (Loop 60f) | [ ] |
| **Caminhada (Walk)** | `Anim_Male_Walk_Fwd.fbx` | Passos firmes e ritmados pelo hub/dungeon (Loop 30f) | [ ] |
| **Corrida (Run)** | `Anim_Male_Run_Fwd.fbx` | Corrida ágil com tronco levemente inclinado à frente (Loop 24f) | [ ] |
| **Início do Pulo** | `Anim_Male_Jump_Start.fbx` | Impulso rápido saindo do chão (8f) | [ ] |
| **Flutuação no Ar** | `Anim_Male_Jump_Loop.fbx` | Pose suspensa no ápice do salto (Loop 12f) | [ ] |
| **Aterrissagem** | `Anim_Male_Jump_Land.fbx` | Amortecimento curto nos joelhos ao tocar o solo (10f) | [ ] |

---

## 2. Esquiva e Defesa Soulslike (Sem Cambalhota)
> **Regra de Design:** Por ser um personagem Chibi cabeçudo, **não utilizamos cambalhota (roll)** para evitar que a cabeça atravesse o chão. Toda esquiva é um **Dash / Quickstep deslizante**.

| ID da Animação | Nome do Arquivo FBX | Descrição / Mecânica | Status |
| :--- | :--- | :--- | :---: |
| **Esquiva Frontal (Dash)** | `Anim_Male_Dodge_Fwd.fbx` | *Quickstep* rápido para frente com rastro de vento (18f) | [ ] |
| **Esquiva Traseira (Backstep)**| `Anim_Male_Dodge_Bwd.fbx` | Salto curto de recuo tático saindo do alcance do golpe (18f) | [ ] |
| **Esquiva Lateral (Esq/Dir)** | `Anim_Male_Dodge_Side.fbx` | Deslize lateral rápido mantendo o olhar no alvo (16f) | [ ] |
| **Entrar em Guarda (Block)** | `Anim_Male_Guard_Start.fbx` | Levanta o escudo ou a arma transversalmente à frente do corpo (6f) | [ ] |
| **Manter Guarda (Loop)** | `Anim_Male_Guard_Loop.fbx` | Postura defensiva contínua enquanto segura o botão de defesa (30f) | [ ] |
| **Impacto no Bloqueio** | `Anim_Male_Guard_Impact.fbx` | Empurrão para trás com faísca ao absorver golpe pesado (12f) | [ ] |
| **Parry (Deflexão Real)** | `Anim_Male_Parry_Deflect.fbx` | Golpe seco com o escudo/arma (frames 4 a 6 ativos) que desequilibra o inimigo | [ ] |
| **Reação de Dano (Hit)** | `Anim_Male_Hit_React.fbx` | Recuo curto ao sofrer dano direto (12f) | [ ] |
| **Morte / Derrota** | `Anim_Male_Death.fbx` | Queda dramática estilizada de joelhos e depois ao chão (35f) | [ ] |

---

## 3. Combate Corpo a Corpo (Por Tipo de Arma)
> Animado usando um cilindro/bastão proxy no `Socket_Weapon_R`. Reaproveitado para todos os Tiers (Bronze ao Ouro).

| Arma | Ataque Básico 1 | Ataque Básico 2 | Ataque Especial (Habilidade) | Status |
| :--- | :--- | :--- | :--- | :---: |
| **Espada (Sword)** | `Anim_Atk_Sword_Slash_01` | `Anim_Atk_Sword_Slash_02` | `Anim_Atk_Sword_Special_Thrust` (Estocada) | [ ] |
| **Adaga (Dagger)** | `Anim_Atk_Dagger_Slash_01` | `Anim_Atk_Dagger_Slash_02` | `Anim_Atk_Dagger_Special_Backstab` (Corte X) | [ ] |
| **Maça (Mace)** | `Anim_Atk_Mace_Smash_01` | `Anim_Atk_Mace_Smash_02` | `Anim_Atk_Mace_Special_GroundSlam` | [ ] |
| **Machado (Battleaxe)** | `Anim_Atk_Battleaxe_Chop_01`| `Anim_Atk_Battleaxe_Chop_02`| `Anim_Atk_Battleaxe_Special_Whirlwind` (Giro) | [ ] |
| **Lança (Spear)** | `Anim_Atk_Spear_Thrust_01` | `Anim_Atk_Spear_Sweep_02` | `Anim_Atk_Spear_Special_Impale` | [ ] |
| **Alabarda (Halberd)** | `Anim_Atk_Halberd_Sweep_01`| `Anim_Atk_Halberd_Chop_02` | `Anim_Atk_Halberd_Special_Crush` | [ ] |

---

## 4. Combate à Distância e Magia
| Categoria | Ação Principal | Ação Secundária / Recarga | Ataque Especial | Status |
| :--- | :--- | :--- | :--- | :---: |
| **Besta de Mão (1H/2H)**| `Anim_Ranged_Crossbow_Shoot`| `Anim_Ranged_Crossbow_Reload`| `Anim_Ranged_Crossbow_Volley` (Rajada) | [ ] |
| **Arco Recurvo (Bow)** | `Anim_Ranged_Bow_DrawHold` | `Anim_Ranged_Bow_Release` | `Anim_Ranged_Bow_PowerShot` | [ ] |
| **Arremesso (Facas/Dardos)**| `Anim_Ranged_Throw_Quick` | `Anim_Ranged_Throw_Double` | `Anim_Ranged_Throw_Fan` (Leque) | [ ] |
| **Varinha & Orbe (Wand)**| `Anim_Magic_Wand_CastBolt` | `Anim_Magic_Orb_Channel` | `Anim_Magic_Wand_ArcaneNova` | [ ] |
| **Cajado (Battle Staff)**| `Anim_Magic_Staff_SwingCast`| `Anim_Magic_Staff_SlamAoE` | `Anim_Magic_Staff_BeamChannel` (Raio) | [ ] |

---

## 5. Coleta, Crafting & Interação (Mini Mundo / Dungeon)
| Ação | Nome do Arquivo FBX | Uso no Jogo | Status |
| :--- | :--- | :--- | :---: |
| **Minerar (Picareta)** | `Anim_Work_Mine_Pickaxe.fbx` | Extrair Cobre, Ferro, Prata, Aço e Ouro nas rochas | [ ] |
| **Forjar (Martelo)** | `Anim_Work_Forge_Hammer.fbx` | Bater o metal quente na bigorna da oficina | [ ] |
| **Coletar Planta/Erva** | `Anim_Work_Gather_Herb.fbx` | Agachar rápido para colher plantas alquímicas | [ ] |
| **Interagir / Alavanca**| `Anim_Interact_PullLever.fbx` | Puxar alavancas de portões e ativar o Altar de TP | [ ] |
