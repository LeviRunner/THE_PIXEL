# Checklist de Produção: Base Masculina (`Hero_Chibi_Male`)

**Contexto:** O modelo base (`Hero_Chibi_Male.max` / `Hero_Chibi_Male_RIG.fbx`) é a estrutura anatômica Chibi Cartoon sobre a qual todas as armaduras, roupas e acessórios são acoplados em tempo real (sistema *Paperdoll* / *Leader Pose*).

---

## 1. Visão Geral das 6 Bases de Produção por Elemento
Cada componente anatômico do personagem passa por **6 pilares de produção**:
1. **Malha (Mesh 3D):** Modelagem low-poly estilizada com topologia limpa para deformação.
2. **UVMap (`.uvw`):** Mapeamento de coordenadas UV aberto e exportado do 3ds Max.
3. **Textura (Material/Texture):** Pintura base, máscara de cor parametrizável ou shader PBR estilizado.
4. **Rig (Skin / Ossos / Morphs):** Pesos de Skin no esqueleto AccuRIG + Rigging/Morph Targets de Olhos, Pálpebras, Sobrancelhas, Bochechas e Cabelo.
5. **Animações (Sets Autorais):** Ciclos de movimento, expressões faciais/piscar, física de cabelo e ataques.
6. **Efeitos (VFX/Shader):** Brilho ocular, suor/poeira, flash de dano (Hit Flash) ou emissão mágica.

---

## 2. Checklist de Produção: Corpo Base (Pele)
> Baseado nos arquivos `.uvw` já finalizados na pasta `0_MALE`.

| Parte do Corpo | Arquivo UVW Referência | 1. Malha | 2. UVMap | 3. Textura | 4. Rig (Skin) | 5. Animações | 6. Efeitos (Shader/VFX) |
| :--- | :--- | :---: | :---: | :---: | :---: | :---: | :---: |
| **Cabeça (Crânio/Rosto)** | `Head_M.uvw` | [x] | [x] | [x] | [x] | [ ] | [ ] |
| **Torso / Peito Base** | `Torso_M.uvw` | [x] | [x] | [x] | [x] | [ ] | [ ] |
| **Pelve / Quadril Base** | `Pelvis_M.uvw` | [x] | [x] | [x] | [x] | [ ] | [ ] |
| **Braço Esquerdo** | `Arm_L_M.uvw` | [x] | [x] | [x] | [x] | [ ] | [ ] |
| **Braço Direito** | `Arm_R_M.uvw` | [x] | [x] | [x] | [x] | [ ] | [ ] |
| **Perna Esquerda** | `Leg_L_M.uvw` | [x] | [x] | [x] | [x] | [ ] | [ ] |
| **Perna Direita** | `Leg_R_M.uvw` | [x] | [x] | [x] | [x] | [ ] | [ ] |
| **Pé Esquerdo** | `Foot_L_M.uvw` | [x] | [x] | [x] | [x] | [ ] | [ ] |
| **Pé Direito** | `Foot_R_M.uvw` | [x] | [x] | [x] | [x] | [ ] | [ ] |

---

## 3. Checklist de Produção: Elementos do Rosto (Sistema Facial Modular em Inglês)
> Todos os sub-elementos faciais seguem nomenclatura 100% em inglês com sufixo `_M` (Male). A coluna **4. Rig** controla o vínculo de ossos oculares / Morph Targets (piscar, expressões) antes de animar.

