# Pendências — Dungeon Stance (UE 5.8)

O que ficou faltando depois da primeira leva de código (branches `feat/core`, `feat/gameplay`, `feat/network`, `feat/ui` sobre `feat/projeto-base`; tudo junto em `dev/integracao`).
O andamento por fase fica em [CHECKLIST_FASES.md](CHECKLIST_FASES.md). Detalhes de cada área estão no `README.md` da pasta em `Source/DungeonStance/`.

> **⚠ O código nunca foi compilado.** A máquina onde foi escrito não tem Visual Studio/MSVC. Cerca de 17.500 linhas de C++ (153 arquivos) foram escritas seguindo a API da UE 5.5+, mas sem verificação. O primeiro passo é compilar e corrigir os erros.

---

## 0. Bloqueadores (fazer primeiro)

- [ ] Instalar **Visual Studio 2022** com o workload "Desenvolvimento de jogos com C++" (+ SDK do Windows e toolchain MSVC v14.3x/v14.4x indicada para a UE 5.8).
- [ ] Botão direito em `DungeonStance.uproject` → *Generate Visual Studio project files* → compilar `DungeonStanceEditor` (Development Editor).
- [ ] Corrigir os erros de compilação. Pontos de risco que os agentes apontaram:
  - **GEs construídos em C++** (`Gameplay/Effects/`): acesso a `GEComponents`, `StackingType`/`StackLimitCount` (podem estar privados/deprecados na 5.8). Plano B: transformar esses GEs em Blueprints `GE_*` configurados no editor.
  - **APIs do GAS 5.5+**: `GetDynamicSpecSourceTags()`, `GetAssetTags()`/`SetAssetTags()`, `SetReplicatedLooseGameplayTagCount`, `GetPrimaryInstance()`, `DynamicGrantedTags`.
  - **Task de root motion da esquiva**: assinatura de `UAbilityTask_ApplyRootMotionConstantForce::ApplyRootMotionConstantForce`.
  - **`SetNetUpdateFrequency`** no construtor (Core e Gameplay).
  - **`Engine/DeveloperSettings.h`**: pode exigir o módulo `"DeveloperSettings"` no `Build.cs`.
  - **HTTP/JSON** (Network): lambda de `OnProcessRequestComplete`, `TryGetObjectField`.
  - **`__has_include`** em `Core/DSBackendBridge.cpp`: depois de juntar as branches, trocar por include direto de `Network/DSBackendSubsystem.h`.
  - **CommonUI**: `UCommonActionWidget::SetEnhancedInputAction`, `FUIInputConfig`.
  - **Encoding**: `LOCTEXT` com acentos exige fontes lidos em UTF-8. Se der erro, salvar os `.cpp` como UTF-8 com BOM.
  - **Automation Tests**: `EAutomationTestFlags` combinados, `TestEqual` com enum.
- [ ] Rodar os Automation Tests: `DungeonStance.Core.RunRules.*`, regras de cartas e matemática de dano (Gameplay/Tests).
- [ ] Revisão de código C++ depois de compilar (memória, replicação, RPCs com validação).

## 1. Decisões em aberto

- [ ] **Backend**: (A) Nakama para conta, progresso e matchmaking × (B) EOS/Steam. O código de Network suporta as duas (`UDSNetworkSettings::Backend`).
- [ ] **Hospedagem de servidores dedicados** (GameLift, Edgegap, Agones ou máquina própria). O target `DungeonStanceServer` exige a **engine compilada do código-fonte**; a versão do Launcher não gera servidor.
- [ ] **Escala por tamanho da party**: valores propostos (`inimigos × (0,4 + 0,15N)`, `vida do chefe × (0,5 + 0,125N)`) precisam de playtest.
- [ ] **Jogador inativo**: a regra do protótipo (5 s sem estado) não foi portada; hoje só conta quem está conectado.
- [ ] **Número de opções de carta**: hoje são 3 (`ADSGameMode::NumCardChoices`).
- [ ] **Na derrota**: os inimigos ficam no mapa até `CloseMatch`. Revisar a experiência.

## 2. Base do projeto (arquivos compartilhados)

- [ ] `Config/DefaultGame.ini`: seção `[/Script/DungeonStance.DSNetworkSettings]` com `LobbyMap`/`DungeonMap` (exemplo em `Network/README.md`).
- [ ] `Config/DefaultGame.ini`: `[/Script/DungeonStance.DSGameplaySettings]` → `CardRegistry=/Game/DungeonStance/Data/Cards/DA_CardRegistry`.
- [ ] `Config/DefaultEngine.ini`: `GameViewportClientClass=/Script/CommonUI.CommonGameViewportClient` e `CommonInputSettings` com Enhanced Input habilitado.
- [ ] `Build.cs`: adicionar `"DeveloperSettings"` se a compilação pedir.
- [ ] Steam/EOS (se B): plugins no `.uproject` e `DefaultPlatformService` no `DefaultEngine.ini`.
- [ ] Travel lobby → dungeon **seamless** (`bUseSeamlessTravel`), para `CopyProperties` manter herói/Stance.

## 3. Core

