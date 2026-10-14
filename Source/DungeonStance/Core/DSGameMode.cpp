#include "Core/DSGameMode.h"

#include "AbilitySystemComponent.h"
#include "Components/CapsuleComponent.h"
#include "Core/DSBackendBridge.h"
#include "Core/DSCardBridge.h"
#include "Core/DSGameplayTags.h"
#include "Core/DSGameState.h"
#include "Core/DSPlayerController.h"
#include "Core/DSPlayerState.h"
#include "Core/DSRunRulesSubsystem.h"
#include "Core/DSWaveSpawnHelpers.h"
#include "DungeonStance.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "GameFramework/PlayerStart.h"
#include "Gameplay/Attributes/DSAttributeSet.h"
#include "Gameplay/Characters/DSEnemyCharacter.h"
#include "Gameplay/Characters/DSHeroCharacter.h"
#include "Gameplay/Data/DSEnemyData.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"
#include "UI/DSHUD.h"

namespace DSGameModeOptions
{
	// Opções de URL aceitas pelo mapa da dungeon ("?PartySize=3?Seed=42?Torment=1").
	constexpr const TCHAR* PartySize = TEXT("PartySize");
	constexpr const TCHAR* Seed = TEXT("Seed");
	constexpr const TCHAR* Torment = TEXT("Torment");
	constexpr const TCHAR* PlayerId = TEXT("PlayerId");
}

ADSGameMode::ADSGameMode(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	PrimaryActorTick.bCanEverTick = false;
	GameStateClass = ADSGameState::StaticClass();
	PlayerStateClass = ADSPlayerState::StaticClass();
	PlayerControllerClass = ADSPlayerController::StaticClass();
	HUDClass = ADSHUD::StaticClass();
	// Ninguém ganha pawn ao entrar: os heróis nascem juntos no anel quando a run começa.
	bStartPlayersAsSpectators = true;
}

// ---------------------------------------------------------------------------
// Ciclo de vida do GameMode
// ---------------------------------------------------------------------------

void ADSGameMode::InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage)
{
	Super::InitGame(MapName, Options, ErrorMessage);

	ExpectedPlayers = DSRunRules::ClampPartySize(UGameplayStatics::GetIntOption(Options, DSGameModeOptions::PartySize, DSConstants::MinPartySize));
	Torment = FMath::Max(0, UGameplayStatics::GetIntOption(Options, DSGameModeOptions::Torment, Torment));
	const int32 RandomSeed = static_cast<int32>(FPlatformTime::Cycles() & 0x7fffffff);
	RunSeed = UGameplayStatics::HasOption(Options, DSGameModeOptions::Seed)
		? UGameplayStatics::GetIntOption(Options, DSGameModeOptions::Seed, RandomSeed)
		: RandomSeed;

	RunRules = GetWorld()->GetSubsystem<UDSRunRulesSubsystem>();
	if (RunRules)
	{
		RunRules->InitializeSeed(RunSeed);
	}
	UE_LOG(LogDungeonStance, Log, TEXT("DSGameMode: party esperada %d, arena %d, tormento %d, semente %d."),
		ExpectedPlayers, ArenaIndex, Torment, RunSeed);
}

void ADSGameMode::StartPlay()
{
	Super::StartPlay();

	// Origem do anel dos heróis: o primeiro PlayerStart do mapa.
	for (TActorIterator<APlayerStart> It(GetWorld()); It; ++It)
	{
		HeroSpawnOrigin = It->GetActorTransform();
		HeroSpawnOrigin.SetScale3D(FVector::OneVector);
		break;
	}

	if (ADSGameState* DSGameState = GetDSGameState())
	{
		DSGameState->ServerSetWave(0, MaxWaves, false);
		DSGameState->ServerSetTeamXP(1, 0, DSRunRules::GetTeamXPToNext(1));
	}
	SetPhase(EDSMatchPhase::WaitingForPlayers, JoinTimeoutSec);
	GetWorldTimerManager().SetTimer(JoinTimeoutTimer, this, &ADSGameMode::HandleJoinTimeout, JoinTimeoutSec, false);

	bMatchInitialized = true;
	// O host de listen server faz login antes do StartPlay.
	TryStartRun();
}

