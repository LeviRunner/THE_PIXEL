#include "Gameplay/DSGameplayTagsGameplay.h"

namespace DSGameplayTags
{
	UE_DEFINE_GAMEPLAY_TAG(Character_Hero, "Character.Hero");
	UE_DEFINE_GAMEPLAY_TAG(Character_Enemy, "Character.Enemy");
	UE_DEFINE_GAMEPLAY_TAG(Character_Elite, "Character.Elite");
	UE_DEFINE_GAMEPLAY_TAG(Character_Boss, "Character.Boss");

	UE_DEFINE_GAMEPLAY_TAG(Ability_Sprint, "Ability.Sprint");
	UE_DEFINE_GAMEPLAY_TAG(Ability_ClassSkill_Whirlwind, "Ability.ClassSkill.Whirlwind");

	UE_DEFINE_GAMEPLAY_TAG(Cooldown_Attack, "Cooldown.Attack");
	UE_DEFINE_GAMEPLAY_TAG(Cooldown_Support, "Cooldown.Support");
	UE_DEFINE_GAMEPLAY_TAG(Cooldown_Support_LostWarriorAura, "Cooldown.Support.LostWarriorAura");
	UE_DEFINE_GAMEPLAY_TAG(Cooldown_Support_SharedBastion, "Cooldown.Support.SharedBastion");
	UE_DEFINE_GAMEPLAY_TAG(Cooldown_Support_RallyingPulse, "Cooldown.Support.RallyingPulse");
	UE_DEFINE_GAMEPLAY_TAG(Cooldown_Support_BattleCry, "Cooldown.Support.BattleCry");
	UE_DEFINE_GAMEPLAY_TAG(Cooldown_Support_WarBanner, "Cooldown.Support.WarBanner");
	UE_DEFINE_GAMEPLAY_TAG(Cooldown_Support_Sanctuary, "Cooldown.Support.Sanctuary");
	UE_DEFINE_GAMEPLAY_TAG(Cooldown_Support_WardTotem, "Cooldown.Support.WardTotem");
	UE_DEFINE_GAMEPLAY_TAG(Cooldown_Support_TacticalCall, "Cooldown.Support.TacticalCall");

	UE_DEFINE_GAMEPLAY_TAG(State_Dodging, "State.Dodging");

	UE_DEFINE_GAMEPLAY_TAG(Buff_Support, "Buff.Support");
	UE_DEFINE_GAMEPLAY_TAG(Data_Buff_Damage, "Data.Buff.Damage");
	UE_DEFINE_GAMEPLAY_TAG(Data_Buff_AttackSpeed, "Data.Buff.AttackSpeed");
	UE_DEFINE_GAMEPLAY_TAG(Data_Buff_MoveSpeed, "Data.Buff.MoveSpeed");
	UE_DEFINE_GAMEPLAY_TAG(Data_Buff_Armor, "Data.Buff.Armor");
	UE_DEFINE_GAMEPLAY_TAG(Data_Buff_DamageReduction, "Data.Buff.DamageReduction");
	UE_DEFINE_GAMEPLAY_TAG(Data_Buff_CritChance, "Data.Buff.CritChance");

	UE_DEFINE_GAMEPLAY_TAG(Data_Regen_Resource, "Data.Regen.Resource");
	UE_DEFINE_GAMEPLAY_TAG(Data_Regen_Stamina, "Data.Regen.Stamina");

	UE_DEFINE_GAMEPLAY_TAG(Event_Damaged, "Event.Damaged");
	UE_DEFINE_GAMEPLAY_TAG(Event_Crit, "Event.Crit");

	// Tags SetByCaller das cartas: uma por atributo e operação.
	// Manter em sincronia com DSCardStatBindings (Cards/DSCardLibrary.cpp).
#define DS_CARD_TAG_PAIR(Attr) \
	UE_DEFINE_GAMEPLAY_TAG_STATIC(Card_##Attr##_Add, "Data.Card." #Attr ".Add"); \
	UE_DEFINE_GAMEPLAY_TAG_STATIC(Card_##Attr##_Mult, "Data.Card." #Attr ".Mult");

	DS_CARD_TAG_PAIR(MaxHealth)
	DS_CARD_TAG_PAIR(Damage)
	DS_CARD_TAG_PAIR(AttackSpeed)
	DS_CARD_TAG_PAIR(AttackRange)
	DS_CARD_TAG_PAIR(MoveSpeed)
	DS_CARD_TAG_PAIR(CritChance)
	DS_CARD_TAG_PAIR(CritDamage)
	DS_CARD_TAG_PAIR(Armor)
	DS_CARD_TAG_PAIR(CooldownReduction)
	DS_CARD_TAG_PAIR(MaxResource)
	DS_CARD_TAG_PAIR(Lifesteal)
	DS_CARD_TAG_PAIR(HealthRegen)
	DS_CARD_TAG_PAIR(ResourceRegen)
	DS_CARD_TAG_PAIR(DamageReduction)
	DS_CARD_TAG_PAIR(DodgeChance)

#undef DS_CARD_TAG_PAIR

	FGameplayTag GetCardSetByCallerTag(FName AttributeName, bool bMultiplier)
	{
		// Lê direto dos FNativeGameplayTag (seguro mesmo durante a construção de CDOs).
		struct FPair { const FNativeGameplayTag* Add; const FNativeGameplayTag* Mult; };
#define DS_CARD_TAG_ENTRY(Attr) { FName(TEXT(#Attr)), FPair{ &Card_##Attr##_Add, &Card_##Attr##_Mult } },
		static const TMap<FName, FPair> Table = {
			DS_CARD_TAG_ENTRY(MaxHealth)
			DS_CARD_TAG_ENTRY(Damage)
			DS_CARD_TAG_ENTRY(AttackSpeed)
			DS_CARD_TAG_ENTRY(AttackRange)
			DS_CARD_TAG_ENTRY(MoveSpeed)
			DS_CARD_TAG_ENTRY(CritChance)
			DS_CARD_TAG_ENTRY(CritDamage)
			DS_CARD_TAG_ENTRY(Armor)
			DS_CARD_TAG_ENTRY(CooldownReduction)
			DS_CARD_TAG_ENTRY(MaxResource)
			DS_CARD_TAG_ENTRY(Lifesteal)
			DS_CARD_TAG_ENTRY(HealthRegen)
			DS_CARD_TAG_ENTRY(ResourceRegen)
			DS_CARD_TAG_ENTRY(DamageReduction)
			DS_CARD_TAG_ENTRY(DodgeChance)
		};
#undef DS_CARD_TAG_ENTRY
		const FPair* Found = Table.Find(AttributeName);
		if (!Found)
		{
			return FGameplayTag();
		}
		return bMultiplier ? Found->Mult->GetTag() : Found->Add->GetTag();
	}
}
