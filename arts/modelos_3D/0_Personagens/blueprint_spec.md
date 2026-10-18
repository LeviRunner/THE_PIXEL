# Blueprint Specification – T‑Pose (Front & Left) – Chibi Anime Style

## Geral
- **Estilo:** low‑poly chibi anime/cartoon, faces grandes, olhos expressivos com brilhos, pele lisa com leve gradiente, bochechas rosadas.
- **Background:** plano cinza claro com grade fina (0,1 m) para servir de referência de escala no Blender.
- **Polígonos:** um pouco mais detalhados que os modelos base anteriores – aproximadamente **2 500 triângulos** por personagem (inclui detalhes faciais e roupas íntimas).
- **Roupas íntimas:** boxer briefs escuros (masculino) / sutiã esportivo + calcinha (feminino) — sem dobras excessivas, apenas duas camadas de polígonos para drapeado leve.
- **Escala:** altura total do personagem = **1,80 m** (medida do topo da cabeça ao chão). A grade de fundo tem 0,5 m de espaçamento, assim a altura da cabeça corresponde a 12 divisões da grade.
- **Formato das imagens:** orthographic, tamanho **1024 × 1024 px**, 300 dpi, PNG com transparência.

## 1 – Masculino – Front (já gerada)
- **Arquivo:** `masculino_front.jpg`
- **Posição:** frente, braços estendidos horizontalmente ao nível dos ombros, pernas separadas levemente.
- **Rosto:** queixo quadrado suavizado, nariz pequeno, olhos grandes castanhos com íris laranja, sobrancelhas finas, leve blush nas bochechas.
- **Cabelo:** curto, castanho escuro, 3‑4 fios principais.
- **Roupas íntimas:** boxer briefs cinza‑escuro, cintura ligeiramente alta.

## 2 – Masculino – Left (pendente)
- **Visão:** perfil esquerdo, cabeça virada ¾ para mostrar a lateral do rosto e a orelha.
- **Detalhes faciais:** nariz em perfil, orelha visível, olho visto de lado (pupila e íris parcialmente ocultas). 
- **Corpo:** mesma proporção do front; braço esquerdo apontando para a câmera (frente), braço direito atrás do corpo (visível como linha). 
- **Cores:** pele `#FFDBB5`, cabelo `#4A3028`, boxer `#3A3A3A`.
- **Objetivo:** usar como referência de **espessura de membros** e *bone placement* para a cadeia de ossos lateral.

## 3 – Feminino – Front (pendente)
- **Visão:** frente, pose T‑pose exatamente como o masculino, mas com a silhueta feminina.
- **Rosto:** mandíbula suavizada, queixo levemente pontudo, nariz pequeno, olhos grandes verdes com luzes, bochechas levemente rosadas.
- **Cabelo:** curto, liso, castanho claro, franjas curtas que caem sobre a testa.
- **Roupas íntimas:** sutiã esportivo cinza‑escuro + calcinha de corte alto (tipo “boyshort”).
- **Altura:** mesma 1,80 m; quadris mais largos (0,46 m) que o masculino (0,38 m).
- **Paleta:** pele `#FFDBB5`, cabelo `#8B5A2B`, sutiã/calcinha `#3A3A3A`.

## 4 – Feminino – Left (pendente)
- **Visão:** perfil esquerdo, mostrando a lateral da cabeça, sobrancelha, nariz e a curva da cintura.
- **Detalhes de roupa:** sutiã visto de lado (corte em “V”), calcinha visível na lateral da perna.
- **Cabelo:** a franja caindo de forma assimétrica, orelha parcialmente coberta.
- **Objetivo:** base para o rig de costelas e quadril, além da modelagem da roupa modular.

---
### Uso no Blender
1. **Importar a blueprint** como plano de fundo (Viewport → Background Images).
2. **Alinhar a grade** da blueprint ao grid do Blender (1 m = 1 unit).
3. **Modelar a malha** começando com o torso (cylinder → sculpt), depois ajustar a cabeça conforme a blueprint frontal.
4. **Separar a roupa íntima** em um objeto próprio para que as armaduras possam ser adicionadas como mesh filhos.
5. **Salvar** o `.blend` como `Base_Masculina.blend` ou `Base_Feminina.blend` dentro da pasta `0_Personagens`.

---
### Próximos passos (automatizados)
- Quando a cota de geração de imagens for restaurada (cerca de 14:40 h), irei gerar os arquivos de imagem pendentes:
  - `masculino_left.jpg`
  - `feminino_front.jpg`
  - `feminino_left.jpg`
- Assim que estiverem prontos, eles serão copiados automaticamente para `E:\The_Pixel\arts\modelos_3D\0_Personagens\blueprints\`.
- Também tentarei gerar novamente a referência da **coruja** e salvá‑la como `coruja.jpg` na mesma pasta.

**Atenção:** as imagens ainda não geradas serão enviadas assim que o timer disparar.