FString ADSGameMode::InitNewPlayer(APlayerController* NewPlayerController, const FUniqueNetIdRepl& UniqueId, const FString& Options, const FString& Portal)
{
	const FString Error = Super::InitNewPlayer(NewPlayerController, UniqueId, Options, Portal);
	ADSPlayerState* DSPlayerState = NewPlayerController ? NewPlayerController->GetPlayerState<ADSPlayerState>() : nullptr;
	const FString PlayerId = UGameplayStatics::ParseOption(Options, DSGameModeOptions::PlayerId);
	if (DSPlayerState && !PlayerId.IsEmpty())
	{
		// TODO(Network): validar o PlayerId com o token da sessão no backend antes de confiar.
		DSPlayerState->SetBackendPlayerId(PlayerId.Left(128));
	}
	return Error;
}

void ADSGameMode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);
	GetWorldTimerManager().ClearTimer(EmptyMatchTimer);
	TryStartRun();
}

void ADSGameMode::HandleStartingNewPlayer_Implementation(APlayerController* NewPlayer)
{
	Super::HandleStartingNewPlayer_Implementation(NewPlayer);

	if (!bRunStarted || bRunFinished || !NewPlayer)
	{
		return;
	}
	// Entrada tardia / reconexão: nasce no anel e recebe as escolhas que o time já ganhou.
	ADSPlayerState* DSPlayerState = NewPlayer->GetPlayerState<ADSPlayerState>();
	if (DSPlayerState)
	{
		if (DSPlayerState->GetHeroClass() == EDSHeroClass::None || DSPlayerState->GetStance() == EDSStance::None)
		{
			DSPlayerState->SetHeroSelection(DefaultHeroClass, DefaultStance);
		}
		const ADSGameState* DSGameState = GetDSGameState();
		const int32 EarnedPicks = DSGameState ? DSGameState->GetTeamLevel() - 1 : 0;
		DSPlayerState->AddPendingCardPicks(EarnedPicks - DSPlayerState->GetPendingCardPicks());
	}
	const int32 RingIndex = GameState ? GameState->PlayerArray.IndexOfByKey(NewPlayer->PlayerState) : 0;
	SpawnHeroInRing(NewPlayer, FMath::Max(0, RingIndex), FMath::Max(RunPartySize, RingIndex + 1));
}

UClass* ADSGameMode::GetDefaultPawnClassForController_Implementation(AController* InController)
{
	const ADSPlayerState* DSPlayerState = InController ? InController->GetPlayerState<ADSPlayerState>() : nullptr;
	if (DSPlayerState)
	{
		if (const TSubclassOf<ADSHeroCharacter>* HeroPawn = HeroPawnClasses.Find(DSPlayerState->GetHeroClass()))
		{
			if (*HeroPawn)
			{
				return HeroPawn->Get();
			}
		}
	}
	return Super::GetDefaultPawnClassForController_Implementation(InController);
}

void ADSGameMode::Logout(AController* Exiting)
{
	Super::Logout(Exiting);

	if (bMatchClosing)
	{
		return;
	}
	if (CountConnectedPlayers(Exiting) == 0)
	{
		GetWorldTimerManager().SetTimer(EmptyMatchTimer, this, &ADSGameMode::HandleEmptyMatchTimeout, EmptyMatchTimeoutSec, false);
		return;
	}
	if (bRunStarted && !bRunFinished)
	{
		// Quem saiu não bloqueia a derrota: confere quando o controller já tiver saído.
		GetWorldTimerManager().SetTimerForNextTick(this, &ADSGameMode::CheckDefeat);
	}
	else
	{
		GetWorldTimerManager().SetTimerForNextTick(this, &ADSGameMode::TryStartRun);
	}
}

void ADSGameMode::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	GetWorldTimerManager().ClearAllTimersForObject(this);
	RemoveAllEnemies();
	Super::EndPlay(EndPlayReason);
}

// ---------------------------------------------------------------------------
// Espera e início da run
// ---------------------------------------------------------------------------

void ADSGameMode::NotifyPlayerReadyChanged(ADSPlayerController* Player)
{
	TryStartRun();
}

void ADSGameMode::TryStartRun()
{
	if (!bMatchInitialized || bRunStarted || bMatchClosing)
	{
		return;
	}
	const bool bEveryoneJoined = CountConnectedPlayers() >= ExpectedPlayers;
	if (bEveryoneJoined && (!bRequireReadyToStart || AreAllPlayersReady()))
	{
		StartRun();
	}
}

