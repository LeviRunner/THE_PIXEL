# Checklist de Animações: Personagem Feminino (`Hero_Chibi_Female`)

> **Esqueleto:** `Hero_Chibi_Female_RIG_Skeleton` (AccuRIG — compatível via Retargeting Direto / Shared Skeleton com `Hero_Chibi_Male`)  
> **Estilo de Gameplay:** Dungeon Crawler Soulslike (Combate tático, sem cambalhotas, esquiva por *Quickstep/Dash*, bloqueio ativo e *Parry*).

---

## 0. Checklist de Rigging e Preparação (Corpo, Olhos/Rosto e Cabelo)
> Antes de exportar os sets autorais de animação, cada grupo anatômico precisa estar devidamente rigado (Skin / Ossos / Morph Targets):

| Grupo / Elemento | Tipo de Rigging | 4. Rig Concluído | 5. Animação Testada |
| :--- | :--- | :---: | :---: |
| **Corpo Base Completo (`Head_F`, `Torso_F`, `Pelvis_F`, Membros)** | AccuRIG (Esqueleto Base + Pesos de Skin) | [ ] | [ ] |
| **Olhos Completos (`Iris_F`, `Centro_iris_F`, `Brilho_iris_F`, `Fundo_iris_F`)** | Eye Bones (`eye_l` / `eye_r`) ou UV Offset / Aim Constraint | [ ] | [ ] |
| **Pálpebras e Cílios (`Palpebra_F`, `Cilios_F`)** | Morph Target (`Blink` / Piscar) ou Osso de Pálpebra | [ ] | [ ] |
| **Sobrancelhas (`Sobrancelhas_F`)** | Morph Targets (`Raiva`, `Surpresa`, `Foco`) ou Ossos `brow_l/r` | [ ] | [ ] |
| **Fundo de Bochechas (`Fundo_bochechas_F`)** | Skin no osso `head` + Morph de Sorriso/Expressão | [ ] | [ ] |
| **Boca Interna (`Dentes_F`, `Lingua_F`)** | Osso da Mandíbula (`jaw`) + Morph de Abertura Vocal | [ ] | [ ] |
| **Cabelo Modular (`Hair_01_F`, `Hair_Front_01_F`, Extensões)** | Osso `head` + Ossos dinâmicos de extensão + Morphs de Tamanho | [ ] | [ ] |

---

## 1. Locomoção e Movimentação Básica
| ID da Animação | Nome do Arquivo FBX | Descrição / Regra Chibi | Status |
| :--- | :--- | :--- | :---: |
| **Idle Padrão** | `Anim_Female_Idle.fbx` | Respiração suave, postura ágil e equilibrada (Loop 60f) | [ ] |
| **Caminhada (Walk)** | `Anim_Female_Walk_Fwd.fbx` | Passos leves e rítmicos pelo hub/dungeon (Loop 30f) | [ ] |
| **Corrida (Run)** | `Anim_Female_Run_Fwd.fbx` | Corrida fluida com tronco inclinado à frente (Loop 24f) | [ ] |
| **Início do Pulo** | `Anim_Female_Jump_Start.fbx` | Impulso ágil saindo do solo (8f) | [ ] |
| **Flutuação no Ar** | `Anim_Female_Jump_Loop.fbx` | Pose suspensa no ápice do salto (Loop 12f) | [ ] |
| **Aterrissagem** | `Anim_Female_Jump_Land.fbx` | Amortecimento suave nos joelhos ao tocar o chão (10f) | [ ] |

---

## 2. Esquiva e Defesa Soulslike (Sem Cambalhota)
> **Regra de Design:** Sem cambalhotas (evita clipping de cabeça e saias/túnicas). A esquiva é um **Combat Dash / Quickstep** veloz rente ao chão.

| ID da Animação | Nome do Arquivo FBX | Descrição / Mecânica | Status |
| :--- | :--- | :--- | :---: |
| **Esquiva Frontal (Dash)** | `Anim_Female_Dodge_Fwd.fbx` | *Quickstep* rápido para frente com impulso de esquiva (18f) | [ ] |
| **Esquiva Traseira (Backstep)**| `Anim_Female_Dodge_Bwd.fbx` | Recuo tático rápido saindo da zona de impacto (18f) | [ ] |
| **Esquiva Lateral (Esq/Dir)** | `Anim_Female_Dodge_Side.fbx` | Deslize lateral veloz mantendo foco no inimigo (16f) | [ ] |
| **Entrar em Guarda (Block)** | `Anim_Female_Guard_Start.fbx` | Ergue o escudo ou arma em postura defensiva (6f) | [ ] |
| **Manter Guarda (Loop)** | `Anim_Female_Guard_Loop.fbx` | Postura firme de bloqueio contínuo (30f) | [ ] |
| **Impacto no Bloqueio** | `Anim_Female_Guard_Impact.fbx`| Absorção de impacto pesado com deslize nos pés (12f) | [ ] |
| **Parry (Deflexão Real)** | `Anim_Female_Parry_Deflect.fbx`| Movimento rápido de deflexão (frames 4-6 ativos) abrindo a guarda inimiga | [ ] |
| **Reação de Dano (Hit)** | `Anim_Female_Hit_React.fbx` | Reação rápida de impacto sofrido (12f) | [ ] |
| **Morte / Derrota** | `Anim_Female_Death.fbx` | Queda estilizada ao perder todo o HP (35f) | [ ] |

---

## 3. Combate Corpo a Corpo (Compartilhado por Socket `Socket_Weapon_R`)
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
| **Minerar (Picareta)** | `Anim_Work_Mine_Pickaxe.fbx` | Extrair minérios dos Tiers 1 ao 5 | [ ] |
| **Forjar (Martelo)** | `Anim_Work_Forge_Hammer.fbx` | Trabalhar equipamentos na bigorna | [ ] |
| **Coletar Planta/Erva** | `Anim_Work_Gather_Herb.fbx` | Colher ervas e flores alquímicas | [ ] |
| **Interagir / Alavanca**| `Anim_Interact_PullLever.fbx` | Acionar mecanismos, portas e Altar de TP | [ ] |
