# 3ds Max — Guia de Exportação Final de Assets (UE5)

Esta pasta guarda **exclusivamente** os arquivos `.FBX`, malhas e texturas validados e prontos para importação direta na **Unreal Engine 5**.
Arquivos brutos de modelagem (`.max`, `.blend`, `.skp`, concepts) devem permanecer em `/arts` — **não** mova arquivos de rascunho para cá.

---

## 📂 Mapa de Pastas: Aonde vai cada Export FBX

Para manter o projeto organizado e compatível com a Unreal Engine 5, cada tipo de modelo exportado deve ser salvo em sua subpasta correspondente:

### 1. `Characters/` (Personagens, Monstros e Animações)
| Subpasta | O que exportar aqui | Exemplo de Nome FBX |
| :--- | :--- | :--- |
| `Characters/Heroes/` | Malhas com rig dos heróis (Skeletal Mesh) e variações | `SK_Hero_Male.fbx`, `SK_Hero_Female.fbx` |
| `Characters/Monsters/` | Mobs comuns das dungeons com rig e animações | `SK_Goblin_Grunt.fbx`, `SK_Bat_Cave.fbx` |
| `Characters/Bosses/` | Chefes de bioma e do Ato I com rig | `SK_Boss_GoblinKing.fbx`, `SK_Boss_Arcodragon.fbx` |
| `Characters/<Nome>/Animations/` | Clipes de animação puros (sem malha ou com rig) | `A_Hero_Run.fbx`, `A_Hero_Slash_01.fbx` |

---

### 2. `Environments/` (Kits Modulares de Terreno e Cenários)
| Subpasta | O que exportar aqui | Exemplo de Nome FBX |
| :--- | :--- | :--- |
| `Environments/Hub_Cidadela/` | Terrenos diamantados, praça central, pontes flutuantes | `SM_Hub_Island_Base_Diamond_01.fbx`, `SM_Hub_Great_Portal_Arch_01.fbx` |
| `Environments/Dungeon_01_Floresta/` | Pisos de terra, barrancos, rochas, pontes de madeira | `SM_Forest_Floor_Dirt_01.fbx`, `SM_Forest_Cliff_Wall_01.fbx` |
| `Environments/Dungeon_02_Gruta/` | Paredes de calcário, leito de rio, estalactites, arcos | `SM_Cave_Floor_WetRock_01.fbx`, `SM_Cave_Stalactite_Cluster_01.fbx` |
| `Environments/Dungeon_03_Cidadela/` | Lajes imperiais, colunas de mármore, arcos de pedra | `SM_Citadel_Floor_Tile_01.fbx`, `SM_Citadel_Pillar_Intact_01.fbx` |
| `Environments/Dungeon_04_Pantano/` | Lodo, mangues, passarelas de tábuas podres, charcos | `SM_Swamp_Floor_Mud_01.fbx`, `SM_Swamp_Tree_Mangrove_Large_01.fbx` |
| `Environments/Dungeon_05_Cupula/` | Mármore branco celestial, ilhas de topo, arcos de ouro | `SM_Sky_Floor_WhiteMarble_01.fbx`, `SM_Sky_Pillar_Celestial_01.fbx` |

---

### 3. `Items/` (Equipamentos, Armas e Coletáveis)
| Subpasta | O que exportar aqui | Exemplo de Nome FBX |
| :--- | :--- | :--- |
| `Items/Weapons/` | Espadas, adagas, maças, machados, lanças, bestas | `SM_Weapon_Sword_Bronze_01.fbx`, `SM_Weapon_Mace_Iron_01.fbx` |
| `Items/Armors/` | Peças modulares de armadura (elmos, peitorais, botas) | `SM_Armor_Chest_Bronze_01.fbx`, `SM_Armor_Helmet_Iron_01.fbx` |
| `Items/Resources/` | Minérios dropados no chão, barras fundidas, ervas | `SM_Ore_Drop_Copper_01.fbx`, `SM_Bar_Ingot_Bronze_01.fbx` |

---

### 4. `Props/` (Objetos Interativos, Estações e Decoração)
| Subpasta | O que exportar aqui | Exemplo de Nome FBX |
| :--- | :--- | :--- |
| `Props/Interactive/` | Baús, alavancas, portões levadiços, armadilhas, altares | `SM_Forest_Chest_Wooden_01.fbx`, `SM_Citadel_Pressure_Plate_01.fbx` |
| `Props/Crafting/` | Bigornas, fornalhas, mesas de alquimia e marcenaria | `SM_Forge_Stone_Furnace_01.fbx`, `SM_Anvil_Reinforced_01.fbx` |
| `Props/Decoration/` | Caixotes, tochas, estandartes, estátuas, móveis | `SM_Citadel_Statue_Emperor_01.fbx`, `SM_Hub_Mailbox_Post_01.fbx` |

---

## ⚙️ Regras Técnicas Obrigatórias de Exportação do 3ds Max

1. **Unidades do Sistema:**
   * 1 Unidade = 1 Centímetro (padrão nativo da Unreal Engine).
   * Na janela de exportação FBX do 3ds Max: desligue *Automatic* e fixe *Scene units converted to: Centimeters*.
2. **Eixos:**
   * **Z para cima (Z-Up)**. Frente do personagem ou objeto olhando para **+X** na Unreal (no 3ds Max isso equivale à frente virada para −Y antes da exportação).
3. **Pivô / Ponto de Origem:**
   * **Personagens:** O pivô central deve estar **entre os pés no chão (Z = 0)** exatamente na origem `(0, 0, 0)`.
   * **Armas:** O pivô deve estar **na empunhadura** (onde a mão do herói segura).
   * **Módulos de Parede/Piso:** O pivô deve estar em um dos cantos inferiores ou na base centralizada para encaixar no grid modular da UE5 (grid de 100cm / 1m).
4. **Limpeza da Malha:**
   * Aplicar **Reset XForm** antes de exportar para zerar rotações e escalas indesejadas (`100%`).
   * Converter sempre para *Editable Poly*.
