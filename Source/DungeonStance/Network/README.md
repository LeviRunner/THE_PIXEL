# Network & Matchmaking

Conexões, sessões e sincronização das instâncias do Dungeon Stance.

## Decisão (registrar aqui)

| Opção | Simulação da dungeon | Login / matchmaking / progresso | Situação |
| --- | --- | --- | --- |
| **A — recomendada para o lançamento** | UE5 Dedicated Server (ou Listen Server no Solo/coop entre amigos) | Nakama mantido como backend de conta, matchmaking, inventário e progressão (REST/WebSocket) | Aproveita o que já existe e testado em `Nakama_Legacy_Reference/` (inventário, progressão, espólio) |
| B | UE5 Dedicated Server | Online Subsystem nativo (Steam / EOS Sessions) | Menos peças, mas progressão e inventário precisam de outro backend |

**Status:** a decidir. Até lá, implementar o gameplay com replicação padrão da UE5 (cliente-servidor), independente do backend.

## O que muda em relação ao protótipo

- No Godot, a partida rodava **dentro do Nakama** (match handler em TypeScript, 10 Hz). Na UE5, a simulação passa para o servidor da Unreal; o Nakama (se mantido) deixa de simular combate.
- Matchmaker: `SQUAD_SIZE = 4` fixo → party de **1 a 5**, com fila por tamanho e opção de grupo pré-formado. Solo não precisa de matchmaking.
- Persistência (rank da classe, runas, inventário, espólio) continua sendo escrita só pelo servidor.

## Evolução para MMORPG (pós-lançamento)

Se o Dungeon Stance tiver aceitação, o caminho de evolução é: hub social persistente → mundo aberto coop por instância (o protótipo já tem `world_*.ts` com 28 skills de coleta/combate/suporte) → shards maiores. Pontos de atenção: replicação em escala (Iris / Replication Graph), servidores por zona, e MassEntity para multidões de NPCs.
