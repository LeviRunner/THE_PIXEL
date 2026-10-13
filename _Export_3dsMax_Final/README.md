# 3ds Max — Exportação Final de Assets

Esta pasta guarda **apenas** assets prontos e validados vindos do 3ds Max, para importação direta na Unreal Engine 5.
Concepts e arquivos de trabalho (`.max`, rascunhos) ficam em `/arts` — **não** mover nada de lá para cá sem estar validado.

## Subpastas

| Pasta | Conteúdo |
| --- | --- |
| `Characters/` | Heróis das 9 classes, monstros e chefes (Skeletal Meshes + animações) |
| `Environments/` | Módulos de dungeon/arena: pisos, paredes, rampas, platôs |
| `Items/` | Armas, armaduras, consumíveis do Cinto de Batalha, drops |
| `Props/` | Estações de craft, baús, totens, estandartes, decoração |

## Requisitos de exportação

- **Unidades:** 1 unidade do 3ds Max = 1 centímetro (padrão Unreal). No export FBX, deixar *Automatic* desligado e *Scene units converted to: Centimeters*.
- **Eixos:** Z para cima (Z-Up). Frente do personagem olhando para **+X** na Unreal (no Max: frente para −Y antes de exportar, conferir com um teste de import).
- **Pivot / origem:**
  - Personagens: na base dos pés (centro X/Y, Z = chão), no ponto (0,0,0) da cena.
  - Items/Props: no centro geométrico ou no ponto lógico de pega/encaixe (empunhadura da arma, base do prop).
- **Formato:** `.fbx` (FBX 2020 ou superior), malha limpa, sem vértices duplicados, transformações resetadas (*Reset XForm* antes de exportar).
- **UVs:** canal 1 = textura; canal 2 = lightmap (só para Static Meshes de cenário que usem luz estática).
- **Escala de referência do jogo:** herói ≈ 180 cm de altura. O protótipo legado usava 40 px = 1 m (1 px = 2,5 cm); alcances e raios da especificação de regras já estão convertidos para cm.

## Nomenclatura (prefixo obrigatório)

| Prefixo | Tipo | Exemplo |
| --- | --- | --- |
| `SK_` | Skeletal Mesh | `SK_Barbaro.fbx` |
| `SM_` | Static Mesh | `SM_Bigorna_Reforcada.fbx` |
| `A_` | Animação (FBX só com a animação) | `A_Barbaro_Redemoinho.fbx` |
| `MAT_` | Material | `MAT_Pedra_Dungeon` |
| `TEX_` | Textura (+ sufixo `_D` cor, `_N` normal, `_ORM` oclusão/rugosidade/metal) | `TEX_Barbaro_D.png` |

## Animação

O fluxo de animação (vídeo IA → 3ds Max → UE5) está em [`video_para_animacao.md`](../docs/video_para_animacao.md). Animações limpas no Max são exportadas para `Characters/<Personagem>/Animations/` com o prefixo `A_`, usando o mesmo esqueleto do `SK_` correspondente.
