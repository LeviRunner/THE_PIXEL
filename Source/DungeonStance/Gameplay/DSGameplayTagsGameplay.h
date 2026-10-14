#pragma once

#include "NativeGameplayTags.h"

// Tags usadas só pela área de Gameplay (as compartilhadas ficam em Core/DSGameplayTags.h).
namespace DSGameplayTags
{
	// Identidade dos personagens (tags soltas no ASC)
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Character_Hero);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Character_Enemy);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Character_Elite);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Character_Boss);

	// Habilidades extras
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Ability_Sprint);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Ability_ClassSkill_Whirlwind);

	// Recargas extras
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Cooldown_Attack);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Cooldown_Support);
	// Uma por habilidade de apoio (abilities.ts), para recargas independentes.
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Cooldown_Support_LostWarriorAura);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Cooldown_Support_SharedBastion);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Cooldown_Support_RallyingPulse);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Cooldown_Support_BattleCry);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Cooldown_Support_WarBanner);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Cooldown_Support_Sanctuary);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Cooldown_Support_WardTotem);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Cooldown_Support_TacticalCall);

	// Estado de combate
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(State_Dodging);

	// Buffs de apoio (SetByCaller do UDSGE_SupportBuff)
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Buff_Support);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Data_Buff_Damage);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Data_Buff_AttackSpeed);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Data_Buff_MoveSpeed);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Data_Buff_Armor);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Data_Buff_DamageReduction);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Data_Buff_CritChance);

	// Regeneração (UDSGE_Regen)
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Data_Regen_Resource);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Data_Regen_Stamina);

	// Eventos
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Event_Damaged);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Event_Crit);

	/**
	 * Tag SetByCaller das cartas para um atributo: "Data.Card.<Atributo>.Add" / ".Mult".
	 * As tags são nativas (definidas no .cpp para cada atributo de DSCardStatBindings).
	 * Retorna tag inválida para atributos não mapeados.
	 */
	FGameplayTag GetCardSetByCallerTag(FName AttributeName, bool bMultiplier);
}
