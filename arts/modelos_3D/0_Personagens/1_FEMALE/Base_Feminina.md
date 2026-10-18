# Base Mesh: Personagem Feminino

**Contexto:** O modelo base (basemesh) será o esqueleto por baixo das armaduras. Para o sistema modular (paperdoll), a silhueta feminina deve se diferenciar principalmente pela largura dos ombros, formato da cintura e quadril. O visual ainda precisa se manter isométrico e low poly.

## Proporções e Escala
* **Altura Total:** `1.70m` a `1.75m` (Levemente menor que o masculino, ou `1.80m` caso prefira padronizar hitboxes).
* **Estilo:** Low poly flat-shaded.
* **Cabeça:** Proporção levemente exagerada. Rosto um pouco mais afilado e pescoço ligeiramente mais fino.
* **Tronco:** Ombros mais estreitos que o masculino. Cintura mais fina e quadris ligeiramente mais largos.
* **Mãos e Pés:** Mãos em formato de bloco, um pouco mais delicadas, mas grossas o bastante para segurar as mesmas espadas e picaretas.

## Preparação para Armaduras Modulares
As armaduras criadas a partir dessa base seguirão o mesmo princípio de "inflar a malha" (+5% ao longo das normais):

1. **Cabeça (Head):** Se o cabelo for adicionado, faça dele um objeto modular separado (hair helmet), para não bugar quando usar um elmo completo.
2. **Tronco (Chest/Torso):** Deve ter o volume necessário para que, ao inflar, crie couraças justas, porém confortáveis.
3. **Braços (Arms) e Pernas (Legs):** Manter os mesmos loops de corte (edge loops) da versão masculina nos ombros, cotovelos e joelhos. Assim as proporções das grevas (botas) e braçadeiras podem ser reaproveitadas apenas diminuindo a escala no eixo X e Y.

## Poses para Rigging
Mantenha a personagem em **A-Pose** (braços descansando num ângulo de 45 graus) ou **T-Pose** durante a modelagem. Isso evitará que as texturas estiquem nos ombros quando as animações (como correr e forjar) forem tocadas no motor do jogo.
