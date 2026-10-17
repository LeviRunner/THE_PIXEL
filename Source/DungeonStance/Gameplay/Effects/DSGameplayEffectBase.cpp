#include "Gameplay/Effects/DSGameplayEffectBase.h"

#include "GameplayEffectComponents/AssetTagsGameplayEffectComponent.h"
#include "GameplayEffectComponents/TargetTagsGameplayEffectComponent.h"
#include "GameplayEffectComponents/TargetTagRequirementsGameplayEffectComponent.h"

void UDSGameplayEffectBase::AddGrantedTags(const FGameplayTagContainer& Tags)
{
	UTargetTagsGameplayEffectComponent* Component = FindOrCreateComponent<UTargetTagsGameplayEffectComponent>(TEXT("DSTargetTags"));
	FInheritedTagContainer Container = Component->GetConfiguredTargetTagChanges();
	for (const FGameplayTag& Tag : Tags)
	{
		Container.AddTag(Tag);
	}
	Component->SetAndApplyTargetTagChanges(Container);
}

void UDSGameplayEffectBase::AddAssetTags(const FGameplayTagContainer& Tags)
{
	UAssetTagsGameplayEffectComponent* Component = FindOrCreateComponent<UAssetTagsGameplayEffectComponent>(TEXT("DSAssetTags"));
	FInheritedTagContainer Container = Component->GetConfiguredAssetTagChanges();
	for (const FGameplayTag& Tag : Tags)
	{
		Container.AddTag(Tag);
	}
	Component->SetAndApplyAssetTagChanges(Container);
}

void UDSGameplayEffectBase::AddOngoingIgnoreTags(const FGameplayTagContainer& Tags)
{
	UTargetTagRequirementsGameplayEffectComponent* Component = FindOrCreateComponent<UTargetTagRequirementsGameplayEffectComponent>(TEXT("DSTagRequirements"));
	Component->OngoingTagRequirements.IgnoreTags.AppendTags(Tags);
}

void UDSGameplayEffectBase::SetDurationBySetByCaller(FGameplayTag DataTag)
{
	DurationPolicy = EGameplayEffectDurationType::HasDuration;
	FSetByCallerFloat SetByCaller;
	SetByCaller.DataTag = DataTag;
	DurationMagnitude = FGameplayEffectModifierMagnitude(SetByCaller);
}

void UDSGameplayEffectBase::AddSetByCallerModifier(const FGameplayAttribute& Attribute, EGameplayModOp::Type Op, FGameplayTag DataTag)
{
	FSetByCallerFloat SetByCaller;
	SetByCaller.DataTag = DataTag;

	FGameplayModifierInfo Info;
	Info.Attribute = Attribute;
	Info.ModifierOp = Op;
	Info.ModifierMagnitude = FGameplayEffectModifierMagnitude(SetByCaller);
	Modifiers.Add(Info);
}

void UDSGameplayEffectBase::AddScalarModifier(const FGameplayAttribute& Attribute, EGameplayModOp::Type Op, float Value)
{
	FGameplayModifierInfo Info;
	Info.Attribute = Attribute;
	Info.ModifierOp = Op;
	Info.ModifierMagnitude = FGameplayEffectModifierMagnitude(FScalableFloat(Value));
	Modifiers.Add(Info);
}

void UDSGameplayEffectBase::AddTargetAttributeBasedModifier(const FGameplayAttribute& Attribute, EGameplayModOp::Type Op, const FGameplayAttribute& Backing, float Coefficient)
{
	FAttributeBasedFloat AttributeBased;
	AttributeBased.Coefficient = FScalableFloat(Coefficient);
	AttributeBased.BackingAttribute = FGameplayEffectAttributeCaptureDefinition(Backing, EGameplayEffectAttributeCaptureSource::Target, /*bSnapshot*/ false);

	FGameplayModifierInfo Info;
	Info.Attribute = Attribute;
	Info.ModifierOp = Op;
	Info.ModifierMagnitude = FGameplayEffectModifierMagnitude(AttributeBased);
	Modifiers.Add(Info);
}

void UDSGameplayEffectBase::SetTargetStacking(int32 Limit)
{
	// Em versões recentes da engine estes campos podem estar marcados como deprecated (acesso via getters).
	PRAGMA_DISABLE_DEPRECATION_WARNINGS
	StackingType = EGameplayEffectStackingType::AggregateByTarget;
	StackLimitCount = FMath::Max(1, Limit);
	StackDurationRefreshPolicy = EGameplayEffectStackingDurationPolicy::RefreshOnSuccessfulApplication;
	StackPeriodResetPolicy = EGameplayEffectStackingPeriodPolicy::NeverReset;
	StackExpirationPolicy = EGameplayEffectStackingExpirationPolicy::RemoveSingleStackAndRefreshDuration;
	PRAGMA_ENABLE_DEPRECATION_WARNINGS
}

void UDSGameplayEffectBase::SetSourceStacking(int32 Limit)
{
	PRAGMA_DISABLE_DEPRECATION_WARNINGS
	StackingType = EGameplayEffectStackingType::AggregateBySource;
	StackLimitCount = FMath::Max(1, Limit);
	StackDurationRefreshPolicy = EGameplayEffectStackingDurationPolicy::RefreshOnSuccessfulApplication;
	StackPeriodResetPolicy = EGameplayEffectStackingPeriodPolicy::NeverReset;
	StackExpirationPolicy = EGameplayEffectStackingExpirationPolicy::ClearEntireStack;
	PRAGMA_ENABLE_DEPRECATION_WARNINGS
}
