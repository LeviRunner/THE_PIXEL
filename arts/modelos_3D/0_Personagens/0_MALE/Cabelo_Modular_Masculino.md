# Guia Técnico e Checklist Passo a Passo: Cabelo Modular (`Hero_Chibi_Male`)

> **Objetivo:** Guia prático com **checklists em tabelas (linhas e colunas)** para você ir marcando `[x]` em cada etapa da criação dos cabelos modulares masculinos (ajuste de Tamanho, Cor via Máscara RGB e Rigging).
> **Blueprints de Modelagem (1 Lado + Simetria + Crescimento):**
> - [`Blueprint_Hair_Male_01_Half_Symmetry.jpg`](./Blueprint_Hair_Male_01_Half_Symmetry.jpg) — Vista Frontal e Perfil Esquerdo (Eixo `X=0` + 4 Partes Coloridas Simplificadas)
> - [`Blueprint_Hair_Male_02_Parts_Growth.jpg`](./Blueprint_Hair_Male_02_Parts_Growth.jpg) — Peças de 1 Lado na Mesma Proporção (Corte em Costuras Vermelhas + Crescimento Curto $\rightarrow$ Médio $\rightarrow$ Longo)

---

## 0. Método de Modelagem Simplificado de 1 Lado (Estilo Vídeo Anime Quad-Strip)
1. **Meia-Calota Base (`Step 1`):** Comece com metade de uma esfera/calota low-poly sobre o lado esquerdo da cabeça (`Head_M`), mantendo o topo (`Apex`) soldado no centro (`X=0`).
2. **Cortes Verticais (`Red Split Seams`):** Selecione as arestas verticais da metade da altura para baixo e aplique **Split / Break** (ou *Chamfer* fino sem faces internas) para separar em **3 tiras principais de 1 lado**:
   - **`[A] Front Bangs Strip` (Azul Claro):** 3 colunas de quads formando metade da franja.
   - **`[B] Side Temple Strip` (Laranja Coral):** 2 colunas de quads na frente da orelha (costeleta/lateral).
   - **`[C] Back Nape Strip` (Roxo Suave):** 4 colunas de quads na nuca com **loops horizontais uniformes** para permitir esticar o comprimento.
3. **Simetria + Quebra de Simetria Visual:** Aplique o modificador **`Symmetry`** no eixo `X=0`. Na UE5, a física independente dos ossos (`hair_l` / `hair_r` / `hair_back`) durante o movimento + 1 mecha central levemente deslocada no *Edit Poly* acima do *Symmetry* eliminam 100% qualquer aparência artificial de espelhamento.

---

## 1. Checklist Passo a Passo de Execução (Para Cada Cabelo Novo)
> Siga as etapas de **01 a 10** na ordem abaixo para produzir o Cabelo Base Masculino:

| Passo | Etapa de Produção | O Que Fazer no 3ds Max / UE5 | Base 01 (`Hair_01_M`) | Estilo 02 (`Hair_02_M`) | Estilo 03 (`Hair_03_M`) | Estilo 04 (`Hair_04_M`) | Estilo 05 (`Hair_05_M`) |
| :---: | :--- | :--- | :---: | :---: | :---: | :---: | :---: |
| **01** | **Modelar Base Topo/Nuca** | Modelar a calota e mechas traseiras (`300–600 tris`) usando `Head_M` como manequim | [ ] | [ ] | [ ] | [ ] | [ ] |
| **02** | **Separar Franja (`Front`)** | Destacar a franja (`Hair_Front_XX_M`) para continuar visível com capuz/chapéu | [ ] | [ ] | [ ] | [ ] | [ ] |
| **03** | **Alinhar Pivô** | Posicionar o Pivô exatamente no centro do osso `head` (ou `0,0,0` do Socket) | [ ] | [ ] | [ ] | [ ] | [ ] |
| **04** | **Abrir UV na Vertical** | Aplicar `Unwrap UVW`, alinhar raiz no topo e ponta embaixo e salvar `Hair_XX_M.uvw` | [ ] | [ ] | [ ] | [ ] | [ ] |
| **05** | **Aplicar Máscara RGB** | Vincular a máscara `T_Hair_Mask_RGB.png` (R=Degradê, G=Brilho, B=Mecha 2) | [ ] | [ ] | [ ] | [ ] | [ ] |
| **06** | **Criar Morph Curto** | Duplicar malha, encolher pontas e adicionar ao modificador *Morpher* (`Morph_Length_Short`) | [ ] | [ ] | [ ] | [ ] | [ ] |
| **07** | **Criar Morph Longo** | Duplicar malha, esticar pontas e adicionar ao *Morpher* (`Morph_Length_Long`) | [ ] | [ ] | [ ] | [ ] | [ ] |
| **08** | **Criar Morph Volume** | Inflar levemente as mechas laterais e adicionar (`Morph_Volume_Spiky`) | [ ] | [ ] | [ ] | [ ] | [ ] |
| **09** | **Rigging / Skin** | Aplicar *Skin* no osso `head` (e ossos de ponta `hair_top`/`hair_back` se houver) | [ ] | [ ] | [ ] | [ ] | [ ] |
| **10** | **Exportar FBX & Testar UE5** | Exportar com *Morph Targets* marcados e testar as 5 cores (`Castanho`, `Preto`, `Branco`, `Ruivo`, `Loiro`) | [ ] | [ ] | [ ] | [ ] | [ ] |