bool ADSGameMode::AreAllPlayersReady() const
{
	const TArray<ADSPlayerState*> Players = GetDSPlayerStates();
	for (const ADSPlayerState* Player : Players)
	{
		if (!Player->IsReady())
		{
			return false;
		}
	}
	return Players.Num() > 0;
}

void ADSGameMode::HandleJoinTimeout()
{
	if (bRunStarted)
	{
		return;
	}
	if (CountConnectedPlayers() > 0)
	{
		UE_LOG(LogDungeonStance, Log, TEXT("Timeout de entrada: começando com %d jogador(es)."), CountConnectedPlayers());
		StartRun();
		return;
	}
	UE_LOG(LogDungeonStance, Log, TEXT("Partida fechada: ninguém entrou."));
	CloseMatch();
}

void ADSGameMode::StartRun()
{
	bRunStarted = true;
	GetWorldTimerManager().ClearTimer(JoinTimeoutTimer);

	PrepareParticipants();
	SpawnHeroes();

	if (ADSGameState* DSGameState = GetDSGameState())
	{
		DSGameState->ServerSetTeamXP(1, 0, DSRunRules::GetTeamXPToNext(1));
	}
	UE_LOG(LogDungeonStance, Log, TEXT("Run iniciada: %d jogador(es), rank médio %.2f."), RunPartySize, AverageRank);

	SetPhase(EDSMatchPhase::BetweenWaves, WaveBreakSec);
	ScheduleNextWave(WaveBreakSec);
}

void ADSGameMode::PrepareParticipants()
{
	const TArray<ADSPlayerState*> Players = GetDSPlayerStates();
	int32 RankSum = 0;
	for (ADSPlayerState* Player : Players)
	{
		if (Player->GetHeroClass() == EDSHeroClass::None || Player->GetStance() == EDSStance::None)
		{
			Player->SetHeroSelection(DefaultHeroClass, DefaultStance);
		}
		Player->ResetRunData();
		RankSum += Player->GetClassRank();
	}
	AverageRank = Players.Num() > 0 ? static_cast<float>(RankSum) / Players.Num() : 1.f;
	RunPartySize = DSRunRules::ClampPartySize(Players.Num());
}

void ADSGameMode::SpawnHeroes()
{
	int32 RingIndex = 0;
	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		APlayerController* Player = It->Get();
		if (Player && Player->PlayerState && !Player->PlayerState->IsOnlyASpectator())
		{
			SpawnHeroInRing(Player, RingIndex++, RunPartySize);
		}
	}
}

void ADSGameMode::SpawnHeroInRing(APlayerController* Player, int32 RingIndex, int32 RingSize)
{
	if (!Player)
	{
		return;
	}
	if (APawn* OldPawn = Player->GetPawn())
	{
		Player->UnPossess();
		OldPawn->Destroy();
	}
	const FVector LocalOffset = DSRunRules::GetSpawnOffset(RingIndex, RingSize, HeroSpawnRadiusCm);
	const FVector Location = HeroSpawnOrigin.TransformPosition(LocalOffset);
	const FTransform SpawnTransform(HeroSpawnOrigin.GetRotation(), Location);
	RestartPlayerAtTransform(Player, SpawnTransform);
}

// ---------------------------------------------------------------------------
// Ondas
// ---------------------------------------------------------------------------

