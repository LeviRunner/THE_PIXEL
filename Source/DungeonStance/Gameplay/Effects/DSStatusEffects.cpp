#include "Gameplay/Effects/DSStatusEffects.h"

#include "Gameplay/Attributes/DSAttributeSet.h"
#include "Core/DSGameplayTags.h"

void UDSGE_TimedStatus::InitTimedStatus(FGameplayTag StatusTag)
{
	SetDurationBySetByCaller(DSTags::Data_Duration);
	AddGrantedTags(FGameplayTagContainer(StatusTag));
	AddAssetTags(FGameplayTagContainer(StatusTag));
}

void UDSGE_DamageOverTime::InitDot(FGameplayTag StatusTag, int32 MaxStacks)
{
	InitTimedStatus(StatusTag);
	Period = FScalableFloat(TickInterval);
	bExecutePeriodicEffectOnApplication = false;
	AddSetByCallerModifier(UDSAttributeSet::GetIncomingDamageAttribute(), EGameplayModOp::Additive, DSTags::Data_Damage);
	SetTargetStacking(MaxStacks);
}

UDSGE_Bleed::UDSGE_Bleed() { InitDot(DSTags::Status_Bleed, MaxStacks); }
UDSGE_Burn::UDSGE_Burn() { InitDot(DSTags::Status_Burn, MaxStacks); }
UDSGE_Poison::UDSGE_Poison() { InitDot(DSTags::Status_Poison, MaxStacks); }

UDSGE_Mark::UDSGE_Mark() { InitTimedStatus(DSTags::Status_Mark); }
UDSGE_Exposed::UDSGE_Exposed() { InitTimedStatus(DSTags::Status_Exposed); }
UDSGE_Weaken::UDSGE_Weaken() { InitTimedStatus(DSTags::Status_Weaken); }
UDSGE_Slow::UDSGE_Slow() { InitTimedStatus(DSTags::Status_Slow); }
UDSGE_Stun::UDSGE_Stun() { InitTimedStatus(DSTags::Status_Stun); }
UDSGE_Taunt::UDSGE_Taunt() { InitTimedStatus(DSTags::Status_Taunt); }
