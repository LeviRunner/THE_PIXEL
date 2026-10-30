# Checklist de Produção: Base Feminina (`Hero_Chibi_Female`)

**Contexto:** O modelo base feminino (`Hero_Chibi_Female.max`) segue a mesma arquitetura modular e proporções Chibi Cartoon da versão masculina, com silhueta ajustada nos ombros, cintura e quadril, compartilhando o mesmo padrão de mapeamento `.uvw` para reaproveitamento total de shaders faciais e animações.

---

## 1. Visão Geral das 6 Bases de Produção por Elemento
Cada componente anatômico da personagem passa pelos **6 pilares de produção**:
1. **Malha (Mesh 3D):** Modelagem low-poly estilizada com os mesmos loops de corte nos ombros, cotovelos, cintura e joelhos.
2. **UVMap (`.uvw`):** Mapeamento de coordenadas UV seguindo a mesma nomenclatura padronizada com sufixo `_F`.
3. **Textura (Material/Texture):** Pintura base, máscara de cor parametrizável e shader PBR estilizado.
4. **Rig (Skin / Ossos / Morphs):** Rigging no esqueleto AccuRIG + Rigging/Morph Targets de Olhos, Pálpebras, Sobrancelhas, Bochechas e Cabelo.
5. **Animações (Sets Autorais):** Ciclos de movimento, expressões faciais/piscar, física de cabelo e ataques.
6. **Efeitos (VFX/Shader):** Brilho ocular, piscar, flash de impacto (Hit Flash) e efeitos de aura.

---

## 2. Checklist de Produção: Corpo Base (Pele)
> Estrutura espelhada na lista de arquivos `.uvw` da base feminina (`1_FEMALE`).

| Parte do Corpo | Arquivo UVW Referência | 1. Malha | 2. UVMap | 3. Textura | 4. Rig (Skin) | 5. Animações | 6. Efeitos (Shader/VFX) |
| :--- | :--- | :---: | :---: | :---: | :---: | :---: | :---: |
| **Cabeça (Crânio/Rosto)** | `Head_F.uvw` | [x] | [x] | [x] | [x] | [ ] | [ ] |
| **Torso / Peito Base** | `Torso_F.uvw` | [x] | [x] | [x] | [x] | [ ] | [ ] |
| **Pelve / Quadril Base** | `Pelvis_F.uvw` | [x] | [x] | [x] | [x] | [ ] | [ ] |
| **Braço Esquerdo** | `Arm_L_F.uvw` | [x] | [x] | [x] | [x] | [ ] | [ ] |
| **Braço Direito** | `Arm_R_F.uvw` | [x] | [x] | [x] | [x] | [ ] | [ ] |
| **Perna Esquerda** | `Leg_L_F.uvw` | [x] | [x] | [x] | [x] | [ ] | [ ] |
| **Perna Direita** | `Leg_R_F.uvw` | [x] | [x] | [x] | [x] | [ ] | [ ] |
| **Pé Esquerdo** | `Foot_L_F.uvw` | [x] | [x] | [x] | [x] | [ ] | [ ] |
| **Pé Direito** | `Foot_R_F.uvw` | [x] | [x] | [x] | [x] | [ ] | [ ] |

---

## 3. Checklist de Produção: Elementos do Rosto (Sistema Facial Modular em Inglês)
> Todos os sub-elementos faciais seguem nomenclatura 100% em inglês com sufixo `_F` (Female). A coluna **4. Rig** controla o vínculo de ossos oculares / Morph Targets (piscar, expressões) antes de animar.

