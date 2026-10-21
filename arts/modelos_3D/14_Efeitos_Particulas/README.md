# Pipeline de Animação Automatizada (Blender + AccuRIG) & Sistema de Partículas Mesh (Niagara UE5)
> **Projeto:** Dungeon Stance (Chibi Cartoon / Dungeon Crawler Soulslike)  
> **Motor:** Unreal Engine 5.5 / 5.8 (MeuProjeto)  
> **Rigging de Origem:** AccuRIG T-Pose Skeletal Mesh (`Hero_Chibi_Male_RIG.fbx`)  
> **Estilo de Combate:** Soulslike Chibi (sem cambalhotas, foco em esquiva rápida/dash, parry, bloqueio em tempo real e peso nos golpes).

---

## 1. O Prompt Mestre para o Script de Automação no Blender

Copie e use este prompt para gerar o script Python (`generate_animations.py`) que roda diretamente no Blender via linha de comando (`blender --background --python generate_animations.py`) ou na aba Scripting:

```markdown
Você é um Technical Animator especialista em Blender Python (bpy) e integração com Unreal Engine 5.
Crie um script Python automatizado completo para o Blender 4.x / 5.x que importe um modelo esquelético vindo do AccuRIG, configure um objeto proxy na mão e gere uma biblioteca completa de animações FBX exportáveis diretamente para a Unreal Engine 5.

### 1. Parâmetros e Diretórios do Projeto:
- Modelo Esquelético Base: "E:/The_Pixel/arts/modelos_3D/0_Personagens/0_MALE/Hero_Chibi_Male_RIG.fbx"
- Pasta de Saída dos FBX de Animação: "E:/The_Pixel/arts/modelos_3D/15_Animacoes/"
- Convenção de Nomes dos Ossos do AccuRIG:
  - Root: "root"
  - Pelvis: "pelvis"
  - Mão Direita: "hand_r"
  - Mão Esquerda: "hand_l"
  - Pés: "foot_r", "foot_l"

### 2. Configuração do Objeto Proxy na Mão (Arma Auxiliar):
- Criar um cilindro low-poly simples (tipo bastão/empunhadura, raio 0.03m, altura 0.7m, 8 vértices).
- Anexar o cilindro ao osso "hand_r" via constraint 'Child Of' para visualizar a orientação de corte, ponta e guarda durante a criação dos keyframes.
- O cilindro é apenas visualizador de rig no Blender e deve ser desmarcado da exportação final de animação (exportar somente Armature com 'Only Deform Bones' e 'Bake Animation').

### 3. Padrão Estilístico das Animações:
- Estilo: Chibi Cartoon Dungeon Crawler Soulslike.
- Peso e Ritmo: Movimentos com antecipação clara, impacto firme (hold de 2 a 3 frames no ponto de impacto) e recuperação responsiva.
- REGRA OBRIGATÓRIA DE ESQUIVA: NÃO FAZER CAMBALHOTA / ROLL. A esquiva deve ser um "Combat Dash / Quickstep" terrestre com o tronco inclinado, pés deslizando e mão firme na arma, compatível com o corpo compacto e cabeçudo do chibi.
- Sistema de Defesa Soulslike:
  - Posição de Guarda / Defesa Ativa (Hold Block).
  - Bloqueio com Impacto (Block Hit Reaction).
  - Parry Preciso (Deflexão com faísca e abertura do inimigo).

### 4. Biblioteca de Animações a Gerar e Exportar:

#### A. Locomoção Básica (Root Motion & In-Place):
1. Anim_Hero_Idle (30 fps, loop 60 frames, respiração e peso sutil nas pernas)
2. Anim_Hero_Walk_Fwd (loop 30 frames, passos rítmicos)
3. Anim_Hero_Run_Fwd (loop 24 frames, corrida ágil com corpo inclinado)
4. Anim_Hero_Jump_Start (8 frames, compressão e impulso)
5. Anim_Hero_Jump_Loop (12 frames, flutuação no ápice)
6. Anim_Hero_Jump_Land (10 frames, amortecimento suave nos joelhos)
7. Anim_Hero_Dodge_Fwd (18 frames, quickstep frontal rápido)
8. Anim_Hero_Dodge_Bwd (18 frames, backstep de recuo tático)
9. Anim_Hero_Dodge_Left / Anim_Hero_Dodge_Right (16 frames, strafe evasivo)

#### B. Defesa e Reação Soulslike:
10. Anim_Hero_Guard_Start (6 frames, transição para guarda alta)
11. Anim_Hero_Guard_Loop (loop 30 frames, postura firme com arma/escudo levantado)
12. Anim_Hero_Guard_Impact (12 frames, empurrão para trás ao absorver ataque pesado)
13. Anim_Hero_Parry_Deflect (14 frames, chicotada rápida da mão/escudo para desviar golpe no frame 4-6)
14. Anim_Hero_Hit_Fwd / Anim_Hero_Hit_Bwd (12 frames, reação a dano sem cair)

#### C. Lista Exaustiva de Armas (Ataque Normal e Especial para Cada Uma):
Gere animações nomeadas exatamente pelo item para permitir mapeamento dinâmico na UE5:
- Espadas (Sword):
  - Anim_Atk_Sword_Slash_01 (Corte horizontal rápido)
  - Anim_Atk_Sword_Slash_02 (Corte diagonal descendente)
  - Anim_Atk_Sword_Special_Thrust (Estocada de perfuração com avanço)
- Adagas (Dagger):
  - Anim_Atk_Dagger_Slash_01 (Corte duplo em X)
  - Anim_Atk_Dagger_Special_Backstab (Golpe perfurante crítico de emboscada)
- Maças (Mace):
  - Anim_Atk_Mace_Smash_01 (Pancada lateral de concussão)
  - Anim_Atk_Mace_Special_GroundSlam (Golpe com salto e impacto no chão)
- Machados de Guerra (Battleaxe):
  - Anim_Atk_Battleaxe_Chop_01 (Machadada vertical pesada)
  - Anim_Atk_Battleaxe_Special_Cleave (Giro em 360 graus varrendo a área)
- Lanças (Spear):
  - Anim_Atk_Spear_Thrust_01 (Estocada rápida de longo alcance)
  - Anim_Atk_Spear_Special_VaultKick (Apoio na haste com chute frontal duplo)
- Alabardas (Halberd):
  - Anim_Atk_Halberd_Sweep_01 (Corte em arco aberto)
  - Anim_Atk_Halberd_Special_OverheadSplit (Golpe descendente esmagador)
- Bestas de Mão (Hand Crossbow):
  - Anim_Ranged_Crossbow_Shoot (Disparo com recuo seco e mira)
  - Anim_Ranged_Crossbow_Reload (Puxada do trilho e recarga de virote)
- Arcos Recurvos (Bow):
  - Anim_Ranged_Bow_Aim_Hold (Puxada da corda tensionada)
  - Anim_Ranged_Bow_Release (Soltura da flecha e relaxamento da corda)
- Varinhas & Orbes Mágicos (Wand / Orb):
  - Anim_Magic_Cast_QuickBolt (Gesto rápido do pulso disparando projétil)
  - Anim_Magic_Cast_ChannelLoop (Canalização contínua com orbe flutuando na mão esquerda)
- Cajados de Batalha (Staff):
  - Anim_Magic_Staff_GroundStrike (Bate da ponta do cajado no chão gerando onda arcana)
  - Anim_Magic_Staff_OrbWave (Varredura mística horizontal liberando raio elemental)

### 5. Configuração da Exportação FBX (bpy.ops.export_scene.fbx):
- use_selection = True (apenas o esqueleto selecionado)
- global_scale = 1.0 (unidades em metros/centímetros sincronizadas com a Unreal)
- bake_anim = True, bake_anim_use_nla_strips = False, bake_anim_use_all_actions = False
- add_leaf_bones = False (evita a criação de ossos fantasmas desnecessários)
- primary_bone_axis = 'Y', secondary_bone_axis = 'X', axis_forward = '-Z', axis_up = 'Y'
```

