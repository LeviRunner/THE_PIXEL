#include "Core/DSGameplayTags.h"

namespace DSTags
{
	UE_DEFINE_GAMEPLAY_TAG(Input_Attack, "Input.Attack");
	UE_DEFINE_GAMEPLAY_TAG(Input_ClassSkill, "Input.ClassSkill");
	UE_DEFINE_GAMEPLAY_TAG(Input_Dodge, "Input.Dodge");
	UE_DEFINE_GAMEPLAY_TAG(Input_Sprint, "Input.Sprint");
	UE_DEFINE_GAMEPLAY_TAG(Input_Support1, "Input.Support.1");
	UE_DEFINE_GAMEPLAY_TAG(Input_Support2, "Input.Support.2");
	UE_DEFINE_GAMEPLAY_TAG(Input_Support3, "Input.Support.3");
	UE_DEFINE_GAMEPLAY_TAG(Input_Support4, "Input.Support.4");
	UE_DEFINE_GAMEPLAY_TAG(Input_TargetLock, "Input.TargetLock");

	UE_DEFINE_GAMEPLAY_TAG(Ability_Attack, "Ability.Attack");
	UE_DEFINE_GAMEPLAY_TAG(Ability_ClassSkill, "Ability.ClassSkill");
	UE_DEFINE_GAMEPLAY_TAG(Ability_Dodge, "Ability.Dodge");
	UE_DEFINE_GAMEPLAY_TAG(Ability_Support, "Ability.Support");

	UE_DEFINE_GAMEPLAY_TAG(State_Dead, "State.Dead");
	UE_DEFINE_GAMEPLAY_TAG(State_Downed, "State.Downed");
	UE_DEFINE_GAMEPLAY_TAG(State_Invulnerable, "State.Invulnerable");
	UE_DEFINE_GAMEPLAY_TAG(State_Sprinting, "State.Sprinting");

	UE_DEFINE_GAMEPLAY_TAG(Status_Bleed, "Status.Bleed");
	UE_DEFINE_GAMEPLAY_TAG(Status_Burn, "Status.Burn");
	UE_DEFINE_GAMEPLAY_TAG(Status_Poison, "Status.Poison");
	UE_DEFINE_GAMEPLAY_TAG(Status_Mark, "Status.Mark");
	UE_DEFINE_GAMEPLAY_TAG(Status_Slow, "Status.Slow");
	UE_DEFINE_GAMEPLAY_TAG(Status_Stun, "Status.Stun");
	UE_DEFINE_GAMEPLAY_TAG(Status_Weaken, "Status.Weaken");
	UE_DEFINE_GAMEPLAY_TAG(Status_Exposed, "Status.Exposed");
	UE_DEFINE_GAMEPLAY_TAG(Status_Taunt, "Status.Taunt");

	UE_DEFINE_GAMEPLAY_TAG(Cooldown_Dodge, "Cooldown.Dodge");
	UE_DEFINE_GAMEPLAY_TAG(Cooldown_ClassSkill, "Cooldown.ClassSkill");

	UE_DEFINE_GAMEPLAY_TAG(Data_Damage, "Data.Damage");
	UE_DEFINE_GAMEPLAY_TAG(Data_Heal, "Data.Heal");
	UE_DEFINE_GAMEPLAY_TAG(Data_Duration, "Data.Duration");
	UE_DEFINE_GAMEPLAY_TAG(Data_Magnitude, "Data.Magnitude");

	UE_DEFINE_GAMEPLAY_TAG(Event_Hit, "Event.Hit");
	UE_DEFINE_GAMEPLAY_TAG(Event_Kill, "Event.Kill");
	UE_DEFINE_GAMEPLAY_TAG(Event_Death, "Event.Death");
}
