<<<<<<< HEAD
# Guia de Uso em Equipe — Git, GitHub, Unreal Engine 5 e 3ds Max

Como a equipe trabalha no desenvolvimento do **Dungeon Stance** sem perder arquivos, sem sobrescrever o trabalho do outro e compartilhando assets 3D (AccuRIG, 3ds Max) e interfaces (UI/Menus na UE5) em perfeita sincronia.

---

## 0. erro commit

git push origin main --no-verify


se tiver dando bug usa esse termo ai para sync

## 1. Antes de tudo (Configuração única por máquina)

1. Instale o [Git](https://git-scm.com/) e o [Git LFS](https://git-lfs.com/) (ou use o [GitHub Desktop](https://desktop.github.com/) que já instala e gerencia o LFS automaticamente com interface visual).
2. Rode no terminal **uma vez**, antes de qualquer commit ou clone:
=======
# Guia de Uso em Equipe — Git, GitHub e Git LFS

Como o grupo trabalha no repositório do Dungeon Stance sem perder arquivos nem sobrescrever o trabalho do outro.
As regras ficam em dois arquivos da raiz: [`.gitignore`](.gitignore) (o que **não** sobe) e [`.gitattributes`](.gitattributes) (o que vai pelo Git LFS e pode ser travado).

## 1. Antes de tudo (uma vez por máquina)

1. Instale o [Git](https://git-scm.com/) e o [Git LFS](https://git-lfs.com/).
2. Rode **uma vez**, antes do primeiro commit ou clone:
>>>>>>> eacfc50a755de3897a0a2ef3b8e59dc80dc89f65

   ```bash
   git lfs install
   ```

<<<<<<< HEAD
   *Sem isso, arquivos binários pesados (modelos 3D, texturas, .uasset) entram no histórico puro do Git, inflando o repositório e estourando o limite de 100 MB do GitHub.*
3. Clone o repositório no seu disco (ex: `E:\The_Pixel`).

---

## 2. Como Compartilhar Assets 3D e Rigging (AccuRIG + 3ds Max)

### O Caso Prático: Boneco com Rig e Sockets das Mãos
Quando você (ou seu colega) cria um personagem no 3ds Max e faz o rigging no AccuRIG:

1. **O que NÃO fazer:** Não compartilhe arquivos `.iAvatar` para tentar continuar o rig na outra máquina (o AccuRIG não exporta nem sincroniza sockets da Unreal por esse formato).
2. **O Fluxo Correto de Compartilhamento:**
   - Exporte o **`.FBX` com preset de Unreal Engine** de dentro do AccuRIG.
   - Salve o arquivo na pasta compartilhada:  
     👉 `_Export_3dsMax_Final/Characters/Heroes/SK_Hero_Male.fbx`
   - Importe esse `.FBX` para dentro da Unreal Engine na pasta do projeto:  
     👉 `Content/Characters/Hero_Male/`
   - Abra o **Skeleton** gerado na Unreal (`SK_Hero_Male_Skeleton`) e crie os dois sockets nos ossos das mãos:
     - `hand_r` ➔ Botão direito ➔ *Add Socket* ➔ **`Socket_Weapon_R`**
     - `hand_l` ➔ Botão direito ➔ *Add Socket* ➔ **`Socket_Weapon_L`**
   - Salve o arquivo (`Ctrl + S`).
3. **Sincronizando com seu colega:**
   - Faça o commit e envie para o GitHub:
     ```bash
     git add _Export_3dsMax_Final/ Content/Characters/
     git commit -m "feat(character): adiciona SK_Hero_Male com sockets de arma configurados"
     git push
     ```
   - **No PC do seu colega:** Ele só precisa rodar **`git pull`** (ou clicar em *Fetch / Pull* no GitHub Desktop).
   - Ao abrir a Unreal dele, o boneco já estará importado, com a malha pesada, os ossos e os dois sockets exatamente no mesmo lugar!

---

## 3. Como Compartilhar Telas e Menus de Interface (UI / Blueprints)

### O Caso Prático: Um cria o Menu enquanto o outro mexe no Personagem
Para evitar que vocês dois entrem em conflito mexendo nas mesmas coisas:

1. **Divisão de Pastas no Projeto da UE5:**
   - Quem cuida de Personagens/Combate mexe em: `Content/Characters/` e `Content/Items/`
   - Quem cuida de Telas/Menus mexe em: `Content/UI/`
2. **Quando seu colega criar as telas:**
   - Ele cria os Widget Blueprints (ex: `Content/UI/WBP_MainMenu.uasset` e `Content/UI/WBP_Lobby.uasset`, conforme desenhado em [`docs/FLUXO_TELAS_E_GAMEPLAY.md`](FLUXO_TELAS_E_GAMEPLAY.md)).
   - Ele faz o commit e envia:
     ```bash
     git add Content/UI/
     git commit -m "feat(ui): implementa menu principal e lobby da cidadela flutuante"
     git push
     ```
3. **No seu PC:**
   - Você roda `git pull`.
   - Automaticamente a pasta `Content/UI/` surge no seu Content Browser da Unreal, pronta para ser aberta, testada e vinculada à inicialização do jogo!

---

## 4. Arquivos Binários da UE5 e Travas de Edição (Git LFS Locks)

Arquivos `.uasset` (Blueprints, Materiais, Texturas, Widgets) e `.umap` (Mapas/Fases) são **arquivos binários**.
*Ao contrário de código C++ ou textos, o Git NÃO consegue juntar alterações de duas pessoas no mesmo `.uasset`.* Se os dois salvarem o mesmo arquivo ao mesmo tempo, um perderá o trabalho!

### Regras para trabalhar sem conflito:
1. **Regra de Ouro:** Nunca editem o mesmo arquivo `.uasset` ou `.umap` no mesmo dia sem combinar.
2. **Usando a Trava de Arquivo (Lock):**
   Antes de editar um arquivo central (como o mapa da Cidadela ou o PlayerController):
   ```bash
   git pull                                         # Sempre atualize antes
   git lfs lock Content/Maps/Cidadela_Hub.umap      # Trava o arquivo no GitHub
   ```
   *(Agora o Git impedirá que qualquer outra pessoa envie alterações nesse arquivo até que você destrave).*
3. **Ao terminar o trabalho:**
   ```bash
   git add Content/Maps/Cidadela_Hub.umap
   git commit -m "feat(map): atualiza layout da praça central da cidadela"
   git push
   git lfs unlock Content/Maps/Cidadela_Hub.umap    # Libera para a equipe
   ```
4. **Consultar travas ativas:** `git lfs locks`

---

## 5. O que sobe para o Git vs. O que fica de fora

| O que SOBE para o Git | O que FICA DE FORA (`.gitignore`) |
| :--- | :--- |
| Código C++ (`Source/DungeonStance/`) | Pasta `arts/` pesada (compartilhada no Drive da equipe) |
| Assets validados em `_Export_3dsMax_Final/` | Pastas de compilação da UE5 (`Binaries/`, `Intermediate/`, `Saved/`, `DerivedDataCache/`) |
| Blueprints, Widgets e Mapas em `Content/` | Arquivos de solução do Visual Studio (`.sln`, `.vs/`) |
| Configurações da engine em `Config/` | Backups e autosaves do 3ds Max (`.bak`, `.max` rascunhos) |
| Documentações em `docs/` e `README.md` | Arquivos pessoais de preferências de usuário |

---

## 6. Rotina Diária Recomendada (Checklist de 10 segundos)

* ☀️ **Ao começar o dia:**  
  Abra o GitHub Desktop (ou terminal) e dê **`git pull`** antes de abrir a Unreal Engine.
* ☕ **Durante o dia:**  
  Comunique no Discord/WhatsApp o que está fazendo: *"Estou mexendo na UI do Menu"* / *"Estou mexendo no boneco e nos sockets"*.
* 🌙 **Ao terminar a tarefa:**  
  Salve tudo na Unreal, faça um commit claro com o que fez (`git commit -m "..."`) e dê **`git push`**.
=======
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
>>>>>>> eacfc50a755de3897a0a2ef3b8e59dc80dc89f65
