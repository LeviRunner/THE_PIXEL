# Coruja (Familiar de Invocação, nível 25)

**Referência no PLANO.md:** Invocação nível 25: *Coruja (coleta penas/ervas)*. Rig da família **R8 Alado** (asas com 2 ossos).
**Estilo:** low poly facetado (flat shading, arestas visíveis), igual às bases de personagem em `0_Personagens`. Visual fofo/estilizado baseado no print da coruja do jogo: olhos grandes laranja, tufos de orelha, corpo marrom e peito bege pintado.

## Escala (personagem = 1,80 m)
| Parte | Medida |
|---|---|
| Altura total (pousada) | **0,45 m** (exagerada para ler bem no isométrico; uma coruja real teria ~0,35 m) |
| Largura do corpo | 0,28 m |
| Profundidade do corpo | 0,24 m |
| Envergadura (asas abertas) | 1,00 m |
| Cabeça (diâmetro) | 0,20 m (≈ 45% da altura, estilo cabeçudo) |
| Olhos (diâmetro cada) | 0,07 m |
| Tufos de orelha | 0,06 m de altura |
| Pés/garras | 0,05 m |
| Pivot | entre os pés, no chão (y = 0), frente para −Z |

## Partes para modelar (malhas separadas ou grupos de vértices)
- [ ] **Corpo**: forma de ovo/pera, 8 a 10 lados no perfil. Mais largo embaixo.
- [ ] **Cabeça**: quase uma esfera achatada de 8 a 12 lados, levemente fundida ao corpo (pescoço quase invisível).
- [ ] **Disco facial**: dois círculos rasos ao redor dos olhos, em tom mais claro, formando um "coração" com uma ponta para baixo.
- [ ] **Olhos**: esferas de baixa resolução (8 lados). Íris laranja, pupila preta e um ponto de brilho. Separe a malha para animar piscadas com shape key.
- [ ] **Bico**: pirâmide pequena de 3 a 4 faces, curvada para baixo, cinza escuro ou amarelo opaco.
- [ ] **Tufos de orelha**: 2 cones achatados com 3 a 4 faces, inclinados para fora.
- [ ] **Peito**: área bege com manchas escuras em "V" (textura ou vertex color). Pode ter 2 ou 3 fileiras de penas como degraus na malha.
- [ ] **Asas** (cada uma):
    - [ ] Fechadas: placa encostada na lateral do corpo, cobrindo até a cauda.
    - [ ] Abertas: 3 segmentos (braço, antebraço, mão), com 4 a 5 "dedos" de penas primárias separados na ponta.
    - [ ] Camadas: penas de cobertura (curtas, em cima) + penas de voo (longas, embaixo).
- [ ] **Cauda**: leque curto de 3 a 5 penas, quase escondido pelas asas fechadas.
- [ ] **Pés**: 2 dedos para frente e 2 para trás (padrão de coruja), com garras pretas em cone.

## Paleta sugerida
- Marrom escuro (costas/asas): `#5A3A28`
- Marrom médio (cabeça/corpo): `#7A4E33`
- Bege do peito: `#C9A27A`, com manchas `#6B4430`
- Disco facial: `#9C7051`
- Olhos: íris `#E8902A`, pupila `#1A1A1A`
- Bico/garras: `#3A3A3A`

## Rig (R8 Alado)
`root > body > head` · `body > wing_L_01 > wing_L_02` (o mesmo para R) · `body > tail` · `body > leg_L/leg_R`.

## Animações mínimas
idle (balança e gira a cabeça), piscar, voar (ciclo), planar, pousar, coletar (bica o chão).

## Orçamento de polígonos
~800 a 1.500 tris. Os olhos e a silhueta das asas são o que mais importa na câmera isométrica.
