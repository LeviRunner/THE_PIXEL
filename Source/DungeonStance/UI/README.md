# UI — Interfaces para grupos de 1 a 5 jogadores

Widgets em **UMG/CommonUI**, com a lógica em C++ (classes base `UDS*Widget`) e o layout em Blueprints (`WBP_*`).

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
- Ações viram chamadas ao `ADSPlayerController` (`Server_SelectHero`, `Server_SetReady`, `Server_PickCard`) ou ao `UDSSessionSubsystem`.
- Atualização por evento (delegates do GameState/PlayerState, `GetGameplayAttributeValueChangeDelegate`, `RegisterGameplayTagEvent`). Timers leves só para relógios/recargas e para o que ainda não tem delegate.
- Textos com `LOCTEXT` (namespace `DungeonStanceUI`); ícones/nomes no `UDSUIDataAsset` (nada de asset fixo no C++).
- Referência visual e fluxo dos menus antigos: `Godot_Legacy_Reference/scripts/ui/`.

## Classes C++

| Arquivo | Classe | Base | Papel |
| --- | --- | --- | --- |
| `DSHUD.h` | `ADSHUD` | `AHUD` | Cria o `UDSHUDLayoutWidget` para o jogador local; guarda o `UDSUIDataAsset` |
| `DSUIDataAsset.h` | `UDSUIDataAsset` | `UPrimaryDataAsset` | Nomes/ícones/cores por classe, stance, tamanho de party, status, habilidade (tag) e carta (fallback) |
| `DSUIStateAccess.h` | namespace `DSUIState` | — | **Único** ponto que chama getters de Core/Gameplay/Network. Se um nome mudar, ajuste só aqui |
| `DSUIStatics.h` | namespace `DSUI` | — | Helpers de apresentação (ícone soft, visibilidade, formatos de tempo, ciclo de enum) |
| `HUD/DSHUDLayoutWidget.h` | `UDSHUDLayoutWidget` | `UCommonActivatableWidget` | Raiz do HUD; espera GS/PS/ASC replicarem; alterna lobby / combate / fim de run pela `EDSMatchPhase` |
| `HUD/DSPartyFrameWidget.h` | `UDSPartyFrameWidget` | `UCommonUserWidget` | Lista de até 5 aliados (local primeiro), reage a entrada/saída no `PlayerArray` |
| `HUD/DSPartyMemberWidget.h` | `UDSPartyMemberWidget` | `UCommonUserWidget` | Vida, escudo, recurso, status, caído, classe, nome |
| `HUD/DSStatusIconWidget.h` | `UDSStatusIconWidget` | `UCommonUserWidget` | Ícone + acúmulos de um status |
| `HUD/DSCastBarWidget.h` | `UDSCastBarWidget` | `UCommonUserWidget` | 7 slots; atualiza recargas num timer de 0,1 s |
| `HUD/DSCastSlotWidget.h` | `UDSCastSlotWidget` | `UCommonUserWidget` | Ícone, recarga restante, custo insuficiente, cargas, glifo de input |
| `HUD/DSWaveIndicatorWidget.h` | `UDSWaveIndicatorWidget` | `UCommonUserWidget` | Onda X/10, chefe, inimigos restantes, nível/XP do time, tempo da fase |
| `HUD/DSTargetIndicatorWidget.h` | `UDSTargetIndicatorWidget` | `UCommonUserWidget` | Vida/escudo do alvo travado |
| `CardPick/DSCardPickWidget.h` | `UDSCardPickWidget` | `UCommonActivatableWidget` | Abre com `PendingCardPicks > 0` e escolhas; timer de 20 s; `Server_PickCard` |
| `CardPick/DSCardOptionWidget.h` | `UDSCardOptionWidget` | `UCommonButtonBase` | Uma carta (botão com foco de gamepad) |
| `Menus/DSLobbyWidget.h` | `UDSLobbyWidget` | `UCommonActivatableWidget` | Classe, stance, party, pronto, solo / procurar / hospedar |
| `Menus/DSEndOfRunWidget.h` | `UDSEndOfRunWidget` | `UCommonActivatableWidget` | Vitória/derrota, ondas, abates, XP de rank, ouro, espólio (TODO) |
| `HeroSheet/DSHeroSheetWidget.h` | `UDSHeroSheetWidget` | `UCommonActivatableWidget` | **Esqueleto**: atributos + abas Ficha/Cofre/Mochila/Cinto |

## Blueprints a criar no editor

Pastas sugeridas: `Content/UI/HUD`, `Content/UI/Menus`, `Content/UI/Data`.
**Negrito** = `BindWidget` obrigatório (o WBP não compila sem ele); o resto é `BindWidgetOptional`.

