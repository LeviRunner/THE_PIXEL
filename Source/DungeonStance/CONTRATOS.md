# Contratos entre áreas — módulo `DungeonStance`

Um único módulo C++ com 4 pastas. Cada pasta tem um dono; quem precisa de algo de outra área usa **só** a API listada aqui.
Includes sempre a partir da raiz do módulo: `#include "Core/DSGameState.h"`.

Arquivos da **base** (não editar dentro de uma feat de área; pedir mudança em `PENDENCIAS.md`):
`DungeonStance.Build.cs`, `DungeonStance.h/.cpp`, `Core/DSTypes.h`, `Core/DSGameplayTags.h/.cpp`, `Config/*`, `*.uproject`.

Convenções: prefixo `DS`, padrão de nomes da Unreal (`PascalCase`, `b` para bool), log `LogDungeonStance`, números de balanceamento em Data Assets (nunca fixos no C++, exceto fórmulas puras portadas do protótipo em `Core/DSRunRules.h`).

## Core — `Core/`

| Classe / arquivo | API pública usada por outras áreas |
| --- | --- |
| `DSRunRules.h` (namespace `DSRunRules`, funções puras) | `IsBossWave(int32 Wave, int32 MaxWaves)`, `GetRegularEnemyCount(int32 Wave)`, `GetWaveHealthMultiplier(int32 Wave, float AvgRank)`, `GetWaveDamageMultiplier(int32 Wave, float AvgRank)`, `GetPartyEnemyCountMultiplier(int32 NumPlayers)`, `GetPartyBossHealthMultiplier(int32 NumPlayers)`, `GetTeamXPToNext(int32 Level)`, `GetRankXPToNext(int32 Rank)`, `GetEndOfRunRankXP(int32 WavesCleared, bool bVictory, int32 Kills)`, `GetSpawnOffset(int32 Index, int32 NumPlayers, float RadiusCm)`, `GetArenaEffectiveLevel(int32 ArenaIndex, int32 Torment)`, `GetWaveGold(int32 Wave, int32 Kills)` |
| `ADSGameMode` (`AGameModeBase`) | Só servidor. Usa `UDSRunRulesSubsystem`/`DSRunRules`. Spawna inimigos via `ADSEnemyCharacter::InitializeFromData`. `void NotifyEnemyKilled(ADSEnemyCharacter*, AController* Killer)` e `void NotifyHeroDowned(ADSHeroCharacter*)` |
| `ADSGameState` (`AGameStateBase`) | Replicado: `int32 CurrentWave`, `int32 MaxWaves`, `bool bIsBossWave`, `EDSMatchPhase MatchPhase`, `int32 TeamLevel`, `int32 TeamXP`, `int32 TeamXPToNext`, `float PhaseEndServerTime`, `int32 EnemiesRemaining`. Getters `BlueprintPure`. Delegates `BlueprintAssignable`: `OnWaveChanged(int32 Wave, bool bIsBoss)`, `OnMatchPhaseChanged(EDSMatchPhase NewPhase)`, `OnTeamXPChanged(int32 Level, int32 XP, int32 XPToNext)`, `OnEnemiesRemainingChanged(int32 Remaining)`. `float GetPhaseTimeRemaining() const` |
| `ADSPlayerState` (`APlayerState`, `IAbilitySystemInterface`) | Cria no construtor `UDSAbilitySystemComponent* AbilitySystemComponent` (replicação `Mixed`) e `UDSAttributeSet* AttributeSet`. Replicado: `EDSHeroClass HeroClass`, `EDSStance Stance`, `int32 ClassRank`, `int32 Kills`, `int32 PendingCardPicks`, `TArray<FDSOwnedCard> OwnedCards`, `TArray<FName> CurrentCardChoices` (só dono), `bool bIsReady`. Getters + delegates `OnPendingPicksChanged(int32)`, `OnCardsChanged()`, `OnCardChoicesChanged()`, `OnHeroSelectionChanged()`. `UDSAttributeSet* GetAttributeSet() const` |
| `ADSPlayerController` (`APlayerController`) | RPCs: `Server_SelectHero(EDSHeroClass, EDSStance)`, `Server_SetReady(bool)`, `Server_PickCard(FName CardId)`. Cria o HUD (`ADSHUD`) no cliente local |
| `UDSGameInstance` (`UGameInstance`) | Perfil local (`FString LocalPlayerId`, `EDSHeroClass LastClass`, `EDSStance LastStance`), `EDSPartySize DesiredPartySize`. Acesso ao backend via `UDSBackendSubsystem` (Network) |
| `UDSRunRulesSubsystem` (`UWorldSubsystem`) | Wrapper Blueprint/configurável das fórmulas + RNG determinístico da run (`InitializeSeed(int32)`, `FRandomStream& GetRunRandom()`) |