| Elemento Facial | Nome Antigo (PT) | Nome Atualizado (EN) | 1. Malha | 2. UVMap | 3. Textura | 4. Rig (Osso/Morph) | 5. Animações | 6. Efeitos (Brilho/VFX) |
| :--- | :--- | :--- | :---: | :---: | :---: | :---: | :---: | :---: |
| **Íris (Cor do Olho)** | `Iris_M` | **`Iris_M.uvw`** | [x] | [x] | [x] | [ ] | [ ] | [ ] |
| **Centro da Íris (Pupila)** | `Centro_iris_M` | **`Iris_Center_M.uvw`** | [x] | [x] | [x] | [ ] | [ ] | [ ] |
| **Brilho da Íris (Highlight)** | `Brilho_iris_M` | **`Iris_Highlight_M.uvw`** | [x] | [x] | [x] | [ ] | [ ] | [ ] |
| **Fundo da Íris / Esclera** | `Fundo_iris_M` | **`Iris_Back_M.uvw`** / **`White_eye_M`** | [x] | [x] | [x] | [ ] | [ ] | [ ] |
| **Pálpebra Superior/Inferior** | `Palpebra_M` | **`Eyelid_M.uvw`** | [x] | [x] | [x] | [ ] | [ ] | [ ] |
| **Cílios** | `Cilios_M` | **`Eyelashes_M.uvw`** | [x] | [x] | [x] | [ ] | [ ] | [ ] |
| **Sobrancelhas** | `Sobrancelhas_M` | **`Eyebrows_M.uvw`** | [x] | [x] | [x] | [ ] | [ ] | [ ] |
| **Fundo das Bochechas (Blush)** | `Fundo_bochechas_M` | **`Cheek_Blush_M.uvw`** | [x] | [x] | [x] | [ ] | [ ] | [ ] |
| **Arcada Dentária (Dentes)** | `Dentes_M` | **`Teeth_M.uvw`** | [x] | [x] | [x] | [ ] | [ ] | [ ] |
| **Língua (Vermelho Sólido)** | `Lingua_M` | **`Tongue_M.uvw`** | [x] | [x] | [x] | [ ] | [ ] | [ ] |
| **Fundo da Boca (Vermelho Escuro Sólido)** | `Boca_M` | **`Mouth_M.uvw`** | [x] | [x] | [x] | [ ] | [ ] | [ ] |

---

## 4. Checklist de Produção: Cabelos Modulares (A Iniciar)
> Ver documentação técnica completa em [`Cabelo_Modular_Masculino.md`](./Cabelo_Modular_Masculino.md).

| Componente de Cabelo | Arquivo UVW Previsto | 1. Malha | 2. UVMap | 3. Textura (Mask RGB) | 4. Rig (Ossos/Morphs) | 5. Animações | 6. Efeitos (Shader/Física) |
| :--- | :--- | :---: | :---: | :---: | :---: | :---: | :---: |
| **Cabelo Base Masculino 01 (Curto Espetado)** | `Hair_01_M.uvw` | [x] | [x] | [x] | [x] | [ ] | [ ] |
| **Franja Modular 01 (Front Hair)** | `Hair_Front_01_M.uvw` | [ ] | [ ] | [ ] | [ ] | [ ] | [ ] |
| **Versão Capuz/Elmo (Hair Cap Compacto)** | `Hair_Cap_01_M.uvw` | [ ] | [ ] | [ ] | [ ] | [ ] | [ ] |

---

## 5. Lista Padrão de Cores Reais (Pele, Cabelo, Sobrancelhas, Cílios e Olhos)
> **Referência Visual Geral:** [`../set_cores_padrao_personagens.jpg`](../set_cores_padrao_personagens.jpg)  
> **Configuração Inicial Padrão (Default no Jogo):** Pele Clara Quente (`#FFDBB5`) + Cabelo, Sobrancelhas e Cílios **Castanhos (Marrom)** (`#6B4435`) + Olhos **Castanho Claro** (`#9E6336`).

### 5.1. Cabelo, Sobrancelhas (`Eyebrows_M.uvw`) e Cílios (`Eyelashes_M.uvw`) — 5 Cores Base
*(As sobrancelhas e cílios acompanham automaticamente a cor do cabelo por padrão, usando o tom da Raiz para maior contraste facial)*

| Ordem / ID | Nome da Cor | Cor Raiz / Cílios / Sobrancelha | Cor Base Principal | Cor Brilho / Pontas | Status no Jogo |
| :--- | :--- | :---: | :---: | :---: | :---: |
| **01 (PADRÃO)** | **Castanho (Marrom Natural)** | `#4A3028` | `#6B4435` | `#8B5A42` | **Cor Padrão Inicial** |
| **02** | **Preto (Ébano Natural)** | `#141416` | `#1F1F24` | `#383842` | Básico Inicial |
| **03** | **Branco Acinzentado (Grisalho/Platinado)** | `#7A7E85` | `#C4C9D1` | `#E8ECF2` | Básico Inicial |
| **04** | **Ruivo (Cobre / Acobreado)** | `#6B2214` | `#A83E24` | `#D4623B` | Básico Inicial |
| **05** | **Loiro (Dourado Natural)** | `#8C6B2B` | `#D9B45B` | `#F2D680` | Básico Inicial |