| Asset | Classe pai | Widgets com nome fixo | Configurar nos Class Defaults |
| --- | --- | --- | --- |
| `DA_DSUIData` | `UDSUIDataAsset` | — | Preencher Classes, Stances, PartySizeNames, Statuses (tags `Status.*`), Abilities (tag de asset → ícone), Cards |
| `BP_DSHUD` | `ADSHUD` | — | `LayoutClass = WBP_DSHUDLayout`, `UIData = DA_DSUIData`. Definir como HUD Class do GameMode |
| `BP_DSMenuHUD` | `ADSHUD` | — | `LayoutClass = WBP_DSMenuLayout` (layout só com `Lobby`) |
| `WBP_DSHUDLayout` | `UDSHUDLayoutWidget` | `PartyFrame`, `CastBar`, `WaveIndicator`, `CardPick`, `TargetIndicator`, `Lobby`, `EndOfRun` | — |
| `WBP_DSMenuLayout` | `UDSHUDLayoutWidget` | `Lobby` | No `Lobby`: `bShowSessionButtons = true`, `bAutoActivate = true` |
| `WBP_DSPartyFrame` | `UDSPartyFrameWidget` | **`MemberContainer`** (Vertical/HorizontalBox) | `MemberWidgetClass = WBP_DSPartyMember` |
| `WBP_DSPartyMember` | `UDSPartyMemberWidget` | **`HealthBar`**, `ShieldBar`, `ResourceBar`, `HealthText`, `PlayerNameText`, `ClassNameText`, `ClassIcon`, `DownedOverlay`, `StatusContainer` | `StatusIconClass = WBP_DSStatusIcon` |
| `WBP_DSStatusIcon` | `UDSStatusIconWidget` | **`StatusIcon`**, `StacksText` | — |
| `WBP_DSCastBar` | `UDSCastBarWidget` | **`Slot_Attack`**, **`Slot_ClassSkill`**, **`Slot_Dodge`**, `Slot_Support1..4` (instâncias de `WBP_DSCastSlot`) | Por instância: `InputAction` (glifo); `InputTag` é preenchido sozinho; `FallbackCooldownTag` (ex.: `Cooldown.Dodge`) |
| `WBP_DSCastSlot` | `UDSCastSlotWidget` | **`AbilityIcon`**, `CooldownBar`, `CooldownText`, `ChargesText`, `InsufficientResourceOverlay`, `EmptySlotOverlay`, `InputHint` (`CommonActionWidget`) | — |
| `WBP_DSWaveIndicator` | `UDSWaveIndicatorWidget` | **`WaveText`**, `BossWarning`, `EnemiesRemainingText`, `TeamLevelText`, `TeamXPBar`, `TeamXPText`, `PhaseTimeText` | Eventos `BP_OnBossWaveStarted`, `BP_OnTeamLevelUp` para animações |
| `WBP_DSTargetIndicator` | `UDSTargetIndicatorWidget` | **`TargetHealthBar`**, `TargetShieldBar`, `TargetHealthText` | Sobrescrever `ResolveLockedTarget` até existir a API do lock-on; nome do alvo em `BP_OnTargetChanged` |
| `WBP_DSCardPick` | `UDSCardPickWidget` | **`OptionContainer`**, `TimerText`, `TimerBar`, `PendingPicksText` | `OptionWidgetClass = WBP_DSCardOption` |
| `WBP_DSCardOption` | `UDSCardOptionWidget` | **`CardNameText`**, `CardDescriptionText`, `StacksText`, `CardIcon` | Estilo de botão CommonUI |
| `WBP_DSLobby` | `UDSLobbyWidget` | **`Btn_ClassPrev`**, **`Btn_ClassNext`**, **`ClassNameText`**, **`Btn_StancePrev`**, **`Btn_StanceNext`**, **`StanceNameText`**, `ClassDescriptionText`, `ClassIcon`, `StanceDescriptionText`, `StanceIcon`, `Btn_PartySizePrev`, `Btn_PartySizeNext`, `PartySizeText`, `Btn_Ready`, `ReadyText`, `Btn_StartSolo`, `Btn_FindMatch`, `Btn_Host`, `StatusText` | `bShowSessionButtons` (menu = true, partida = false). Botões = `UCommonButtonBase` (ex.: `WBP_DSButton`) |
| `WBP_DSEndOfRun` | `UDSEndOfRunWidget` | **`ResultTitleText`**, **`Btn_Continue`**, `HeroClassText`, `WavesClearedText`, `KillsText`, `RankXPText`, `GoldText`, `LootContainer` | `BP_OnContinueRequested` → abrir mapa do menu |
| `WBP_DSHeroSheet` | `UDSHeroSheetWidget` | **`TabSwitcher`** (4 filhos na ordem Ficha/Cofre/Mochila/Cinto), `Btn_TabSheet`, `Btn_TabVault`, `Btn_TabBackpack`, `Btn_TabBelt`, `StatHealthText`, `StatDamageText`, `StatAttackSpeedText`, `StatCritText`, `StatArmorText`, `StatMoveSpeedText` | Esqueleto |
| `WBP_DSButton` | `UCommonButtonBase` | — | Botão padrão + `CommonButtonStyle` |

## Configuração de projeto necessária (base)

- `Project Settings > Engine > General > Game Viewport Client Class = CommonGameViewportClient` (roteamento de input do CommonUI).
- `CommonInputSettings`: `InputData` com ações Confirm/Back e `bEnableEnhancedInputSupport = true` (glifos de `UInputAction` no `InputHint`).
- `ADSGameMode::HUDClass = BP_DSHUD`.
