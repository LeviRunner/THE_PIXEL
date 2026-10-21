# Guia Técnico e Checklist Passo a Passo: Cabelo Modular (`Hero_Chibi_Female`)

> **Objetivo:** Guia prático com **checklists em tabelas (linhas e colunas)** para você ir marcando `[x]` em cada etapa da criação dos cabelos modulares femininos (3 slots combináveis, ajuste de Tamanho, Cor via Máscara RGB e Rigging/Física).

---

## 1. Checklist Passo a Passo de Execução (Para Cada Cabelo Novo)
> Siga as etapas de **01 a 10** na ordem abaixo para produzir o Cabelo Base Feminino:

| Passo | Etapa de Produção | O Que Fazer no 3ds Max / UE5 | Base 01 (`Hair_01_F`) | Estilo 02 (`Hair_02_F`) | Estilo 03 (`Hair_03_F`) | Estilo 04 (`Hair_04_F`) | Estilo 05 (`Hair_05_F`) |
| :---: | :--- | :--- | :---: | :---: | :---: | :---: | :---: |
| **01** | **Modelar Base Topo** | Modelar a calota craniana (`300–500 tris`) usando `Head_F` como manequim | [ ] | [ ] | [ ] | [ ] | [ ] |
| **02** | **Separar Franja (`Front`)** | Destacar a franja e mechas laterais do rosto (`Hair_Front_XX_F`) | [ ] | [ ] | [ ] | [ ] | [ ] |
| **03** | **Separar Extensão (`Back`)**| Destacar rabo de cavalo, trança, maria-chiquinha ou longo solto (`Hair_Back_XX_F`) | [ ] | [ ] | [ ] | [ ] | [ ] |
| **04** | **Alinhar Pivô** | Posicionar o Pivô exatamente no centro do osso `head` | [ ] | [ ] | [ ] | [ ] | [ ] |
| **05** | **Abrir UV na Vertical** | Aplicar `Unwrap UVW`, alinhar raiz no topo e ponta embaixo e salvar `Hair_XX_F.uvw` | [ ] | [ ] | [ ] | [ ] | [ ] |
| **06** | **Aplicar Máscara RGB** | Vincular a máscara `T_Hair_Mask_RGB.png` (R=Degradê, G=Brilho, B=Mecha/Fita) | [ ] | [ ] | [ ] | [ ] | [ ] |
| **07** | **Criar Morph Curto/Longo** | Criar `Morph_Length_Short` (altura ombro) e `Morph_Length_Long` (altura cintura) | [ ] | [ ] | [ ] | [ ] | [ ] |
| **08** | **Criar Morph Volume** | Criar `Morph_Volume_Full` (+15% volume nas mechas externas) | [ ] | [ ] | [ ] | [ ] | [ ] |
| **09** | **Rigging / Ossos de Física**| Aplicar *Skin* no osso `head` + ossos dinâmicos `hair_back_01` / `hair_tail_l/r` | [ ] | [ ] | [ ] | [ ] | [ ] |
| **10** | **Exportar FBX & Testar UE5**| Exportar com *Morph Targets* e testar as 5 cores (`Castanho`, `Preto`, `Branco`, `Ruivo`, `Loiro`) | [ ] | [ ] | [ ] | [ ] | [ ] |

---

## 2. Checklist de Produção por Sub-Peça do Cabelo (As 6 Bases)