---

## 2. Sistema de Partículas e Efeitos Visuais (VFX Mesh / Niagara)

No estilo **Chibi Cartoon Low Poly**, os efeitos visuais **não devem ser sprites de fumaça fotorrealista 2D**, mas sim **pequenos modelos 3D volumétricos (Static Meshes)** emitidos e animados pelo sistema **Niagara** da Unreal Engine 5.

### Lista de Modelos Mesh VFX a Modelar no 3ds Max:
Local de salvamento dos modelos FBX: `E:\The_Pixel\arts\modelos_3D\14_Efeitos_Particulas\Meshes\`

| Nome do Asset Mesh | Polígonos | Formato Visual | Uso no Jogo |
| :--- | :---: | :--- | :--- |
| `SM_VFX_Smoke_Bubble_01` | 32 a 48 tris | Esfera deformada estilizada com facetas 'low poly' suaves | Bolinhas de fumaça saindo da chaminé da casa e forja |
| `SM_VFX_Smoke_Bubble_Small_02` | 24 tris | Pequena bolha irregular para puffs de dissipação | Final de rastro de fumaça e sopros de impacto |
| `SM_VFX_Floating_Pebble_01` | 16 a 28 tris | Fragmento rochoso pontiagudo com quinas marcadas | Pedrinhas orbitando a ilha flutuante e o Altar de TP |
| `SM_VFX_Floating_Pebble_02` | 20 tris | Rocha facetada plana | Pedrinhas menores com rotação lenta aleatória |
| `SM_VFX_Slash_Arc_Mesh_01` | 24 tris | Fatia geométrica curva em arco com espessura variável | Trilha de corte da espada/machado (Weapon Trail Mesh) |
| `SM_VFX_Spark_Diamond_01` | 12 tris | Losango / estrela de 4 pontas 3D espessa | Faíscas de parry, impacto de lâminas e mineração |
| `SM_VFX_Mana_Rune_Particle_01` | 8 tris | Placa rúnica fina hexagonal com chanfro | Símbolos mágicos flutuando saindo do portal e do altar |
| `SM_VFX_Water_Drop_01` | 18 tris | Gota estilizada com topo cônico e base esférica | Pingos de estalactites na Gruta e respingos da roda d'água |

---

## 3. Passo a Passo de Produção: Do 3ds Max ao Niagara na UE5

### Passo 1: Modelar as Meshes de Efeito no 3ds Max
1. Crie uma **GeoSphere** com 2 segmentos (para ficar bem facetada, cerca de 32 a 48 polígonos).
2. Puxe levemente alguns vértices para que não seja uma esfera perfeita redonda de computador, mas uma nuvenzinha fofa cartoon.
3. Centralize o pivô na origem `(0, 0, 0)`.
4. Aplique **Reset XForm** e converta para **Editable Poly**.
5. Exporte como FBX: `SM_VFX_Smoke_Bubble_01.fbx` com escala em Centímetros (1.0).

### Passo 2: Criar o Material Estilizado na Unreal Engine 5
1. Crie um material `M_VFX_Cartoon_Solid`:
   - Shading Model: **Default Lit** (recebe sombras bonitas da luz do sol) ou **Unlit** com Fresnel para borda brilhante.
   - Base Color: Parâmetro de cor (Branco para fumaça, Laranja para faísca, Ciano para mana).
   - Opcional: Adicione um nó **Particle Color** multiplicado na cor para que o Niagara controle o fade-out e desbotamento.

### Passo 3: Configurar o Emissor Niagara na UE5
1. Clique com o botão direito no Content Drawer -> **FX** -> **Niagara System** -> selecione o template *Simple Sprite Burst* ou *Fountain*.
2. Exclua o módulo `Sprite Renderer` (pois não queremos sprites 2D).
3. Adicione o módulo **`Mesh Renderer`**:
   - No campo **Particle Mesh**, atribua a sua malha: `SM_VFX_Smoke_Bubble_01`.
4. No módulo **Initialize Particle**:
   - `Lifetime`: 2.0 a 3.5 segundos.
   - `Mesh Scale`: Vector aleatório entre `0.3` e `1.2`.
5. No módulo **Particle Update**:
   - Adicione **Scale Mesh Size by Speed** ou uma curva de **Scale Mesh Size** que começa pequena (`0.1`), infla rápido (`1.0`) e encolhe lentamente até sumir (`0.0`).
   - Adicione **Add Velocity** com Z positivo (`+120 cm/s`) para subir lentamente como chaminé.
   - Adicione **Vortex Force** suave ou **Curl Noise Force** para fazer as bolhas dançarem de leve no ar.

---

## 4. O Sistema de Pedras Flutuantes Orbitantes (Ilha da Cidadela)

Para as pedrinhas que flutuam ao redor da ilha pessoal e do grande portal:
1. **Mesh Base**: Usamos a malha de pedra que você já modelou para a ilha (`SM_VFX_Floating_Pebble_01`).
2. **Sistema Niagara (`NS_Floating_Island_Pebbles`)**:
   - `Spawn Rate`: 8 a 15 partículas ativas simultâneas.
   - `Lifetime`: 15 a 25 segundos (longa duração).
   - `Shape Location`: **Torus** ou **Box** largo em volta da borda inferior da ilha.
   - `Point Attraction Force`: Atração gravitacional suave em direção ao centro da ilha.
   - `Vortex Force`: Rotação orbital lenta em torno do eixo Z da ilha.
   - `Mesh Orientation`: Rotação angular contínua muito sutil (`Random Rotation Rate`), dando a impressão mágica de gravidade zero.
