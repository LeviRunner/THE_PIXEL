#include "Core/DSGameState.h"

#include "Core/DSRunRules.h"
#include "Net/UnrealNetwork.h"

ADSGameState::ADSGameState(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	TeamXPToNext = DSRunRules::GetTeamXPToNext(TeamLevel);
}

void ADSGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ADSGameState, CurrentWave);
	DOREPLIFETIME(ADSGameState, MaxWaves);
	DOREPLIFETIME(ADSGameState, bIsBossWave);
	DOREPLIFETIME(ADSGameState, MatchPhase);
	DOREPLIFETIME(ADSGameState, TeamLevel);
	DOREPLIFETIME(ADSGameState, TeamXP);
	DOREPLIFETIME(ADSGameState, TeamXPToNext);
	DOREPLIFETIME(ADSGameState, PhaseEndServerTime);
	DOREPLIFETIME(ADSGameState, EnemiesRemaining);
}

float ADSGameState::GetPhaseTimeRemaining() const
{
	if (PhaseEndServerTime <= 0.f)
	{
		return 0.f;
	}
	return FMath::Max(0.f, PhaseEndServerTime - static_cast<float>(GetServerWorldTimeSeconds()));
}

void ADSGameState::ServerSetWave(int32 InWave, int32 InMaxWaves, bool bInIsBossWave)
{
	check(HasAuthority());
	CurrentWave = InWave;
	MaxWaves = InMaxWaves;
	bIsBossWave = bInIsBossWave;
	OnRep_Wave();
}

void ADSGameState::ServerSetMatchPhase(EDSMatchPhase InPhase, float InPhaseEndServerTime)
{
	check(HasAuthority());
	// O prazo vai antes da fase: quem escuta OnMatchPhaseChanged já lê o timer novo.
	PhaseEndServerTime = InPhaseEndServerTime;
	MatchPhase = InPhase;
	OnRep_MatchPhase();
}

void ADSGameState::ServerSetPhaseEndTime(float InPhaseEndServerTime)
{
	check(HasAuthority());
	PhaseEndServerTime = InPhaseEndServerTime;
}

void ADSGameState::ServerSetTeamXP(int32 InLevel, int32 InXP, int32 InXPToNext)
{
	check(HasAuthority());
	TeamLevel = InLevel;
	TeamXP = InXP;
	TeamXPToNext = InXPToNext;
	OnRep_TeamXP();
}

void ADSGameState::ServerSetEnemiesRemaining(int32 InRemaining)
{
	check(HasAuthority());
	if (EnemiesRemaining == InRemaining)
	{
		return;
	}
	EnemiesRemaining = InRemaining;
	OnRep_EnemiesRemaining();
}

void ADSGameState::OnRep_Wave()
{
	OnWaveChanged.Broadcast(CurrentWave, bIsBossWave);
}

void ADSGameState::OnRep_MatchPhase()
{
	OnMatchPhaseChanged.Broadcast(MatchPhase);
}

void ADSGameState::OnRep_TeamXP()
{
	OnTeamXPChanged.Broadcast(TeamLevel, TeamXP, TeamXPToNext);
}

void ADSGameState::OnRep_EnemiesRemaining()
{
	OnEnemiesRemainingChanged.Broadcast(EnemiesRemaining);
}