- [ ] `ADSGameMode`: definir `GameSessionClass = ADSGameSession` (Network).
- [ ] `PreLogin`: ler `?DSToken=` e chamar `VerifyPlayerToken` do backend. Hoje o `?PlayerId=` é aceito **sem validação** (TODO de segurança).
- [ ] `ClassRank` fica em 1 até existir o caminho "servidor recebe o rank verificado do backend".
- [ ] `ReviveHero` passar a usar `ADSHeroCharacter::Revive()` (Gameplay).
- [ ] Hook para inimigos invocados no meio da onda (lacaios do chefe *brood*) entrarem na contagem de `EnemiesRemaining`.
- [ ] Trocar `SpawnActor` + `InitializeFromData` por `SpawnActorDeferred` se a Gameplay precisar dos dados antes do `BeginPlay`.
- [ ] XP de inimigo depende de a `MaxHealth` já estar aplicada após `InitializeFromData`. Conferir em jogo.
- [ ] Delegates pedidos pela UI: `ADSGameState::OnPlayerAdded/OnPlayerRemoved` e `ADSPlayerState::OnReadyChanged`. Hoje a UI faz polling de 0,25–0,5 s.
- [ ] `FDSRunResult` replicado completo, com ouro e itens do Espólio (a UI ainda mostra ouro 0).

## 4. Gameplay

**Assets a criar no editor** (lista completa em `Gameplay/README.md`):
- [ ] Input: `IMC_Hero` + `IA_*`. `IA_TargetLock` = botão do meio do mouse / R3; `IA_TargetSwitch` = roda do mouse / analógico direito.
- [ ] 9 × `DA_Class_*` e 9 × `BP_Hero_*`, ligados em `HeroPawnClasses` do GameMode.
- [ ] `DA_AbilitySet_Common` + um `DA_AbilitySet_<Classe>` por classe.
- [ ] `DT_Cards`: importar `Gameplay/Data/Import/DT_Cards.json` (192 cartas) + `DA_CardRegistry`.
- [ ] 8 × `DA_Support_*` (de `SupportAbilities.json`) + 8 BPs de `UDSGA_SupportAura`.
- [ ] `DA_Enemy_*`, `BP_Enemy`, BPs de projétil e NavMesh nos mapas.
- [ ] Animações/montages e GameplayCues de VFX.

**Código faltando:**
- [ ] Skills em C++ de Rajada, Lança de Ossos, Flecha Perfurante e Meteoro (as de área/cone usam `UDSGA_AreaStrike` em BP).
- [ ] Mecânicas de classe: invocações do Necromante, aura do Cruzado, lentidão do Arcanista, escudo inicial de onda.
- [ ] Cartas de gatilho (cura no crítico, combustão, espinhos…). O gancho existe (`Event.Crit`, `GetCardStat`), mas nenhuma está ligada.
- [ ] Escudo de apoio expirar após a duração; cura contínua (Flor da Vida/Sentinela).
- [ ] Cargas de habilidades de apoio e a regra "5ª habilidade substitui uma".
- [ ] Recarga interna do atordoamento.
- [ ] DoTs com dono/duração por acúmulo (hoje o spec é único e usa o primeiro).
- [ ] Sprint com *saved move* no CharacterMovement, para evitar correção de posição em rede.
- [ ] IA: migrar de `MoveTo` por decisão para StateTree.

## 5. Network

- [ ] Criar o RPC **`submit_run_results`** no Nakama (servidor → servidor, `http_key`, idempotente por `run_id`, com clamp de rank_xp/ouro, reaproveitando `grantMatchXp`/espólio).
- [ ] Run solo com Nakama **não salva progresso**: o cliente não tem a `http_key`. Definir o fluxo (servidor de validação ou RPC de cliente com validação).
- [ ] Matchmaker por ticket do Nakama (WebSocket) + alocação de servidor dedicado. Hoje é "procura sessão → entra, senão hospeda".
- [ ] `UDSPartySubsystem`: grupo pré-formado com convite. Hoje só o líder entra na fila.
- [ ] Refresh de token e criptografia do NetDriver.
- [ ] Segredos (`DS_NAKAMA_SERVER_KEY`, `DS_NAKAMA_HTTP_KEY`) vêm de **variáveis de ambiente**; nunca colocar no `.ini` versionado.

## 6. UI

**Assets a criar** (nomes dos `BindWidget` obrigatórios em `UI/README.md`):
- [ ] `DA_DSUIData`, `BP_DSHUD`, `BP_DSMenuHUD`.
- [ ] `WBP_DSHUDLayout`, `WBP_DSMenuLayout`, `WBP_DSPartyFrame`, `WBP_DSPartyMember`, `WBP_DSStatusIcon`, `WBP_DSCastBar`, `WBP_DSCastSlot`, `WBP_DSWaveIndicator`, `WBP_DSTargetIndicator`, `WBP_DSCardPick`, `WBP_DSCardOption`, `WBP_DSLobby`, `WBP_DSEndOfRun`, `WBP_DSHeroSheet`, `WBP_DSButton`.
- [ ] `HUDClass = BP_DSHUD` no GameMode (hoje está `ADSHUD` em C++).

**Arte:** ícones de 9 classes, 4 Stances, 9 status, 3 habilidades básicas + 8 de apoio, cartas; estilos CommonUI e glifos de input (teclado/gamepad).

**Telas faltando:**
- [ ] Inventário real (grades Cofre 120 / Mochila 28 / Cinto 4) e Espólio.
- [ ] Lista de sessões navegável (hoje "Procurar" entra no primeiro resultado).
- [ ] Menu de pausa e opções.
- [ ] Nome/ícone das cartas pela API de `UDSCardData` (`UDSCardLibrary::GetCardRegistry()->FindCard(Id)`). Hoje usa o fallback do UIData.
- [ ] Contagem de cargas no slot de habilidade (depende da Gameplay definir as cargas).

## 7. Testes e qualidade

- [ ] PIE com 5 clientes (Listen Server) jogando uma run completa.
- [ ] Latência simulada (`Net PktLag=100`, `PktLoss=2`) para validar predição de habilidades e esquiva.
- [ ] Testes funcionais (Functional Tests em mapa) para ciclo de ondas e escolha de cartas.
- [ ] Revisão de segurança do fluxo de login/token e do envio de resultados ao backend.