| Elemento Facial | Nome Antigo (PT) | Nome Atualizado (EN) | 1. Malha | 2. UVMap | 3. Textura | 4. Rig (Osso/Morph) | 5. Animações | 6. Efeitos (Brilho/VFX) |
| :--- | :--- | :--- | :---: | :---: | :---: | :---: | :---: | :---: |
| **Íris (Cor do Olho)** | `Iris_F` | **`Iris_F`** | [x] | [x] | [x] | [ ] | [ ] | [ ] |
| **Centro da Íris (Pupila)** | `Centro_iris_F` | **`Iris_Center_F`** | [x] | [x] | [x] | [ ] | [ ] | [ ] |
| **Brilho da Íris (Highlight - Cor Fixa)** | `Brilho_iris_F` | **`Iris_Highlight_F`** | [x] | [x] | [x] | [ ] | [ ] | [ ] |
| **Fundo da Íris (Anel Interno)** | `Fundo_iris_F` | **`Iris_Back_F`** | [x] | [x] | [x] | [ ] | [ ] | [ ] |
| **Fundo Branco do Olho (Esclera - Cor Fixa)** | `White_eye_F` | **`White_eye_F`** | [x] | [x] | [x] | [ ] | [ ] | [ ] |
| **Pálpebra Superior/Inferior** | `Palpebra_F` | **`Eyelid_F`** | [x] | [x] | [x] | [ ] | [ ] | [ ] |
| **Sombra Interna do Olho (Preto Translúcido)** | `01sombra_interna_olho` | **`Eye_Inner_Shadow_F`** | [x] | [x] | [x] | [ ] | [ ] | [ ] |
| **Cílios (Alongados Fem.)** | `Cilios_F` | **`Eyelashes_F`** | [x] | [x] | [x] | [ ] | [ ] | [ ] |
| **Sobrancelhas** | `Sobrancelhas_F` | **`Eyebrows_F`** | [x] | [x] | [x] | [ ] | [ ] | [ ] |
| **Linhas do Rosto (Nariz + Lábio - Preto Sólido)** | `01sombra_nariz / boca` | **`Face_Lines_F`** | [x] | [x] | [x] | [ ] | [ ] | [ ] |
| **Touca / Fundo Fixo do Cabelo (Raiz)** | `Hair_Cap_F` | **`Hair_Cap_F`** | [x] | [x] | [x] | [ ] | [ ] | [ ] |
| **Fundo das Bochechas (Blush)** | `Fundo_bochechas_F` | **`Cheek_Blush_F`** | [x] | [x] | [x] | [ ] | [ ] | [ ] |
| **Arcada Dentária (Dentes - Cor Fixa)** | `Dentes_F` | **`Teeth_F`** | [x] | [x] | [x] | [ ] | [ ] | [ ] |
| **Língua (Vermelho Sólido Fixo)** | `Lingua_F` | **`Tongue_F`** | [x] | [x] | [x] | [ ] | [ ] | [ ] |
| **Fundo da Boca (Vermelho Escuro Sólido Fixo)** | `Boca_F` | **`Mouth_F`** | [x] | [x] | [x] | [ ] | [ ] | [ ] |

---

## 4. Checklist de Produção: Cabelos Modulares (A Iniciar)
> Ver documentação técnica completa em [`Cabelo_Modular_Feminino.md`](./Cabelo_Modular_Feminino.md).

| Componente de Cabelo | Arquivo UVW Previsto | 1. Malha | 2. UVMap | 3. Textura (Mask RGB) | 4. Rig (Ossos/Morphs) | 5. Animações | 6. Efeitos (Shader/Física) |
| :--- | :--- | :---: | :---: | :---: | :---: | :---: | :---: |
| **Cabelo Base Feminino 01 (Longo/Médio)** | `Hair_01_F.uvw` | [x] | [x] | [x] | [x] | [ ] | [ ] |
| **Franja Modular 01 (Front Hair)** | `Hair_Front_01_F.uvw` | [ ] | [ ] | [ ] | [ ] | [ ] | [ ] |
| **Versão Capuz/Elmo (Hair Cap Compacto)** | `Hair_Cap_01_F.uvw` | [ ] | [ ] | [ ] | [ ] | [ ] | [ ] |

---

