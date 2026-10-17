#include "Gameplay/Abilities/DSAbilitySet.h"

#include "AbilitySystemComponent.h"
#include "GameplayEffect.h"
#include "Gameplay/Abilities/DSGameplayAbility.h"
#include "DungeonStance.h"

void FDSAbilitySetGrantedHandles::AddAbilitySpecHandle(const FGameplayAbilitySpecHandle& Handle)
{
	if (Handle.IsValid())
	{
		AbilitySpecHandles.Add(Handle);
	}
}

void FDSAbilitySetGrantedHandles::AddEffectHandle(const FActiveGameplayEffectHandle& Handle)
{
	if (Handle.IsValid())
	{
		EffectHandles.Add(Handle);
	}
}

void FDSAbilitySetGrantedHandles::TakeFromAbilitySystem(UAbilitySystemComponent* ASC)
{
	if (!ASC || !ASC->IsOwnerActorAuthoritative())
	{
		return;
	}
	for (const FGameplayAbilitySpecHandle& Handle : AbilitySpecHandles)
	{
		ASC->ClearAbility(Handle);
	}
	for (const FActiveGameplayEffectHandle& Handle : EffectHandles)
	{
		ASC->RemoveActiveGameplayEffect(Handle);
	}
	AbilitySpecHandles.Reset();
	EffectHandles.Reset();
}

void UDSAbilitySet::GiveToAbilitySystem(UAbilitySystemComponent* ASC, FDSAbilitySetGrantedHandles* OutHandles, UObject* SourceObject) const
{
	if (!ASC || !ASC->IsOwnerActorAuthoritative())
	{
		return; // conceder habilidades é só no servidor
	}

	for (const FDSAbilitySet_Attribute& Entry : InitialAttributes)
	{
		if (Entry.Attribute.IsValid() && ASC->HasAttributeSetForAttribute(Entry.Attribute))
		{
			ASC->SetNumericAttributeBase(Entry.Attribute, Entry.BaseValue);
		}
	}

	for (const FDSAbilitySet_Ability& Entry : GrantedAbilities)
	{
		if (!IsValid(Entry.Ability))
		{
			UE_LOG(LogDungeonStance, Warning, TEXT("AbilitySet %s: habilidade inválida."), *GetNameSafe(this));
			continue;
		}
		FGameplayAbilitySpec Spec(Entry.Ability, Entry.AbilityLevel, INDEX_NONE, SourceObject);
		if (Entry.InputTag.IsValid())
		{
			Spec.GetDynamicSpecSourceTags().AddTag(Entry.InputTag);
		}
		const FGameplayAbilitySpecHandle Handle = ASC->GiveAbility(Spec);
		if (OutHandles)
		{
			OutHandles->AddAbilitySpecHandle(Handle);
		}
	}

	for (const FDSAbilitySet_Effect& Entry : GrantedEffects)
	{
		if (!IsValid(Entry.Effect))
		{
			continue;
		}
		const UGameplayEffect* EffectCDO = Entry.Effect->GetDefaultObject<UGameplayEffect>();
		const FActiveGameplayEffectHandle Handle = ASC->ApplyGameplayEffectToSelf(EffectCDO, Entry.EffectLevel, ASC->MakeEffectContext());
		if (OutHandles)
		{
			OutHandles->AddEffectHandle(Handle);
		}
	}
}