---

## 2. Checklist de Produção por Sub-Peça do Cabelo (As 6 Bases)

| Sub-Peça Modular | Arquivo `.uvw` | 1. Malha | 2. UVMap | 3. Textura Mask | 4. Rig (Osso/Morph) | 5. Animações | 6. Efeitos (Shader) |
| :--- | :--- | :---: | :---: | :---: | :---: | :---: | :---: |
| **Cabelo Base 01 (Topo/Nuca)** | `Hair_01_M.uvw` | [x] | [x] | [x] | [x] | [ ] | [ ] |
| **Franja Modular 01 (Testa)** | `Hair_Front_01_M.uvw` | [ ] | [ ] | [ ] | [ ] | [ ] | [ ] |
| **Hair Cap 01 (Versão Elmo/Capuz)**| `Hair_Cap_01_M.uvw` | [ ] | [ ] | [ ] | [ ] | [ ] | [ ] |
| **Cabelo Estilo 02 (Partido)** | `Hair_02_M.uvw` | [ ] | [ ] | [ ] | [ ] | [ ] | [ ] |
| **Cabelo Estilo 03 (Samurai)** | `Hair_03_M.uvw` | [ ] | [ ] | [ ] | [ ] | [ ] | [ ] |
| **Cabelo Estilo 04 (Moicano)** | `Hair_04_M.uvw` | [ ] | [ ] | [ ] | [ ] | [ ] | [ ] |
| **Cabelo Estilo 05 (Cacheado)** | `Hair_05_M.uvw` | [ ] | [ ] | [ ] | [ ] | [ ] | [ ] |

---

## 3. Checklist da Máscara de Textura dos UVs de Cabelo (`0_MALE`)

| Arquivo `.uvw` | Canal R (Raiz -> Ponta) | Canal G (Faixa de Brilho) | Canal B (Mecha Bicolor) | 1. UV Vertical | 2. Mask PNG Aplicada | 3. sRGB Desativado (UE5) | 4. 5 Cores Testadas |
| :--- | :--- | :--- | :--- | :---: | :---: | :---: | :---: |
| **`Hair_01_M.uvw`** | Preto no topo (Raiz) -> Branco embaixo (Ponta) | Faixa branca horizontal a 35% do topo | Branco apenas nas mechas de destaque | [ ] | [ ] | [ ] | [ ] |
| **`Hair_Front_01_M.uvw`** | Preto na raiz da testa -> Branco na ponta da franja | Faixa branca alinhada à altura da cabeça | Branco opcional em 1 mecha frontal | [ ] | [ ] | [ ] | [ ] |
| **`Hair_Cap_01_M.uvw`** | Gradiente suave da nuca/costeletas | Sem brilho forte (fica sob o elmo) | Preto total (`0`) | [ ] | [ ] | [ ] | [ ] |

---

## 4. Resumo Rápido da Lógica Técnica (Para Consulta)

1. **Ajuste de Tamanho no Painel de Personalização:**
   - O slider da UI controla os Morph Targets `Morph_Length_Short` (`-1.0`) até `Morph_Length_Long` (`+1.0`) e `Morph_Volume_Spiky`, alterando o comprimento das mechas sem descolar a raiz do crânio.
2. **Ajuste de Cor Dinâmica (5 Cores Reais Iniciais):**
   - Usa a textura universal [`../T_Hair_Mask_RGB.png`](../T_Hair_Mask_RGB.png).
   - **01 Castanho (Padrão):** Raiz `#4A3028` | Base `#6B4435` | Brilho `#8B5A42`
   - **02 Preto:** Raiz `#141416` | Base `#1F1F24` | Brilho `#383842`
   - **03 Branco Acinzentado:** Raiz `#7A7E85` | Base `#C4C9D1` | Brilho `#E8ECF2`
   - **04 Ruivo:** Raiz `#6B2214` | Base `#A83E24` | Brilho `#D4623B`
   - **05 Loiro:** Raiz `#8C6B2B` | Base `#D9B45B` | Brilho `#F2D680`
