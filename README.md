# Dungeon Stance — Unreal Engine 5

Roguelike cooperativo de ação em dungeons instanciadas para **1 a 5 jogadores** (Solo, Dupla, Trio, Esquadrão de 4, Grupo de 5).
Cada herói entra na dungeon com uma **Stance** (Crítico, Arcana, Defesa ou Apoio) e evolui durante a run com cartas da sua árvore, enfrentando 10 ondas com chefes nas ondas 5 e 10.

Primeiro grande lançamento: o **Ato I de Dungeon Stance** (as 5 Dungeons iniciais, cobrindo os Tiers 1 ao 5: Bronze, Ferro, Prata, Aço e Ouro).
A expansão para **MMORPG (Ato II)** levará a jornada aos Tiers 6, 7 e 8 (Vermécio, Sombraço e Auralita) até o nível máximo.

**Stack:** Unreal Engine 5 (C++ + Blueprints + Gameplay Ability System) · 3ds Max · backend de conta a decidir (Nakama ou EOS/Steam).

---

## 📚 Documentação Atualizada do Projeto

| Documento | Conteúdo |
| :--- | :--- |
| [`docs/FLUXO_TELAS_E_GAMEPLAY.md`](docs/FLUXO_TELAS_E_GAMEPLAY.md) | **Fluxo oficial de telas** (Menu Solo/Co-op, Cidadela Flutuante, Portal e Mecânica da Pedra de TP) |
| [`docs/CICLO_5_DUNGEONS.md`](docs/CICLO_5_DUNGEONS.md) | **Ciclo das 5 Dungeons do Ato I** (1 Tier exclusivo por Dungeon: Bronze, Ferro, Prata, Aço e Ouro) |
| [`docs/ROTEIRO_VERTICAL_SLICES.md`](docs/ROTEIRO_VERTICAL_SLICES.md) | **Roteiro em Fatias Jogáveis** (Personagem -> Movimentação -> Combate -> Mineração -> Forja) |
| [`docs/PLANO_DUNGEON_STANCE_UE5.md`](docs/PLANO_DUNGEON_STANCE_UE5.md) | Plano de execução técnica da UE5, arquitetura C++/GAS e migração |
| [`docs/ESPEC_REGRAS_DUNGEON_STANCE.md`](docs/ESPEC_REGRAS_DUNGEON_STANCE.md) | Regras e fórmulas extraídas do protótipo, convertidas para unidades da UE5 |
| [`docs/CLASSES.md`](docs/CLASSES.md) | Especificação das 9 classes de combate |
| [`docs/ARVORES.MD`](docs/ARVORES.MD) | Árvores de talentos e caminhos |
| [`docs/video_para_animacao.md`](docs/video_para_animacao.md) | Pipeline de animação: vídeo IA → 3ds Max → AccuRIG → UE5 |
| [`CHECKLIST_FASES.md`](CHECKLIST_FASES.md) | Checklist de fases de implementação |
| [`PENDENCIAS.md`](PENDENCIAS.md) | Pendências e decisões em aberto por área |
| [`GUIA_EQUIPE.md`](GUIA_EQUIPE.md) | Guia de trabalho em equipe com Git e Git LFS |

---

## 📂 Organização das Pastas do Projeto

| Pasta | Descrição e Uso |
| :--- | :--- |
| `_Export_3dsMax_Final/` | **FBX finais e prontos para importar na UE5** (estrutura com `Characters/`, `Environments/`, `Items/`, `Props/`) |
| `arts/modelos_3D/` | Backlog e diretórios de modelagem em andamento para as 5 dungeons e os hubs da Cidadela |
| `arts/CONCEPT/` & `arts/FIGMA/` | Artes conceituais, blueprints e interfaces |
| `docs/` | Especificações de design, regras, lores e anotações originais (`docs/prints/`) |
| `Source/DungeonStance/` | Código-fonte C++ do jogo na Unreal Engine 5 |
| `Godot_Legacy_Reference/` | Cliente do protótipo legado em Godot 4 (apenas para consulta) |
| `Nakama_Legacy_Reference/` | Servidor autoritativo legado em TypeScript (fonte das regras originais) |
| `build/` & `tools/` | Scripts de build e ferramentas utilitárias |

---

## 🎯 Próximo Passo Prático
Consulte [`docs/ROTEIRO_VERTICAL_SLICES.md`](docs/ROTEIRO_VERTICAL_SLICES.md) para testar a primeira fatia jogável (importação do herói com rigging do AccuRIG e movimentação no motor de jogo).
