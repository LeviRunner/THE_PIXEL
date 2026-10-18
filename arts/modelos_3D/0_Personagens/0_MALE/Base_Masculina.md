# Base Mesh: Personagem Masculino

**Contexto:** O modelo base (basemesh) será o esqueleto por baixo das armaduras. Para um jogo com visual isométrico e sistema de "paperdoll" (onde as peças de armadura são encaixadas em tempo real), a base precisa ser volumosa (chunky) para que as armaduras também ganhem volume e fiquem legíveis.

## Proporções e Escala
* **Altura Total:** `1.80m` (Z = 1.8 no Blender).
* **Estilo:** Low poly flat-shaded (sem smooth shading, ou com edge split).
* **Cabeça:** Levemente maior que a proporção realista humana (ajuda a dar foco em elmos e capuzes).
* **Tronco:** Ombreiras largas e cintura em formato "V" suave, mas sem exagerar nos músculos (afinal, a armadura cobrirá o corpo).
* **Mãos e Pés:** Mãos em formato de bloco (sem dedos individuais super definidos, apenas o polegar separado) para facilitar o encaixe das armas. Pés como botas simples.

## Preparação para Armaduras Modulares
Para que as armaduras sirvam de forma fácil sem *clipping* (atravessar a malha), a modelagem da base deve seguir blocos anatômicos limpos:

1. **Cabeça (Head):** Pescoço separado visualmente da cabeça.
2. **Tronco (Chest/Torso):** Volume simples, para que o "Peitoral" (Armor Chestplate) seja apenas essa mesma malha de tronco escalada em +5% no eixo Normal.
3. **Braços (Arms):** Delimitar cortes horizontais nos ombros, cotovelos e punhos. 
4. **Pernas (Legs):** Delimitar cortes horizontais na virilha, joelhos e calcanhares.

## Poses para Rigging
Ao modelar no Blender/3ds Max, mantenha o personagem em **A-Pose** ou **T-Pose** (Braços abertos, pernas levemente separadas). Isso facilitará o *Skinning* automático quando você for ligar os ossos do seu `ModularCharacter` e também garante que qualquer armadura modelada sobre essa base não distorça nas axilas e na virilha.

---

## ✅ Status de Modelagem: Concluído
- [x] **Base Masculina Concluída (3ds Max)**:
  - **Polígonos:** ~4.530 polígonos
  - **Vértices:** ~3.668 vértices
  - **Proporção:** Chibi Cartoon (Cabeça volumosa, corpo estilizado)
  - **Olhos:** Sistema modular de cavidade + íris curva + fitas de cílios e sobrancelhas
  - **Mãos e Pés:** Mãos estilizadas em bloco anatômico ideal para segurar armas (props)

