# Guia de Uso em Equipe — Git, GitHub, Unreal Engine 5 e 3ds Max

Como a equipe trabalha no desenvolvimento do **Dungeon Stance** sem perder arquivos, sem sobrescrever o trabalho do outro e compartilhando assets 3D (AccuRIG, 3ds Max) e interfaces (UI/Menus na UE5) em perfeita sincronia.

---

## 1. Antes de tudo (Configuração única por máquina)

1. Instale o [Git](https://git-scm.com/) e o [Git LFS](https://git-lfs.com/) (ou use o [GitHub Desktop](https://desktop.github.com/) que já instala e gerencia o LFS automaticamente com interface visual).
2. Rode no terminal **uma vez**, antes de qualquer commit ou clone:

   ```bash
   git lfs install
   ```

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