void ADSGameMode::StartNextWave()
{
	if (bRunFinished)
	{
		return;
	}
	++CurrentWave;
	bWaveActive = true;
	const bool bBoss = DSRunRules::IsBossWave(CurrentWave, MaxWaves);
	if (ADSGameState* DSGameState = GetDSGameState())
	{
		DSGameState->ServerSetWave(CurrentWave, MaxWaves, bBoss);
	}

	// Caídos levantam no início de toda onda; escolhas não feitas ficam pendentes para a próxima pausa.
	ReviveDownedHeroes();
	ClearAllCardChoices();
	KillsAtWaveStart.Reset();
	for (ADSPlayerState* Player : GetDSPlayerStates())
	{
		KillsAtWaveStart.Add(Player, Player->GetKills());
	}
	RunPartySize = DSRunRules::ClampPartySize(CountConnectedPlayers());

	FRandomStream WaveRandom = RunRules ? RunRules->MakeWaveRandom(CurrentWave) : FRandomStream(DSRunRules::GetWaveSeed(RunSeed, CurrentWave));
	SpawnWaveEnemies(WaveRandom);

	if (ADSGameState* DSGameState = GetDSGameState())
	{
		DSGameState->ServerSetEnemiesRemaining(LiveEnemies.Num());
	}
	SetPhase(EDSMatchPhase::WaveInProgress, WaveTimeoutSec);
	GetWorldTimerManager().SetTimer(WaveTimeoutTimer, this, &ADSGameMode::HandleWaveTimeout, WaveTimeoutSec, false);
	UE_LOG(LogDungeonStance, Log, TEXT("Onda %d/%d iniciada (%d inimigos%s)."), CurrentWave, MaxWaves, LiveEnemies.Num(), bBoss ? TEXT(", chefe") : TEXT(""));

	if (LiveEnemies.Num() == 0)
	{
		UE_LOG(LogDungeonStance, Error, TEXT("Onda %d sem inimigos: configure RegularEnemies/Bosses/EnemyCharacterClass no GameMode."), CurrentWave);
		GetWorldTimerManager().SetTimerForNextTick(this, &ADSGameMode::HandleWaveCleared);
	}
}

void ADSGameMode::SpawnWaveEnemies(FRandomStream& WaveRandom)
{
	const bool bBoss = DSRunRules::IsBossWave(CurrentWave, MaxWaves);
	const int32 Total = DSRunRules::GetScaledWaveEnemyCount(CurrentWave, MaxWaves, RunPartySize);
	const int32 RegularCount = Total - (bBoss ? 1 : 0);
	const TArray<FVector> HeroLocations = GetHeroLocations();
	const FVector Center = DSWaveSpawnHelpers::Centroid(HeroLocations, HeroSpawnOrigin.GetLocation());

	for (int32 Index = 0; Index < RegularCount; ++Index)
	{
		const FDSEnemySpawnEntry* Entry = DSWaveSpawnHelpers::PickRegularEntry(RegularEnemies, CurrentWave, WaveRandom);
		if (!Entry)
		{
			UE_LOG(LogDungeonStance, Error, TEXT("Nenhum inimigo regular liberado para a onda %d."), CurrentWave);
			break;
		}
		SpawnEnemy(Entry->EnemyData, Entry->bElite, false, Center, HeroLocations, WaveRandom);
	}

	if (bBoss)
	{
		const FDSBossSpawnEntry* BossEntry = DSWaveSpawnHelpers::FindBossEntry(Bosses, CurrentWave);
		if (BossEntry)
		{
			SpawnEnemy(BossEntry->EnemyData, BossEntry->bElite, true, Center, HeroLocations, WaveRandom);
		}
		else
		{
			UE_LOG(LogDungeonStance, Error, TEXT("Onda de chefe %d sem chefe configurado."), CurrentWave);
		}
	}
}

bool ADSGameMode::SpawnEnemy(const UDSEnemyData* Data, bool bElite, bool bBoss, const FVector& Center, const TArray<FVector>& HeroLocations, FRandomStream& WaveRandom)
{
	if (!Data || !EnemyCharacterClass)
	{
		UE_LOG(LogDungeonStance, Error, TEXT("SpawnEnemy: UDSEnemyData ou EnemyCharacterClass ausente."));
		return false;
	}
	const int32 Level = DSRunRules::GetArenaEffectiveLevel(ArenaIndex, Torment);
	const float BossScale = bBoss ? DSRunRules::GetPartyBossHealthMultiplier(RunPartySize) : 1.f;
	const float HealthMultiplier = DSRunRules::GetWaveHealthMultiplier(CurrentWave, AverageRank) * BossScale;
	const float DamageMultiplier = DSRunRules::GetWaveDamageMultiplier(CurrentWave, AverageRank);

	const ADSEnemyCharacter* DefaultEnemy = EnemyCharacterClass.GetDefaultObject();
	const UCapsuleComponent* Capsule = DefaultEnemy ? DefaultEnemy->GetCapsuleComponent() : nullptr;
	const float HalfHeight = Capsule ? Capsule->GetScaledCapsuleHalfHeight() : 0.f;

	FVector Location = DSWaveSpawnHelpers::FindEnemySpawnLocation(GetWorld(), Center, HeroLocations, EnemySpawnRing, WaveRandom);
	Location.Z += HalfHeight;
	const FRotator Facing(0.f, (Center - Location).Rotation().Yaw, 0.f);

	FActorSpawnParameters Params;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
	ADSEnemyCharacter* Enemy = GetWorld()->SpawnActor<ADSEnemyCharacter>(EnemyCharacterClass, Location, Facing, Params);
	if (!Enemy)
	{
		UE_LOG(LogDungeonStance, Warning, TEXT("Falha ao spawnar %s."), *GetNameSafe(Data));
		return false;
	}
	Enemy->InitializeFromData(Data, Level, HealthMultiplier, DamageMultiplier, bElite);
	Enemy->OnEnemyDied.AddUObject(this, &ADSGameMode::HandleEnemyDied);

	FDSLiveEnemyRecord Record;
	Record.Enemy = Enemy;
	Record.XP = ComputeEnemyXP(Enemy, HealthMultiplier, Level);
	LiveEnemies.Add(Record);
	return true;
}