## 5. Lista Padrão de Cores Reais (Pele, Cabelo, Sobrancelhas, Cílios e Olhos)
> **Referência Visual Geral:** [`../set_cores_padrao_personagens.jpg`](../set_cores_padrao_personagens.jpg)  
> **Configuração Inicial Padrão (Default no Jogo):** Pele Clara Quente (`#FFDBB5`) + Cabelo, Sobrancelhas e Cílios **Castanhos (Marrom)** (`#6B4435`) + Olhos **Castanho Claro** (`#9E6336`).

### 5.1. Cabelo, Sobrancelhas (`Eyebrows_F.uvw`) e Cílios (`Eyelashes_F.uvw`) — 5 Cores Base
*(As sobrancelhas e cílios acompanham automaticamente a cor do cabelo por padrão, usando o tom da Raiz para maior contraste facial)*

| Ordem / ID | Nome da Cor | Cor Raiz / Cílios / Sobrancelha | Cor Base Principal | Cor Brilho / Pontas | Status no Jogo |
| :--- | :--- | :---: | :---: | :---: | :---: |
| **01 (PADRÃO)** | **Castanho (Marrom Natural)** | `#4A3028` | `#6B4435` | `#8B5A42` | **Cor Padrão Inicial** |
| **02** | **Preto (Ébano Natural)** | `#141416` | `#1F1F24` | `#383842` | Básico Inicial |
| **03** | **Branco Acinzentado (Grisalho/Platinado)** | `#7A7E85` | `#C4C9D1` | `#E8ECF2` | Básico Inicial |
| **04** | **Ruivo (Cobre / Acobreado)** | `#6B2214` | `#A83E24` | `#D4623B` | Básico Inicial |
| **05** | **Loiro (Dourado Natural)** | `#8C6B2B` | `#D9B45B` | `#F2D680` | Básico Inicial |

### 5.2. Olhos / Íris (`Iris_F.uvw` e `Iris_Center_F.uvw`) — 5 Cores Reais Mais Comuns

| Ordem / ID | Nome da Cor dos Olhos | Íris Base (`Iris_F`) | Centro/Pupila (`Iris_Center_F`) | Highlight/Anel (`Iris_Highlight_F`) | Status no Jogo |
| :--- | :--- | :---: | :---: | :---: | :---: |
| **01 (PADRÃO)** | **Castanho Claro (Mel / Âmbar)** | `#9E6336` | `#4D2C16` | `#D4955A` | **Cor Padrão Inicial** |
| **02** | **Castanho Escuro (Café)** | `#4A2C18` | `#1F1108` | `#75492B` | Básico Inicial |
| **03** | **Azul Claro (Safira Real)** | `#3B7EA1` | `#1B4359` | `#78BBE6` | Básico Inicial |
| **04** | **Verde (Esmeralda / Avelã)** | `#4B8B5A` | `#23472B` | `#85C792` | Básico Inicial |
| **05** | **Cinza (Acinzentado Prateado)** | `#7E8C94` | `#424B52` | `#B8C7D1` | Básico Inicial |

### 5.3. Tons de Pele da Vida Real (`Head_F`, `Torso_F`, Membros e `Cheek_Blush_F`) — 5 Tons Reais

| Ordem / ID | Tom de Pele (Escala Real) | Cor Base da Pele | Sombra / Subsurface | Blush (`Cheek_Blush_F`) | Status no Jogo |
| :--- | :--- | :---: | :---: | :---: | :---: |
| **01 (PADRÃO)** | **Pele Clara Rosada / Pêssego** | `#FFDBB5` | `#E5B58D` | `#F09B8C` | **Cor Padrão Inicial** |
| **02** | **Pele Muito Clara / Marfim** | `#FFE9D6` | `#EBC6AC` | `#F5A89E` | Básico Inicial |
| **03** | **Pele Trigo / Morena Clara** | `#E8B88A` | `#C99364` | `#D97F6C` | Básico Inicial |
| **04** | **Pele Morena Média / Oliva** | `#C68B59` | `#A36A3B` | `#B85D4B` | Básico Inicial |
| **05** | **Pele Negra / Marrom Quente** | `#8D5524` | `#6B3C14` | `#853B2B` | Básico Inicial |

---

