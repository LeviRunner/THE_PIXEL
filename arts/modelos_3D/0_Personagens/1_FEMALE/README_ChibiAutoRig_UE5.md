# Chibi Anime UE5 Auto-Rig v2.0 (`ThePixel_Tools` — 3ds Max 2027)

Sistema nativo de **Rigging Facial & Corporal Automatizado**, **Auto-Skinning por Zonas Anatômicas (BlenRig 6 Architecture)** e **Painel Lateral com Desenho de Rosto Anime Interativo (Face Picker GUI)**, desenvolvido exclusivamente para o **Autodesk 3ds Max 2027** (`MAXScript` moderno + API `pymxs` Python 3) com compatibilidade direta para **Unreal Engine 5 (UE5)**.

---

## 1. Novidades da Arquitetura v2.0 (Inspirada no BlenRig 6)

1. **Desenho de Rosto Anime 2D no Painel Lateral (`Anime Face Picker GUI`):**
   - Dentro do quadro amarelo ao lado da cabeça (`CTRL_UI_Panel_Main`), o script desenha um **rosto anime completo em linhas suaves (`SplineShape`)**.
   - Cada traço do desenho (arcos das sobrancelhas, arcos curvos das pálpebras superiores e inferiores, pupilas, nariz, lábio superior, lábio inferior, cantos da boca e linha do queixo/mandíbula) funciona como um seletor interativo conectado aos ossos 3D via `Position_Script` e `Rotation_Script`.
   - O rosto 3D da personagem fica **100% limpo de poluição visual**, permitindo ver as expressões com perfeição!

2. **Fechamento Esférico Curvo das Pálpebras e Pele Orbital (`eyelid_arc`):**
   - Resolve o problema clássico de afundamento reto no meio do olho: quando você baixa a linha da pálpebra superior (ou sobe a inferior) no painel, o controlador executa uma trajetória curva (`curve_y = -abs(dz) * 0.42`), fazendo os cílios e a pálpebra deslizarem **pela frente da curvatura do globo ocular**.
   - A malha `Head_F` recebe pesos de zona orbital (`FACIAL_L/R_EyelidUpper` e `EyelidLower`), puxando a pele da pálpebra da cabeça junto com `Eyelid_F`, `Eyelashes_F` e `Eye_Inner_Shadow_F` para fechar o olho completamente!

3. **Abertura Real da Boca, Mandíbula e Arcada Dentária (`jaw_arc`):**
   - Separação topológica automática na linha da fenda labial (`mouth_z`):
     - **Lábio Superior e Arcada Dentária Superior (`Teeth_F`):** permanecem ancorados ao crânio (`head` / `FACIAL_C_MouthUpper`).
     - **Lábio Inferior, Queixo, Língua (`Tongue_F`) e Arcada Inferior:** acompanham a rotação e descida da mandíbula (`FACIAL_C_Jaw` / `FACIAL_C_MouthLower`).

4. **Sobrancelhas com Interpolação Gaussiana Contínua:**
   - Os 3 segmentos da sobrancelha (`Inner`, `Mid`, `Outer`) usam pesos Gaussianos sobrepostos na malha `Eyebrows_F` sem quebrar nem marcar a testa da malha `Head_F`.

---

## 2. Estrutura do Repositório

| Caminho | Descrição |
| :--- | :--- |
| [`src/ChibiAutoRig_UE5_Core.py`](file:///C:/Users/levi/Documents/ChibiAutoRig-UE5-3dsMax2027/src/ChibiAutoRig_UE5_Core.py) | Motor Python 3 (`pymxs`) com escaneamento anatômico, geração de ossos `FACIAL_*`, Face Picker Anime e Auto-Skinning BlenRig. |
| [`macros/ChibiAutoRig_UE5_Launcher.ms`](file:///C:/Users/levi/Documents/ChibiAutoRig-UE5-3dsMax2027/macros/ChibiAutoRig_UE5_Launcher.ms) | Lançador direto e MacroScript com seletores rápidos e botões de **Teste de Expressões Instantâneas** (Piscar, Abrir Boca, Sorrir, Sobrancelhas, Reset T-Pose). |
| [`macros/ChibiAutoRig_UE5_Macro.mcr`](file:///C:/Users/levi/Documents/ChibiAutoRig-UE5-3dsMax2027/macros/ChibiAutoRig_UE5_Macro.mcr) | Arquivo de Macro instalável na categoria `ThePixel_Tools` da barra de ferramentas do 3ds Max 2027. |
| `assets/icons/` | Ícones personalizados (`24i`, `32i`, `64i`) da ferramenta. |

---

## 3. Como Executar e Testar no 3ds Max 2027

1. No **3ds Max 2027**, vá em **`Script` > `Executar Script...` (Run Script...)**.
2. Selecione:
   `E:\The_Pixel\arts\modelos_3D\0_Personagens\1_FEMALE\ChibiAutoRig_UE5_Launcher.ms`
3. Clique no botão:
   **`GERAR RIG FACIAL UE5 + PAINEL ANIME + AUTO-SKIN`**
4. Na **Seção 4 (`Testar Expressões Automáticas`)**, clique em:
   - **`Testar Piscar (Fechar Olhos)`**
   - **`Testar Abrir Boca (A)`**
   - **`Testar Sorriso Feliz`**
   - **`Testar Sobrancelha Séria`**
   - **`RESETAR ROSTO PARA POSE NEUTRA (T-POSE)`**
5. Ou clique diretamente em qualquer linha do **Rosto Anime** desenhado dentro do quadro amarelo ao lado da cabeça e mova com a ferramenta **Mover (`W`)**!
