# UI — Interfaces para grupos de 1 a 5 jogadores

Widgets em **UMG/CommonUI**, com a lógica em C++ (classes base `UDS*Widget`) e o layout em Blueprints.

## Telas e elementos esperados

- **HUD da party:** até 5 quadros de aliados (vida, recurso, status, "caído"), sem quebrar layout com 1 jogador.
- **Barra de casts:** ataque, skill da classe, esquiva e até 4 habilidades de apoio (Q/E/R/F) com recarga e cargas.
- **Escolha de cartas** ao subir de nível do time: 20 s após cada onda limpa (`CARD_PICK_SEC` no protótipo).
- **Indicador de onda/chefe**, nível e XP do time.
- **Lobby/seleção:** classe, caminho (stance), tamanho da party (Solo, Dupla, Trio, Esquadrão de 4, Grupo de 5).
- **Ficha do herói, inventário (Cofre, Mochila, Cinto de Batalha) e Espólio** de fim de run.

## Diretrizes

- Suporte completo a gamepad (o protótipo já usava RT/RB/B/A e lock-on no R3).
- A UI só lê estado replicado (GameState/PlayerState/atributos GAS); nunca decide regra.
- Referência visual e fluxo dos menus antigos: `Godot_Legacy_Reference/scripts/ui/`.