int32 ADSGameMode::ComputeEnemyXP(ADSEnemyCharacter* Enemy, float HealthMultiplier, int32 EnemyLevel) const
{
	// XP vem da vida base do inimigo (sem a escala da onda), como no bestiary.ts.
	const UAbilitySystemComponent* ASC = Enemy ? Enemy->GetAbilitySystemComponent() : nullptr;
	bool bFound = false;
	const float MaxHealth = ASC ? ASC->GetGameplayAttributeValue(UDSAttributeSet::GetMaxHealthAttribute(), bFound) : 0.f;
	const bool bUsable = bFound && MaxHealth > 0.f && HealthMultiplier > 0.f;
	const float BaseHealth = bUsable ? MaxHealth / HealthMultiplier : DSRunRules::GetEnemyBaseHealth(EnemyLevel);
	return DSRunRules::GetEnemyCombatXP(BaseHealth);
}

void ADSGameMode::HandleEnemyDied(ADSEnemyCharacter* Enemy, AController* Killer)
{
	NotifyEnemyKilled(Enemy, Killer);
}

void ADSGameMode::NotifyEnemyKilled(ADSEnemyCharacter* Enemy, AController* Killer)
{
	const int32 Index = LiveEnemies.IndexOfByPredicate([Enemy](const FDSLiveEnemyRecord& Record) { return Record.Enemy.Get() == Enemy; });
	if (!Enemy || Index == INDEX_NONE)
	{
		return; // Já contado (delegate + chamada direta) ou não pertence à onda.
	}
	const int32 XP = LiveEnemies[Index].XP;
	LiveEnemies.RemoveAtSwap(Index);
	Enemy->OnEnemyDied.RemoveAll(this);

	if (ADSPlayerState* KillerState = Killer ? Killer->GetPlayerState<ADSPlayerState>() : nullptr)
	{
		KillerState->AddKill();
	}
	AddTeamXP(XP);

	PruneInvalidEnemies();
	if (ADSGameState* DSGameState = GetDSGameState())
	{
		DSGameState->ServerSetEnemiesRemaining(LiveEnemies.Num());
	}
	if (bWaveActive && LiveEnemies.Num() == 0)
	{
		HandleWaveCleared();
	}
}

void ADSGameMode::HandleWaveTimeout()
{
	if (!bWaveActive)
	{
		return;
	}
	UE_LOG(LogDungeonStance, Log, TEXT("Onda %d não foi limpa em %.0f s: encerrando a onda."), CurrentWave, WaveTimeoutSec);
	RemoveAllEnemies();
	HandleWaveCleared();
}

void ADSGameMode::HandleWaveCleared()
{
	if (!bWaveActive || bRunFinished)
	{
		return;
	}
	bWaveActive = false;
	GetWorldTimerManager().ClearTimer(WaveTimeoutTimer);
	WavesCleared = CurrentWave;
	AwardWaveGold();

	if (CurrentWave >= MaxWaves)
	{
		FinishRun(true);
		return;
	}

	// Cartas só são oferecidas entre ondas: pausa longa se alguém tem escolha pendente.
	RollCardChoicesForAll();
	const bool bPicking = AnyPendingPicks();
	const float BreakSec = bPicking ? PickBreakSec : WaveBreakSec;
	BreakMinServerTime = GetServerTime() + WaveBreakSec;
	SetPhase(bPicking ? EDSMatchPhase::CardPick : EDSMatchPhase::BetweenWaves, BreakSec);
	ScheduleNextWave(BreakSec);
}