### 5.2. Olhos / Íris (`Iris_M.uvw` e `Iris_Center_M.uvw`) — 5 Cores Reais Mais Comuns

| Ordem / ID | Nome da Cor dos Olhos | Íris Base (`Iris_M`) | Centro/Pupila (`Iris_Center_M`) | Highlight/Anel (`Iris_Highlight_M`) | Status no Jogo |
| :--- | :--- | :---: | :---: | :---: | :---: |
| **01 (PADRÃO)** | **Castanho Claro (Mel / Âmbar)** | `#9E6336` | `#4D2C16` | `#D4955A` | **Cor Padrão Inicial** |
| **02** | **Castanho Escuro (Café)** | `#4A2C18` | `#1F1108` | `#75492B` | Básico Inicial |
| **03** | **Azul Claro (Safira Real)** | `#3B7EA1` | `#1B4359` | `#78BBE6` | Básico Inicial |
| **04** | **Verde (Esmeralda / Avelã)** | `#4B8B5A` | `#23472B` | `#85C792` | Básico Inicial |
| **05** | **Cinza (Acinzentado Prateado)** | `#7E8C94` | `#424B52` | `#B8C7D1` | Básico Inicial |

### 5.3. Tons de Pele da Vida Real (`Head_M`, `Torso_M`, Membros e `Cheek_Blush_M`) — 5 Tons Reais

| Ordem / ID | Tom de Pele (Escala Real) | Cor Base da Pele | Sombra / Subsurface | Blush (`Cheek_Blush_M`) | Status no Jogo |
| :--- | :--- | :---: | :---: | :---: | :---: |
| **01 (PADRÃO)** | **Pele Clara Rosada / Pêssego** | `#FFDBB5` | `#E5B58D` | `#F09B8C` | **Cor Padrão Inicial** |
| **02** | **Pele Muito Clara / Marfim** | `#FFE9D6` | `#EBC6AC` | `#F5A89E` | Básico Inicial |
| **03** | **Pele Trigo / Morena Clara** | `#E8B88A` | `#C99364` | `#D97F6C` | Básico Inicial |
| **04** | **Pele Morena Média / Oliva** | `#C68B59` | `#A36A3B` | `#B85D4B` | Básico Inicial |
| **05** | **Pele Negra / Marrom Quente** | `#8D5524` | `#6B3C14` | `#853B2B` | Básico Inicial |

---

## 6. Checklist Passo a Passo das Máscaras de Textura de Cada `.uvw` (`0_MALE`)
> Siga as colunas da esquerda para a direita para criar/vincular as máscaras de textura de cada um dos **19 arquivos `.uvw` masculinos**:

