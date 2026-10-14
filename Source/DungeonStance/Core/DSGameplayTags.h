#pragma once

#include "NativeGameplayTags.h"

// Tags nativas compartilhadas. Tags específicas de uma área podem ficar num
// arquivo próprio dessa área; tags usadas por mais de uma área ficam aqui.
namespace DSTags
{
	// Input → habilidades (o ASC ativa a habilidade com a tag de input correspondente)
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Input_Attack);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Input_ClassSkill);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Input_Dodge);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Input_Sprint);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Input_Support1);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Input_Support2);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Input_Support3);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Input_Support4);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Input_TargetLock);

	// Habilidades
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Ability_Attack);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Ability_ClassSkill);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Ability_Dodge);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Ability_Support);

	// Estados
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(State_Dead);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(State_Downed);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(State_Invulnerable);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(State_Sprinting);

	// Status (status.ts)
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Status_Bleed);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Status_Burn);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Status_Poison);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Status_Mark);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Status_Slow);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Status_Stun);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Status_Weaken);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Status_Exposed);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Status_Taunt);

	// Cooldowns
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Cooldown_Dodge);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Cooldown_ClassSkill);

	// Valores passados por SetByCaller
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Data_Damage);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Data_Heal);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Data_Duration);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Data_Magnitude);

	// Eventos de gameplay
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Event_Hit);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Event_Kill);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Event_Death);
}