void ADSGameMode::AwardWaveGold()
{
	for (ADSPlayerState* Player : GetDSPlayerStates())
	{
		const int32 KillsThisWave = Player->GetKills() - KillsAtWaveStart.FindRef(Player);
		Player->AddRunGold(DSRunRules::GetWaveGold(CurrentWave, KillsThisWave));
	}
}

void ADSGameMode::ReviveDownedHeroes()
{
	int32 RingIndex = 0;
	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		APlayerController* Player = It->Get();
		if (!Player || !Player->PlayerState || Player->PlayerState->IsOnlyASpectator())
		{
			continue;
		}
		ADSHeroCharacter* Hero = Cast<ADSHeroCharacter>(Player->GetPawn());
		if (!Hero)
		{
			SpawnHeroInRing(Player, RingIndex, RunPartySize); // Pawn destruído ao morrer.
		}
		else if (!IsHeroUp(Hero))
		{
			ReviveHero(Hero);
		}
		++RingIndex;
	}
	DownedHeroes.Reset();
}

void ADSGameMode::ReviveHero_Implementation(ADSHeroCharacter* Hero)
{
	UAbilitySystemComponent* ASC = Hero ? Hero->GetAbilitySystemComponent() : nullptr;
	if (!ASC)
	{
		return;
	}
	ASC->SetLooseGameplayTagCount(DSTags::State_Downed, 0);
	ASC->SetLooseGameplayTagCount(DSTags::State_Dead, 0);
	bool bFound = false;
	const float MaxHealth = ASC->GetGameplayAttributeValue(UDSAttributeSet::GetMaxHealthAttribute(), bFound);
	if (bFound)
	{
		ASC->SetNumericAttributeBase(UDSAttributeSet::GetHealthAttribute(), MaxHealth);
	}
}

void ADSGameMode::RemoveAllEnemies()
{
	for (const FDSLiveEnemyRecord& Record : LiveEnemies)
	{
		if (ADSEnemyCharacter* Enemy = Record.Enemy.Get())
		{
			Enemy->OnEnemyDied.RemoveAll(this);
			Enemy->Destroy();
		}
	}
	LiveEnemies.Reset();
	if (ADSGameState* DSGameState = GetDSGameState())
	{
		DSGameState->ServerSetEnemiesRemaining(0);
	}
}

void ADSGameMode::PruneInvalidEnemies()
{
	// Inimigo destruído sem morrer (caiu do mapa, streaming) não pode travar a onda.
	LiveEnemies.RemoveAllSwap([](const FDSLiveEnemyRecord& Record) { return !Record.Enemy.IsValid(); });
}

// ---------------------------------------------------------------------------
// XP do time e cartas
// ---------------------------------------------------------------------------

void ADSGameMode::AddTeamXP(int32 Amount)
{
	ADSGameState* DSGameState = GetDSGameState();
	if (!DSGameState)
	{
		return;
	}
	const DSRunRules::FTeamXPResult Result = DSRunRules::AddTeamXP(DSGameState->GetTeamXP(), DSGameState->GetTeamLevel(), Amount);
	DSGameState->ServerSetTeamXP(Result.Level, Result.XP, DSRunRules::GetTeamXPToNext(Result.Level));
	if (Result.LevelsGained <= 0)
	{
		return;
	}
	// Cada nível acima do 1 dá uma escolha de carta a cada jogador.
	for (ADSPlayerState* Player : GetDSPlayerStates())
	{
		Player->AddPendingCardPicks(Result.LevelsGained);
	}
}

void ADSGameMode::RollCardChoices(ADSPlayerState& Player)
{
	if (Player.GetPendingCardPicks() <= 0)
	{
		Player.SetCurrentCardChoices(TArray<FName>());
		return;
	}
	FRandomStream Fallback(RunSeed);
	FRandomStream& Random = RunRules ? RunRules->GetRunRandom() : Fallback;
	const TArray<FName> Choices = DSCardBridge::RollChoices(Player, NumCardChoices, Random);
	if (Choices.Num() == 0)
	{
		// Árvore inteira no máximo: as escolhas são consumidas em silêncio (squad_state.ts).
		Player.ClearPendingCardPicks();
	}
	Player.SetCurrentCardChoices(Choices);
}

