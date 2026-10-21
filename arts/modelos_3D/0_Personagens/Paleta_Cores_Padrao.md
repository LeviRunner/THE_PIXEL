# Paleta Padrão de Cores Reais: Personagens Chibi (`0_MALE` & `1_FEMALE`)

**Imagem de Referência Visual:** [`set_cores_padrao_personagens.jpg`](./set_cores_padrao_personagens.jpg)  
**Blueprints Base:** [`blueprints/masculino_front.jpg`](./blueprints/masculino_front.jpg), [`blueprints/masculino_left.jpg`](./blueprints/masculino_left.jpg), [`blueprints/feminino_front.jpg`](./blueprints/feminino_front.jpg), [`blueprints/feminino_left.jpg`](./blueprints/feminino_left.jpg)

---

## 1. Configuração Padrão Inicial (Default ao Abrir o Criador de Personagem)

| Componente | Cor Padrão Inicial | Código HEX Principal | Arquivos `.uvw` Afetados |
| :--- | :--- | :---: | :--- |
| **Pele Base** | **Pele Clara Quente (Pêssego)** | `#FFDBB5` | `Head`, `Torso`, `Pelvis`, `Arm_L/R`, `Leg_L/R`, `Foot_L/R` |
| **Bochechas (Blush)** | **Rosado Suave** | `#F09B8C` | `Fundo_bochechas_M.uvw` / `Fundo_bochechas_F.uvw` |
| **Cabelo** | **Castanho (Marrom Natural)** | `#6B4435` | `Hair_01_M.uvw` / `Hair_01_F.uvw` |
| **Sobrancelhas e Cílios** | **Castanho Escuro (Raiz)** | `#4A3028` | `Sobrancelhas_M/F.uvw`, `Cilios_M/F.uvw` |
| **Olhos (Íris)** | **Castanho Claro (Mel / Âmbar)** | `#9E6336` | `Iris_M/F.uvw`, `Centro_iris_M/F.uvw`, `Brilho_iris_M/F.uvw` |

---

## 2. Cores de Cabelo, Sobrancelhas e Cílios (5 Cores Reais Básicas)
> No Material Dinâmico da Unreal Engine 5 (`M_Character_Hair_Master`), as sobrancelhas (`Sobrancelhas_M/F`) e os cílios (`Cilios_M/F`) herdam automaticamente o parâmetro `RootColor` do cabelo (podendo também ser alterados separadamente pelo jogador se desejar).

| ID | Nome da Cor | `RootColor` (Raiz / Sobrancelha / Cílios) | `BaseColor` (Cor Principal) | `TipHighlightColor` (Pontas / Brilho) |
| :--- | :--- | :---: | :---: | :---: |
| **01 (DEFAULT)** | **Castanho (Marrom)** | `#4A3028` | `#6B4435` | `#8B5A42` |
| **02** | **Preto (Ébano)** | `#141416` | `#1F1F24` | `#383842` |
| **03** | **Branco Acinzentado (Platinado/Grisalho)** | `#7A7E85` | `#C4C9D1` | `#E8ECF2` |
| **04** | **Ruivo (Acobreado Natural)** | `#6B2214` | `#A83E24` | `#D4623B` |
| **05** | **Loiro (Dourado Natural)** | `#8C6B2B` | `#D9B45B` | `#F2D680` |

---

## 3. Cores de Olhos / Íris (5 Cores Reais Mais Comuns)
> Aplicadas às malhas `Iris_M/F.uvw`, `Centro_iris_M/F.uvw` e `Brilho_iris_M/F.uvw` (enquanto `Fundo_iris_M/F.uvw` mantém o branco levemente quente `#F7F4F0`).

| ID | Nome da Cor dos Olhos | `IrisBaseColor` (`Iris_M/F`) | `PupilCenterColor` (`Centro_iris_M/F`) | `IrisRingGlow` (`Brilho_iris_M/F`) |
| :--- | :--- | :---: | :---: | :---: |
| **01 (DEFAULT)** | **Castanho Claro (Mel / Âmbar)** | `#9E6336` | `#4D2C16` | `#D4955A` |
| **02** | **Castanho Escuro (Café)** | `#4A2C18` | `#1F1108` | `#75492B` |
| **03** | **Azul Claro (Safira)** | `#3B7EA1` | `#1B4359` | `#78BBE6` |
| **04** | **Verde (Esmeralda / Avelã)** | `#4B8B5A` | `#23472B` | `#85C792` |
| **05** | **Cinza (Prateado Acinzentado)** | `#7E8C94` | `#424B52` | `#B8C7D1` |

---

## 4. Tons de Pele da Vida Real (5 Tons Reais)
> Aplicados de forma unificada em todas as partes do corpo (`Head`, `Torso`, `Pelvis`, `Arm_L`, `Arm_R`, `Leg_L`, `Leg_R`, `Foot_L`, `Foot_R`) e integrados com a malha de blush (`Fundo_bochechas`) e pálpebras (`Palpebra`).

| ID | Tom de Pele Real | `SkinBaseColor` | `SkinSubsurfaceShadow` (Pálpebra/Sombra) | `BlushColor` (`Fundo_bochechas`) |
| :--- | :--- | :---: | :---: | :---: |
| **01 (DEFAULT)** | **Pele Clara Rosada / Pêssego** | `#FFDBB5` | `#E5B58D` | `#F09B8C` |
| **02** | **Pele Muito Clara / Marfim** | `#FFE9D6` | `#EBC6AC` | `#F5A89E` |
| **03** | **Pele Trigo / Morena Clara** | `#E8B88A` | `#C99364` | `#D97F6C` |
| **04** | **Pele Morena Média / Oliva** | `#C68B59` | `#A36A3B` | `#B85D4B` |
| **05** | **Pele Negra / Marrom Quente** | `#8D5524` | `#6B3C14` | `#853B2B` |