| Arquivo `.uvw` | Tipo de Máscara / Canais Utilizados | 1. Renderizar Template UV | 2. Pintar / Gerar Máscara PNG | 3. Desativar sRGB (UE5) | 4. Conectar Parâmetro Cor | 5. Testar 5 Cores |
| :--- | :--- | :---: | :---: | :---: | :---: | :---: |
| **`Head_M.uvw`** | **Canal R:** Sombra suave (pescoço/orelha/nariz) \| **Canal G:** Rubor facial leve | [x] | [x] | [ ] | [ ] | [ ] |
| **`Torso_M.uvw`** | **Canal R:** Gradiente suave de oclusão (axilas/cintura) \| **Canal B:** Roupa íntima (`#3A3A3A`) | [x] | [x] | [ ] | [ ] | [ ] |
| **`Pelvis_M.uvw`** | **Canal R:** Oclusão base \| **Canal B:** Máscara 100% branca para cor do Boxer Briefs (`#3A3A3A`) | [x] | [x] | [ ] | [ ] | [ ] |
| **`Arm_L_M.uvw`** | **Canal R:** Gradiente suave ombro -> mão (tom levemente mais quente nos dedos) | [x] | [x] | [ ] | [ ] | [ ] |
| **`Arm_R_M.uvw`** | **Canal R:** Gradiente suave ombro -> mão (espelhado do `Arm_L_M`) | [x] | [x] | [ ] | [ ] | [ ] |
| **`Leg_L_M.uvw`** | **Canal R:** Oclusão suave na coxa + leve tom rosado no joelho (`Canal G`) | [x] | [x] | [ ] | [ ] | [ ] |
| **`Leg_R_M.uvw`** | **Canal R:** Oclusão suave na coxa + leve tom rosado no joelho (`Canal G`) | [x] | [x] | [ ] | [ ] | [ ] |
| **`Foot_L_M.uvw`** | **Canal R:** Sombra na sola do pé e entre os dedos | [x] | [x] | [ ] | [ ] | [ ] |
| **`Foot_R_M.uvw`** | **Canal R:** Sombra na sola do pé e entre os dedos | [x] | [x] | [ ] | [ ] | [ ] |
| **`Iris_M.uvw`** | **Canal R:** Degradê vertical (escuro em cima -> claro embaixo) \| **Canal G:** Aro externo da íris | [x] | [x] | [ ] | [ ] | [ ] |
| **`Iris_Center_M.uvw`**| **Máscara Sólida (`1.0`):** Recebe `PupilCenterColor` (tom escuro da cor do olho) | [x] | [x] | [ ] | [ ] | [ ] |
| **`Iris_Highlight_M.uvw`**| **Cor Fixa Não-Personalizável (`#FFFFFF`):** Branco puro emissivo | [x] | [x] | [ ] | [ ] | [ ] |
| **`Iris_Back_M.uvw` / `White_eye_M`** | **Cor Fixa Não-Personalizável (`#F7F4F0`):** **Canal R** com sombra suave no topo (abaixo da pálpebra) | [x] | [x] | [ ] | [ ] | [ ] |
| **`Eyelid_M.uvw`** | **Canal R:** Degradê do centro para fora \| Recebe `SkinSubsurfaceShadow` | [x] | [x] | [ ] | [ ] | [ ] |
| **`Eyelashes_M.uvw`** | **Canal R:** Degradê do centro para fora \| Recebe `RootColor` do cabelo escolhido | [x] | [x] | [ ] | [ ] | [ ] |
| **`Eyebrows_M.uvw`**| **Canal R:** Degradê do centro para fora \| Recebe `RootColor` do cabelo escolhido | [x] | [x] | [ ] | [ ] | [ ] |
| **`Cheek_Blush_M.uvw`**| **Canal R / Alpha:** Gradiente oval suave no centro \| Recebe `BlushColor` | [x] | [x] | [ ] | [ ] | [ ] |
| **`Teeth_M.uvw`** | **Cor Fixa Não-Personalizável (`#FAF6F0`):** **Canal R** com leve sombra de gengiva/oclusão nos cantos | [x] | [x] | [ ] | [ ] | [ ] |
| **`Tongue_M.uvw`** | **Cor Fixa Não-Personalizável (Vermelho Sólido `R=220` / `#D96B6B`):** Máscara vermelha sólida completa | [x] | [x] | [ ] | [ ] | [ ] |
| **`Mouth_M.uvw`** | **Cor Fixa Não-Personalizável (Vermelho Escuro Sólido `R=130` / `#8B2626`):** Máscara vermelha escura sólida completa | [x] | [x] | [ ] | [ ] | [ ] |

---

## 7. Especificações Técnicas da Malha Base Masculina
- **Arquivo Fonte 3ds Max:** `Hero_Chibi_Male.max`
- **Arquivo Rigged Exportado:** `Hero_Chibi_Male_RIG.fbx`
- **Contagem Poligonal:** ~4.530 polígonos / ~3.668 vértices.
- **Sockets Configurados na UE5:**
  - `Socket_Weapon_R` (no osso `hand_r`, ajustado para o centro da palma da mão direita).
  - `Socket_Weapon_L` (no osso `hand_l`, ajustado para a mão/antebraço esquerdo).
  - `Socket_Hair_Attachment` (no osso `head`, para encaixe de cabelos e elmos modulares).
- **Retargeting na UE5:**
  - `root`: *Animation*
  - `pelvis`: *Animation Scaled*
  - Demais ossos da cadeia (`spine`, braços, mãos, pernas): *Skeleton* (impede deformação das proporções Chibi).