void ADSGameMode::RollCardChoicesForAll()
{
	for (ADSPlayerState* Player : GetDSPlayerStates())
	{
		RollCardChoices(*Player);
	}
}

bool ADSGameMode::AnyPendingPicks() const
{
	for (const ADSPlayerState* Player : GetDSPlayerStates())
	{
		if (Player->GetPendingCardPicks() > 0 && Player->GetCurrentCardChoices().Num() > 0)
		{
			return true;
		}
	}
	return false;
}

void ADSGameMode::ClearAllCardChoices()
{
	for (ADSPlayerState* Player : GetDSPlayerStates())
	{
		Player->SetCurrentCardChoices(TArray<FName>());
	}
}

void ADSGameMode::NotifyCardPicked(ADSPlayerController* Player)
{
	ADSPlayerState* DSPlayerState = Player ? Player->GetDSPlayerState() : nullptr;
	if (!DSPlayerState || bWaveActive || bRunFinished)
	{
		return;
	}
	RollCardChoices(*DSPlayerState); // Novas opções se ainda houver escolha pendente.
	if (AnyPendingPicks())
	{
		return;
	}
	// Todos escolheram: próxima onda em 1 s, mas nunca antes da pausa mínima de 4 s.
	const float Now = GetServerTime();
	const float StartAt = FMath::Max(BreakMinServerTime, Now + AllPickedGraceSec);
	const float Delay = FMath::Max(StartAt - Now, KINDA_SMALL_NUMBER);
	if (Delay < GetWorldTimerManager().GetTimerRemaining(PhaseTimer))
	{
		SetPhase(EDSMatchPhase::BetweenWaves, Delay);
		ScheduleNextWave(Delay);
	}
}

// ---------------------------------------------------------------------------
// Derrota, vitória e encerramento
// ---------------------------------------------------------------------------

void ADSGameMode::NotifyHeroDowned(ADSHeroCharacter* Hero)
{
	if (!Hero || !bRunStarted || bRunFinished)
	{
		return;
	}
	DownedHeroes.Add(Hero);
	CheckDefeat();
}

bool ADSGameMode::IsHeroUp(const ADSHeroCharacter* Hero) const
{
	return Hero && Hero->IsAlive() && !DownedHeroes.Contains(Hero);
}

void ADSGameMode::CheckDefeat()
{
	if (!bRunStarted || bRunFinished)
	{
		return;
	}
	int32 Connected = 0;
	int32 Standing = 0;
	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		const APlayerController* Player = It->Get();
		if (!Player || !Player->PlayerState || Player->PlayerState->IsOnlyASpectator())
		{
			continue;
		}
		++Connected;
		Standing += IsHeroUp(Cast<ADSHeroCharacter>(Player->GetPawn())) ? 1 : 0;
	}
	if (Connected > 0 && Standing == 0)
	{
		FinishRun(false);
	}
}

void ADSGameMode::FinishRun(bool bVictory)
{
	if (bRunFinished)
	{
		return;
	}
	bRunFinished = true;
	bWaveActive = false;
	GetWorldTimerManager().ClearTimer(WaveTimeoutTimer);
	GetWorldTimerManager().ClearTimer(PhaseTimer);
	ClearAllCardChoices();

	// Só quem está conectado no fim recebe recompensa (anti-abandono).
	TArray<FDSRunResult> Results;
	for (ADSPlayerState* Player : GetDSPlayerStates())
	{
		if (bVictory)
		{
			Player->AddRunGold(DSRunRules::GoldVictoryBonus);
		}
		const FDSRunResult Result = BuildRunResult(*Player, bVictory);
		Player->SetRunResult(Result);
		Results.Add(Result);
	}
	UE_LOG(LogDungeonStance, Log, TEXT("Run terminada: %s, %d onda(s) limpa(s)."), bVictory ? TEXT("vitória") : TEXT("derrota"), WavesCleared);

	DSBackendBridge::SubmitRunResults(GetGameInstance(), Results);
	OnRunFinished.Broadcast(bVictory, Results);

	SetPhase(bVictory ? EDSMatchPhase::Victory : EDSMatchPhase::Defeat, EndScreenSec);
	GetWorldTimerManager().SetTimer(PhaseTimer, this, &ADSGameMode::HandleEndScreenFinished, EndScreenSec, false);
}