## Gameplay — `Gameplay/`

| Classe / arquivo | API pública |
| --- | --- |
| `Abilities/DSAbilitySystemComponent.h` (`UAbilitySystemComponent`) | `AbilityInputTagPressed(FGameplayTag)`, `AbilityInputTagReleased(FGameplayTag)`, `ProcessAbilityInput(float, bool)`, `GrantAbilitySet(const UDSAbilitySet*)` |
| `Attributes/DSAttributeSet.h` (`UAttributeSet`) | Atributos com `ATTRIBUTE_ACCESSORS`: `Health`, `MaxHealth`, `Shield`, `Resource`, `MaxResource`, `Stamina`, `MaxStamina`, `Damage`, `AttackSpeed`, `AttackRange`, `MoveSpeed`, `CritChance`, `CritDamage`, `Armor`, `CooldownReduction`; meta `IncomingDamage`, `IncomingHeal` |
| `Characters/DSCharacterBase.h` (`ACharacter`, `IAbilitySystemInterface`) | `bool IsAlive() const`, delegate nativo `FDSOnDeath OnDeath` |
| `Characters/DSHeroCharacter.h` | ASC vem do `ADSPlayerState` (`PossessedBy`/`OnRep_PlayerState`). Input via Enhanced Input → tags `Input.*`. `UDSTargetLockComponent* GetTargetLock() const` |
| `Characters/DSEnemyCharacter.h` | ASC próprio. `void InitializeFromData(const UDSEnemyData* Data, int32 Level, float HealthMultiplier, float DamageMultiplier, bool bElite)`. Delegate nativo `OnEnemyDied(ADSEnemyCharacter* Enemy, AController* Killer)` |
| `Data/DSClassData.h` (`UPrimaryDataAsset`) | Base stats, `EDSAttackType`, habilidades concedidas, ícone, nome |
| `Data/DSCardData.h` + `Cards/DSCardLibrary.h` | `UDSCardData` (id, stance, ramo, pais, máx. acúmulos, modificadores). `UDSCardLibrary::IsCardRollable(...)`, `RollCardChoices(...)`, `ValidateCardPick(...)`, `ApplyCardToPlayer(...)` |
| `Data/DSEnemyData.h` | Arquétipo, multiplicadores, comportamento de IA |
| `Stance/DSStanceComponent.h` | Stance ativa e cartas aplicadas como GEs |

## Network — `Network/`

| Classe / arquivo | API pública |
| --- | --- |
| `DSSessionSubsystem` (`UGameInstanceSubsystem`) | `HostSession(EDSPartySize, bool bIsLAN)`, `FindSessions(EDSPartySize)`, `JoinSession(int32 ResultIndex)`, `DestroySession()`, `StartSoloRun()`. Delegates `OnSessionCreated(bool)`, `OnSessionsFound(const TArray<FDSSessionInfo>&)`, `OnSessionJoined(bool)` |
| `DSBackendSubsystem` (`UGameInstanceSubsystem`) + interface `IDSBackendService` | `Authenticate(...)`, `FetchProfile(...)`, `SubmitRunResults(const TArray<FDSRunResult>&)`. Implementações: `UDSNullBackend` (offline) e `UDSNakamaBackend` (REST) |

## UI — `UI/`

| Classe / arquivo | API pública |
| --- | --- |
| `ADSHUD` (`AHUD`) | Cria o layout principal (`UDSHUDLayoutWidget`) para o jogador local |
| `UDS*Widget` (`UCommonUserWidget`/`UCommonActivatableWidget`) | Só lê estado replicado; ações viram chamadas ao `ADSPlayerController` / subsistemas de Network |
