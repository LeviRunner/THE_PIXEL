# Dungeon Stance — Unreal Engine 5

Roguelike cooperativo de ação em dungeons instanciadas para **1 a 5 jogadores** (Solo, Dupla, Trio, Esquadrão de 4, Grupo de 5).
Cada herói entra na dungeon com uma **Stance** (Crítico, Arcana, Defesa ou Apoio) e evolui durante a run com cartas da sua árvore, enfrentando 10 ondas com chefes nas ondas 5 e 10.

Primeiro lançamento: o modo **Dungeon Stance**. Se tiver aceitação da comunidade, o projeto evolui para **MMORPG**.

**Stack:** Unreal Engine 5 (C++ + Blueprints + Gameplay Ability System) · 3ds Max · backend de conta a decidir (Nakama ou EOS/Steam).

## Documentos

| Documento | Conteúdo |
| --- | --- |
| [GUIA_EQUIPE.md](GUIA_EQUIPE.md) | Como o grupo usa Git, GitHub e Git LFS (o que sobe, travas de arquivos, rotina) |
| [PLANO_DUNGEON_STANCE_UE5.md](docs/PLANO_DUNGEON_STANCE_UE5.md) | Plano de execução, decisões técnicas e roteiro até o MMORPG |
| [ESPEC_REGRAS_DUNGEON_STANCE.md](docs/ESPEC_REGRAS_DUNGEON_STANCE.md) | Regras e fórmulas extraídas do protótipo, já convertidas para unidades da UE5 |
| [CLASSES.md](docs/CLASSES.md) | As 9 classes (valores do protótipo) |
| [ARVORES.MD](docs/ARVORES.MD) | Árvore de talentos (pontos ganhos ao subir as habilidades: Vitalidade, Ataque, Defesa, Magia) |
| [LISTA_MODELOS.MD](docs/LISTA_MODELOS.MD) | Lista de modelos 3D |
| [video_para_animacao.md](docs/video_para_animacao.md) | Pipeline de animação: vídeo IA → 3ds Max → UE5 |

## Pastas

| Pasta | Uso |
| --- | --- |
| `docs/` | Plano, especificação de regras e documentos de design |
| `arts/` | Concepts e assets brutos em andamento no 3ds Max — **não modificar**. Fora do Git (drive do grupo) |
| `_Export_3dsMax_Final/` | FBX validados, prontos para importar na UE5 (regras no README da pasta) |
| `Source/DungeonStance/` | Código C++ do jogo: `Core/`, `Gameplay/`, `UI/`, `Network/` |
| `Godot_Legacy_Reference/` | Cliente do protótipo Godot 4 (consulta). README antigo em `README_LEGACY.md`. Fora do Git |
| `Nakama_Legacy_Reference/` | Servidor autoritativo do protótipo em TypeScript — **fonte das regras de jogo**. Fora do Git |
| `build/` | Build jogável do protótipo (`launcher.ps1`) |
| `tools/` | Scripts de build e conversão |

## Protótipo legado

O protótipo Godot + Nakama continua rodando para consulta de gameplay: veja `Godot_Legacy_Reference/README_LEGACY.md`.
Os scripts `tools/build.ps1` e `build/launcher.ps1` já apontam para as pastas renomeadas.
