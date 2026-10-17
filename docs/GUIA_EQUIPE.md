# Guia de Uso em Equipe — Git, GitHub e Git LFS

Como o grupo trabalha no repositório do Dungeon Stance sem perder arquivos nem sobrescrever o trabalho do outro.
As regras ficam em dois arquivos da raiz: [`.gitignore`](.gitignore) (o que **não** sobe) e [`.gitattributes`](.gitattributes) (o que vai pelo Git LFS e pode ser travado).

## 1. Antes de tudo (uma vez por máquina)

1. Instale o [Git](https://git-scm.com/) e o [Git LFS](https://git-lfs.com/).
2. Rode **uma vez**, antes do primeiro commit ou clone:

   ```bash
   git lfs install
   ```

   Sem isso, as imagens e modelos entram no histórico do Git sem LFS, o repositório incha e o GitHub recusa arquivos acima de 100 MB.
3. Clone o repositório:

   ```bash
   git clone <endereço-do-repositório-no-github>
   ```

## 2. O que sobe para o GitHub

- `README.md`, este guia, `docs/`, `Source/`, `_Export_3dsMax_Final/` e `tools/`.
- Do `build/`, só o `launcher.ps1`.
- Quando o projeto UE5 for criado: `.uproject`, `Config/` e `Content/`.

## 3. O que fica de fora

- **`arts/`:** concepts e arquivos de trabalho do 3ds Max (mais de 600 MB). Grande demais para o repositório; compartilhe por um drive do grupo. Só o que for validado entra no Git, exportado em `_Export_3dsMax_Final/`.
- **Protótipos legados:** `Godot_Legacy_Reference/` e `Nakama_Legacy_Reference/`. Ficam só para consulta local (ou no drive do grupo); as regras que importam estão em [`docs/ESPEC_REGRAS_DUNGEON_STANCE.md`](docs/ESPEC_REGRAS_DUNGEON_STANCE.md).
- **Gerado pela UE5:** `Binaries/`, `Intermediate/`, `Saved/`, `DerivedDataCache/` e os arquivos de projeto do Visual Studio (`.sln`, `.vs/`). Cada máquina recria ao abrir/compilar o projeto.
- **Build do protótipo:** `SquadRoguelike.exe` (173 MB) e o `.pck`, que o `tools/build.ps1` recria.
- **Temporários de arte:** autosaves e backups do 3ds Max, `.skb` do SketchUp, `Thumbs.db`.
- **Segredos e configurações pessoais:** `.env`, chaves, `.ini` de usuário e a pasta `Content/Developers/`, que é de testes de cada pessoa no editor.

> Como `arts/` e os protótipos não estão no Git, quem clonar o repositório precisa copiá-los do drive do grupo para a raiz do projeto, se quiser consultá-los ou rodar `tools/build.ps1` e `build/launcher.ps1`.

## 4. Editar em grupo (Git LFS + travas)

- `.uasset`, `.umap`, `.max`, `.skp`, `.cdr` e `.psd` vão pelo LFS e **podem ser travados**. Esses arquivos não se combinam num merge: se duas pessoas editarem o mesmo arquivo, o trabalho de uma delas se perde.
- FBX, OBJ, GLB, imagens, áudio, vídeo e ZIP também vão pelo LFS (sem trava).

### Fluxo para editar um arquivo travável

```bash
git pull                                   # pegue a versão mais recente primeiro
git lfs lock Content/Maps/Dungeon_01.umap  # trava: ninguém mais consegue enviar alterações nele
# ... edite no Unreal / 3ds Max ...
git add Content/Maps/Dungeon_01.umap
git commit -m "feat: adiciona sala do chefe na Dungeon_01"
git push
git lfs unlock Content/Maps/Dungeon_01.umap  # libera para o grupo
```

- Ver quem travou o quê: `git lfs locks`
- Não deixe arquivos travados de um dia para o outro sem avisar o grupo.
- Destravar o arquivo de outra pessoa (só se ela sumiu e combinado com o grupo): `git lfs unlock --force <arquivo>`

## 5. Rotina do dia a dia

1. `git pull` antes de começar.
2. Trabalhe numa branch própria para tarefas maiores: `git switch -c feat/nome-da-tarefa`.
3. Commits pequenos, com mensagem no formato `tipo: descrição` (`feat`, `fix`, `docs`, `refactor`, `chore`, `art`).
4. `git push` e abra um Pull Request no GitHub para revisão antes de juntar na `main`.
5. Assets do 3ds Max só entram em `_Export_3dsMax_Final/` depois de validados (regras em [`_Export_3dsMax_Final/README.md`](_Export_3dsMax_Final/README.md)).

## 6. Cota do Git LFS

O GitHub dá de graça uma cota limitada de LFS (armazenamento e download). Com a arte crescendo, provavelmente vai ser preciso um pacote pago de dados LFS ou outro host (por exemplo, um servidor Git próprio ou Azure DevOps). Acompanhe o uso em **Settings → Billing** da conta/organização dona do repositório.