| Sub-Peça Modular | Arquivo `.uvw` | 1. Malha | 2. UVMap | 3. Textura Mask | 4. Rig (Osso/Morph) | 5. Animações | 6. Efeitos (Shader/Física) |
| :--- | :--- | :---: | :---: | :---: | :---: | :---: | :---: |
| **Cabelo Base 01 (Longo Padrão)** | `Hair_01_F.uvw` | [ ] | [ ] | [ ] | [ ] | [ ] | [ ] |
| **Franja Modular 01 (Front Hair)** | `Hair_Front_01_F.uvw` | [ ] | [ ] | [ ] | [ ] | [ ] | [ ] |
| **Extensão Traseira 01 (Back Hair)** | `Hair_Back_01_F.uvw` | [ ] | [ ] | [ ] | [ ] | [ ] | [ ] |
| **Hair Cap 01 (Versão Elmo/Capuz)** | `Hair_Cap_01_F.uvw` | [ ] | [ ] | [ ] | [ ] | [ ] | [ ] |
| **Cabelo Estilo 02 (Chanel/Bob)** | `Hair_02_F.uvw` | [ ] | [ ] | [ ] | [ ] | [ ] | [ ] |
| **Cabelo Estilo 03 (Rabo de Cavalo)**| `Hair_03_F.uvw` | [ ] | [ ] | [ ] | [ ] | [ ] | [ ] |
| **Cabelo Estilo 04 (Maria-Chiquinha)**| `Hair_04_F.uvw` | [ ] | [ ] | [ ] | [ ] | [ ] | [ ] |
| **Cabelo Estilo 05 (Trança Longa)** | `Hair_05_F.uvw` | [ ] | [ ] | [ ] | [ ] | [ ] | [ ] |

---

## 3. Checklist da Máscara de Textura dos UVs de Cabelo (`1_FEMALE`)

| Arquivo `.uvw` | Canal R (Raiz -> Ponta) | Canal G (Faixa de Brilho) | Canal B (Mecha / Presilha) | 1. UV Vertical | 2. Mask PNG Aplicada | 3. sRGB Desativado (UE5) | 4. 5 Cores Testadas |
| :--- | :--- | :--- | :--- | :---: | :---: | :---: | :---: |
| **`Hair_01_F.uvw`** | Preto no topo (Raiz) -> Branco embaixo (Ponta) | Faixa branca horizontal a 35% do topo | Branco nas mechas/presilhas secundárias | [ ] | [ ] | [ ] | [ ] |
| **`Hair_Front_01_F.uvw`** | Preto na raiz da testa -> Branco na ponta da franja | Faixa branca alinhada à altura da cabeça | Branco opcional em mechas laterais | [ ] | [ ] | [ ] | [ ] |
| **`Hair_Back_01_F.uvw`** | Gradiente contínuo da nuca até a ponta longa | Faixa de brilho no meio do comprimento | Branco em fitas/laços de amarrar | [ ] | [ ] | [ ] | [ ] |
| **`Hair_Cap_01_F.uvw`** | Gradiente suave da nuca | Sem brilho forte (fica sob o elmo) | Preto total (`0`) | [ ] | [ ] | [ ] | [ ] |

---

## 4. Resumo Rápido da Lógica Técnica (Para Consulta)

1. **Ajuste de Tamanho no Painel de Personalização:**
   - O slider da UI controla os Morph Targets `Morph_Length_Short` (`-1.0`) até `Morph_Length_Long` (`+1.0`) e `Morph_Volume_Full`, além da escala local dos ossos de extensão (`hair_back_01`).
2. **Ajuste de Cor Dinâmica (5 Cores Reais Iniciais):**
   - Usa a textura universal [`../T_Hair_Mask_RGB.png`](../T_Hair_Mask_RGB.png).
   - **01 Castanho (Padrão):** Raiz `#4A3028` | Base `#6B4435` | Brilho `#8B5A42`
   - **02 Preto:** Raiz `#141416` | Base `#1F1F24` | Brilho `#383842`
   - **03 Branco Acinzentado:** Raiz `#7A7E85` | Base `#C4C9D1` | Brilho `#E8ECF2`
   - **04 Ruivo:** Raiz `#6B2214` | Base `#A83E24` | Brilho `#D4623B`
   - **05 Loiro:** Raiz `#8C6B2B` | Base `#D9B45B` | Brilho `#F2D680`