FDSRunResult ADSGameMode::BuildRunResult(const ADSPlayerState& Player, bool bVictory) const
{
	FDSRunResult Result;
	Result.PlayerId = Player.GetBackendPlayerId();
	Result.HeroClass = Player.GetHeroClass();
	Result.WavesCleared = WavesCleared;
	Result.Kills = Player.GetKills();
	Result.bVictory = bVictory;
	Result.RankXPEarned = DSRunRules::GetEndOfRunRankXP(WavesCleared, bVictory, Player.GetKills());
	Result.GoldEarned = Player.GetRunGold();
	return Result;
}

void ADSGameMode::HandleEndScreenFinished()
{
	SetPhase(EDSMatchPhase::Ended, 0.f);
	CloseMatch();
}

void ADSGameMode::HandleEmptyMatchTimeout()
{
	if (CountConnectedPlayers() > 0 || bMatchClosing)
	{
		return;
	}
	UE_LOG(LogDungeonStance, Log, TEXT("Partida abandonada na onda %d; sem recompensas."), CurrentWave);
	bRunFinished = true;
	SetPhase(EDSMatchPhase::Ended, 0.f);
	CloseMatch();
}

void ADSGameMode::CloseMatch_Implementation()
{
	if (bMatchClosing)
	{
		return;
	}
	bMatchClosing = true;
	GetWorldTimerManager().ClearAllTimersForObject(this);
	RemoveAllEnemies();

	if (!PostMatchTravelURL.IsEmpty())
	{
		GetWorld()->ServerTravel(PostMatchTravelURL);
	}
	else if (IsRunningDedicatedServer())
	{
		FPlatformMisc::RequestExit(false);
	}
	else if (UGameInstance* GameInstance = GetGameInstance())
	{
		GameInstance->ReturnToMainMenu();
	}
}

// ---------------------------------------------------------------------------
// Utilitários
// ---------------------------------------------------------------------------

void ADSGameMode::SetPhase(EDSMatchPhase Phase, float DurationSec)
{
	ADSGameState* DSGameState = GetDSGameState();
	if (!DSGameState)
	{
		return;
	}
	const float EndTime = DurationSec > 0.f ? GetServerTime() + DurationSec : 0.f;
	DSGameState->ServerSetMatchPhase(Phase, EndTime);
}

void ADSGameMode::ScheduleNextWave(float DelaySec)
{
	GetWorldTimerManager().SetTimer(PhaseTimer, this, &ADSGameMode::StartNextWave, DelaySec, false);
}

float ADSGameMode::GetServerTime() const
{
	const AGameStateBase* State = GameState;
	return State ? static_cast<float>(State->GetServerWorldTimeSeconds()) : GetWorld()->GetTimeSeconds();
}

int32 ADSGameMode::CountConnectedPlayers(const AController* Excluding) const
{
	int32 Count = 0;
	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		const APlayerController* Player = It->Get();
		if (Player && Player != Excluding && Player->PlayerState && !Player->PlayerState->IsOnlyASpectator())
		{
			++Count;
		}
	}
	return Count;
}

TArray<ADSPlayerState*> ADSGameMode::GetDSPlayerStates() const
{
	TArray<ADSPlayerState*> Result;
	if (!GameState)
	{
		return Result;
	}
	for (APlayerState* State : GameState->PlayerArray)
	{
		ADSPlayerState* DSPlayerState = Cast<ADSPlayerState>(State);
		if (DSPlayerState && !DSPlayerState->IsOnlyASpectator())
		{
			Result.Add(DSPlayerState);
		}
	}
	return Result;
}

TArray<FVector> ADSGameMode::GetHeroLocations() const
{
	TArray<FVector> Locations;
	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		const APlayerController* Player = It->Get();
		const ADSHeroCharacter* Hero = Player ? Cast<ADSHeroCharacter>(Player->GetPawn()) : nullptr;
		if (IsHeroUp(Hero))
		{
			Locations.Add(Hero->GetActorLocation());
		}
	}
	return Locations;
}

ADSGameState* ADSGameMode::GetDSGameState() const
{
	return GetGameState<ADSGameState>();
}
