# Paleta Padrão de Cores Reais: Personagens Chibi (`0_MALE` & `1_FEMALE`)

**Imagem de Referência Visual:** [`set_cores_padrao_personagens.jpg`](./set_cores_padrao_personagens.jpg)  
**Blueprints Base:** [`blueprints/masculino_front.jpg`](./blueprints/masculino_front.jpg), [`blueprints/masculino_left.jpg`](./blueprints/masculino_left.jpg), [`blueprints/feminino_front.jpg`](./blueprints/feminino_front.jpg), [`blueprints/feminino_left.jpg`](./blueprints/feminino_left.jpg)

---

## 1. Configuração Padrão Inicial (Default ao Abrir o Criador de Personagem)

| Componente | Cor Padrão Inicial | Código HEX Principal | Arquivos `.uvw` Afetados |
| :--- | :--- | :---: | :--- |
| **Pele Base** | **Pele Clara Quente (Pêssego)** | `#FFDBB5` | `Head`, `Torso`, `Pelvis`, `Arm_L/R`, `Leg_L/R`, `Foot_L/R` |
| **Bochechas (Blush)** | **Rosado Suave** | `#F09B8C` | `Cheek_Blush_M.uvw` / `Cheek_Blush_F.uvw` |
| **Cabelo** | **Castanho (Marrom Natural)** | `#6B4435` | `Hair_01_M.uvw` / `Hair_01_F.uvw` |
| **Sobrancelhas e Cílios** | **Castanho Escuro (Raiz)** | `#4A3028` | `Eyebrows_M/F.uvw`, `Eyelashes_M/F.uvw` |
| **Olhos (Íris)** | **Castanho Claro (Mel / Âmbar)** | `#9E6336` | `Iris_M/F.uvw`, `Iris_Center_M/F.uvw`, `Iris_Highlight_M/F.uvw` |

---

## 2. Cores de Cabelo, Sobrancelhas e Cílios (5 Cores Reais Básicas)
> No Material Dinâmico da Unreal Engine 5 (`M_Character_Hair_Master`), as sobrancelhas (`Eyebrows_M/F`) e os cílios (`Eyelashes_M/F`) herdam automaticamente o parâmetro `RootColor` do cabelo (podendo também ser alterados separadamente pelo jogador se desejar).

| ID | Nome da Cor | `RootColor` (Raiz / Sobrancelha / Cílios) | `BaseColor` (Cor Principal) | `TipHighlightColor` (Pontas / Brilho) |
| :--- | :--- | :---: | :---: | :---: |
| **01 (DEFAULT)** | **Castanho (Marrom)** | `#4A3028` | `#6B4435` | `#8B5A42` |
| **02** | **Preto (Ébano)** | `#141416` | `#1F1F24` | `#383842` |
| **03** | **Branco Acinzentado (Platinado/Grisalho)** | `#7A7E85` | `#C4C9D1` | `#E8ECF2` |
| **04** | **Ruivo (Acobreado Natural)** | `#6B2214` | `#A83E24` | `#D4623B` |
| **05** | **Loiro (Dourado Natural)** | `#8C6B2B` | `#D9B45B` | `#F2D680` |

---

## 3. Cores de Olhos / Íris (5 Cores Reais Mais Comuns)
> Aplicadas às malhas `Iris_M/F.uvw`, `Iris_Center_M/F.uvw` e `Iris_Highlight_M/F.uvw` (enquanto `White_eye_F` / `Iris_Back_M/F.uvw` mantém o branco levemente quente `#F7F4F0`).

| ID | Nome da Cor dos Olhos | `IrisBaseColor` (`Iris_M/F`) | `PupilCenterColor` (`Iris_Center_M/F`) | `IrisRingGlow` (`Iris_Highlight_M/F`) |
| :--- | :--- | :---: | :---: | :---: |
| **01 (DEFAULT)** | **Castanho Claro (Mel / Âmbar)** | `#9E6336` | `#4D2C16` | `#D4955A` |
| **02** | **Castanho Escuro (Café)** | `#4A2C18` | `#1F1108` | `#75492B` |
| **03** | **Azul Claro (Safira)** | `#3B7EA1` | `#1B4359` | `#78BBE6` |
| **04** | **Verde (Esmeralda / Avelã)** | `#4B8B5A` | `#23472B` | `#85C792` |
| **05** | **Cinza (Prateado Acinzentado)** | `#7E8C94` | `#424B52` | `#B8C7D1` |

---

## 4. Tons de Pele da Vida Real (5 Tons Reais)
> Aplicados de forma unificada em todas as partes do corpo (`Head`, `Torso`, `Pelvis`, `Arm_L`, `Arm_R`, `Leg_L`, `Leg_R`, `Foot_L`, `Foot_R`) e integrados com a malha de blush (`Cheek_Blush`) e pálpebras (`Eyelid`).

| ID | Tom de Pele Real | `SkinBaseColor` | `SkinSubsurfaceShadow` (Pálpebra/Sombra) | `BlushColor` (`Cheek_Blush`) |
| :--- | :--- | :---: | :---: | :---: |
| **01 (DEFAULT)** | **Pele Clara Rosada / Pêssego** | `#FFDBB5` | `#E5B58D` | `#F09B8C` |
| **02** | **Pele Muito Clara / Marfim** | `#FFE9D6` | `#EBC6AC` | `#F5A89E` |
| **03** | **Pele Trigo / Morena Clara** | `#E8B88A` | `#C99364` | `#D97F6C` |
| **04** | **Pele Morena Média / Oliva** | `#C68B59` | `#A36A3B` | `#B85D4B` |
| **05** | **Pele Negra / Marrom Quente** | `#8D5524` | `#6B3C14` | `#853B2B` |

---

## 4. Elementos Anatômicos com Cor Fixa (Não-Personalizáveis pelo Jogador)
*(Mantêm sempre a mesma cor padrão realista independente da customização de pele, cabelo ou íris)*

| Elemento Facial / Corporal | Arquivos UVW / PNG Referência (EN) | Cor Base Fixa | Sombra / Degradê (`Canal R` da Máscara) | Observação Técnica |
| :--- | :--- | :---: | :---: | :--- |
| **Fundo Branco do Olho (Esclera)** | `White_eye_F` / `Iris_Back_F` | `#F7F4F0` | `#D1C7BE` | Sombra suave na parte superior abaixo da pálpebra (`White_eye_F_Mask.png`). |
| **Brilho da Íris (Highlight)** | `Iris_Highlight_M` / `Iris_Highlight_F` | `#FFFFFF` | — | Branco puro emissivo (`1.0`), não altera cor. |
| **Arcada Dentária (Dentes)** | `Teeth_M` / `Teeth_F` | `#FAF6F0` | `#D8CFC4` | Marfim dental natural com leve oclusão nos cantos. |
| **Língua (Vermelho Sólido)** | `Tongue_M` / `Tongue_F` | `#D96B6B` (`R=220`) | — | Máscara vermelha sólida completa (`Tongue_F_Mask.png`). |
| **Fundo da Boca (Vermelho Escuro Sólido)** | `Mouth_M` / `Mouth_F` | `#8B2626` (`R=130`) | — | Máscara vermelha escura sólida completa (`Mouth_F_Mask.png`). |
| **Roupa Íntima Base (Top / Shorts / Cueca)** | `Torso_F`, `Pelvis_F`, `Pelvis_M` | `#3A3A3A` | — | Isolada com 100% de pureza no **`Canal B` (Azul `R=0, G=0, B=255`)**. |