## 6. Checklist Passo a Passo das Máscaras de Textura de Cada `.uvw` / `.png` (`1_FEMALE`)
> Siga as colunas da esquerda para a direita para criar/vincular as máscaras de textura de cada arquivo da base feminina (`1_FEMALE`):

| Arquivo `.uvw` / `.png` (EN) | Máscara em `Masks_RGB\` | Tipo de Máscara / Canais Utilizados | 1. Renderizar Template UV | 2. Pintar / Gerar Máscara PNG | 3. Desativar sRGB (UE5) | 4. Conectar Parâmetro Cor | 5. Testar 5 Cores |
| :--- | :--- | :--- | :---: | :---: | :---: | :---: | :---: |
| **`Head_F.uvw`** | `Head_F_Mask.png` | **Canal R:** Rubor simétrico das bochechas, ponta do nariz, concha das orelhas e sombra do pescoço | [x] | [x] | [ ] | [ ] | [ ] |
| **`Torso_F.uvw`** | `Torso_F_Mask.png` | **Canal R:** Oclusão suave (cintura/colo) \| **Canal B:** Máscara 100% compacta do Top Esportivo (`#3A3A3A`) | [x] | [x] | [ ] | [ ] | [ ] |
| **`Pelvis_F.uvw`** | `Pelvis_F_Mask.png` | **Canal R:** Oclusão nas virilhas \| **Canal B:** Máscara 100% Azul compacta (`R=0, G=0, B=255`) para Shorts/Calcinha (`#3A3A3A`) | [x] | [x] | [ ] | [ ] | [ ] |
| **`Arm_L_F.uvw`** | `Arm_L_F_Mask.png` | **Canal R:** Gradiente contínuo do ombro -> mão (tom mais quente nos dedos) | [x] | [x] | [ ] | [ ] | [ ] |
| **`Arm_R_F.uvw`** | `Arm_R_F_Mask.png` | **Canal R:** Gradiente contínuo do ombro -> mão (espelhado do `Arm_L_F`) | [x] | [x] | [ ] | [ ] | [ ] |
| **`Leg_L_F.uvw`** | `Leg_L_F_Mask.png` | **Canal R:** Oclusão suave na coxa e sombra firme no tornozelo + leve tom no joelho (`Canal G`) | [x] | [x] | [ ] | [ ] | [ ] |
| **`Leg_R_F.uvw`** | `Leg_R_F_Mask.png` | **Canal R:** Oclusão suave na coxa e sombra firme no tornozelo + leve tom no joelho (`Canal G`) | [x] | [x] | [ ] | [ ] | [ ] |
| **`Foot_L_F.uvw`** | `Foot_L_F_Mask.png` | **Canal R:** Sombra de transição no tornozelo, sola do pé e entre os dedos | [x] | [x] | [ ] | [ ] | [ ] |
| **`Foot_R_F.uvw`** | `Foot_R_F_Mask.png` | **Canal R:** Sombra de transição no tornozelo, sola do pé e entre os dedos | [x] | [x] | [ ] | [ ] | [ ] |
| **`Iris_F.uvw`** | `Iris_F_Mask.png` | **Canal R:** Degradê vertical (escuro em cima -> claro embaixo) \| **Canal G:** Aro externo da íris | [x] | [x] | [ ] | [ ] | [ ] |
| **`Iris_Center_F.uvw`** | `Iris_Center_F_Mask.png` | **Máscara Sólida (`1.0`):** Recebe `PupilCenterColor` (tom escuro da cor do olho) | [x] | [x] | [ ] | [ ] | [ ] |
| **`Iris_Highlight_F.uvw`** | `Iris_Highlight_F_Mask.png` | **Cor Fixa Não-Personalizável (`#FFFFFF`):** Branco puro emissivo | [x] | [x] | [ ] | [ ] | [ ] |
| **`Iris_Back_F.uvw`** | `Iris_Back_F_Mask.png` | **Cor Fixa Não-Personalizável:** Sombra interna do globo ocular | [x] | [x] | [ ] | [ ] | [ ] |
| **`White_eye_F.png`** | `White_eye_F_Mask.png` | **Cor Fixa Não-Personalizável (`#F7F4F0`):** Fundo branco do olho com **Canal R** de sombra suave da pálpebra superior | [x] | [x] | [ ] | [ ] | [ ] |
| **`Eyelid_F.uvw`** | `Eyelid_F_Mask.png` | **Canal R:** Degradê suave do centro do rosto para fora nas duas ilhas \| Recebe `SkinSubsurfaceShadow` | [x] | [x] | [ ] | [ ] | [ ] |
| **`Eye_Inner_Shadow_F.uvw`** | `Eye_Inner_Shadow_F_Mask.png` | **Canal R / Translucidez:** Sombra superior interna do olho (Preto `#000000` com ~40% de Opacidade) | [x] | [x] | [ ] | [ ] | [ ] |
| **`Eyelashes_F.uvw`** | `Eyelashes_F_Mask.png` | **Canal R:** Degradê suave do centro do rosto para fora nas ilhas \| Recebe `RootColor` do cabelo | [x] | [x] | [ ] | [ ] | [ ] |
| **`Eyebrows_F.uvw`** | `Eyebrows_F_Mask.png` | **Canal R:** Degradê suave do centro do rosto para fora nas duas ilhas \| Recebe `RootColor` do cabelo | [x] | [x] | [ ] | [ ] | [ ] |
| **`Face_Lines_F.uvw`** | `Face_Lines_F_Mask.png` | **Cor Fixa Não-Personalizável (`#000000`):** Traço preto sólido da ponta direita do nariz e lábio superior | [x] | [x] | [ ] | [ ] | [ ] |
| **`Hair_Cap_F.uvw`** | `Hair_Cap_F_Mask.png` | **Canal R:** Degradê suave da coroa capilar \| Recebe `RootColor` escuro do cabelo (`#4A3028`) | [x] | [x] | [ ] | [ ] | [ ] |
| **`Cheek_Blush_F.uvw`** | `Cheek_Blush_F_Mask.png` | **Canal R / Alpha:** Gradiente oval suave no centro \| Recebe `BlushColor` | [x] | [x] | [ ] | [ ] | [ ] |
| **`Teeth_F.uvw`** | `Teeth_F_Mask.png` | **Cor Fixa Não-Personalizável (`#FAF6F0`):** **Canal R** com leve sombra de gengiva/oclusão nos cantos | [x] | [x] | [ ] | [ ] | [ ] |
| **`Tongue_F.uvw`** | `Tongue_F_Mask.png` | **Cor Fixa Não-Personalizável (Vermelho Sólido `R=220` / `#D96B6B`):** Máscara vermelha sólida completa | [x] | [x] | [ ] | [ ] | [ ] |
| **`Mouth_F.uvw`** | `Mouth_F_Mask.png` | **Cor Fixa Não-Personalizável (Vermelho Escuro Sólido `R=130` / `#8B2626`):** Máscara vermelha escura sólida completa para o interior da boca | [x] | [x] | [ ] | [ ] | [ ] |

---

## 7. Especificações Técnicas da Malha Base Feminina
- **Arquivo Fonte 3ds Max:** `Hero_Chibi_Female.max`
- **Arquivos FBX Exportados:** `Hero_Chibi_Female.fbx` / `Hero_Chibi_Female_2.fbx`
- **Sockets Padrão na UE5 (idênticos ao masculino para compartilhar código e armas):**
  - `Socket_Weapon_R` (no osso `hand_r`, ajustado para a palma da mão direita).
  - `Socket_Weapon_L` (no osso `hand_l`, ajustado para a mão esquerda).
  - `Socket_Hair_Attachment` (no osso `head`, para cabelos e acessórios de cabeça).
- **Retargeting / Compartilhamento de Esqueleto:**
  - Processada via AccuRIG com a mesma hierarquia de ossos (`root` -> `pelvis` -> `spine` -> membros), permitindo que **100% das animações geradas para o esqueleto Chibi funcionem em ambos os gêneros**.
