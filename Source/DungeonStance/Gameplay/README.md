# Gameplay — Dungeon Stance & Cast Framework

Sistema central de combate em grupo, construído sobre o **Gameplay Ability System (GAS)**.

- **Dungeon Stance** é o modo de lançamento: uma dungeon instanciada com ondas de inimigos e chefes, jogada por 1 a 5 heróis.
- **Cast Framework** é a camada que padroniza todo "cast" do jogo (ataque rápido, skill da classe, esquiva, habilidades de apoio, procs de cartas) como Gameplay Abilities com custo, recarga e alvo.

## Componentes e dados esperados

| Peça | Tipo | Origem no protótipo |
| --- | --- | --- |
| `UDSAbilitySystemComponent` | `UAbilitySystemComponent` | — |
| `UDSAttributeSet` | `UAttributeSet` | `classes.ts` (`BaseStats`): Vida, Dano, Vel. ataque, Alcance, Mov., Crítico, Armadura, Recurso |
| `UDSClassData` | `UPrimaryDataAsset` | `classes.ts` + `CLASSES.md` (9 classes) |
| `UDSStanceComponent` | `UActorComponent` | Caminho escolhido na run: `critical` / `magic` / `defense` / `support` (`cards.ts`) |
| `UDSCardData` / árvore | `UDataAsset` + `UDataTable` | `cards_*.ts`, `cards_node.ts` |
| `UDSGA_*` | `UGameplayAbility` | Skills de classe, esquiva, 8 habilidades de apoio (`abilities.ts`) |
| `UDSGE_*` | `UGameplayEffect` | Status: sangramento, queimadura, veneno, lentidão, atordoamento, marca, provocação (`status.ts`) |
| `UDSEnemyData` | `UPrimaryDataAsset` | `bestiary.ts` (arquétipos, comportamentos, drops) |
| IA inimiga | StateTree ou Behavior Tree | `bestiary_ai.ts` (melee, swarm, charger, blinker, kiter, ranged, brood) |

## Diretrizes

- Toda habilidade respeita o contexto de party: alvos múltiplos, raio de aliados e sinergias de 1 a 5 jogadores.
- Recargas e custos rodam no servidor (GAS Cooldown/Cost GEs). O cliente só faz predição.
- Números de balanceamento ficam em Data Assets/Data Tables, nunca hardcoded no C++.
- Fórmulas e valores extraídos do protótipo: [`ESPEC_REGRAS_DUNGEON_STANCE.md`](../../../docs/ESPEC_REGRAS_DUNGEON_STANCE.md).
