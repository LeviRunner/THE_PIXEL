#include "UI/HUD/DSPartyMemberWidget.h"

#include "AbilitySystemComponent.h"
#include "Components/Image.h"
#include "Components/PanelWidget.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "Core/DSGameplayTags.h"
#include "Core/DSPlayerState.h"
#include "GameplayEffectTypes.h"
#include "Gameplay/Attributes/DSAttributeSet.h"
#include "UI/DSUIDataAsset.h"
#include "UI/DSUIStateAccess.h"
#include "UI/DSUIStatics.h"
#include "UI/HUD/DSStatusIconWidget.h"

#define LOCTEXT_NAMESPACE "DungeonStanceUI"

void UDSPartyMemberWidget::SetPlayerState(ADSPlayerState* InPlayerState)
{
	if (PlayerState.Get() == InPlayerState)
	{
		return;
	}
	UnbindFromPlayer();
	PlayerState = InPlayerState;
	BindToPlayer();
}

void UDSPartyMemberWidget::NativeDestruct()
{
	UnbindFromPlayer();
	Super::NativeDestruct();
}

TArray<FGameplayAttribute> UDSPartyMemberWidget::GetWatchedAttributes()
{
	return {
		UDSAttributeSet::GetHealthAttribute(),
		UDSAttributeSet::GetMaxHealthAttribute(),
		UDSAttributeSet::GetShieldAttribute(),
		UDSAttributeSet::GetResourceAttribute(),
		UDSAttributeSet::GetMaxResourceAttribute()
	};
}

FGameplayTagContainer UDSPartyMemberWidget::GetWatchedStatusTags() const
{
	FGameplayTagContainer Tags;
	if (const UDSUIDataAsset* UIData = DSUI::GetUIData(this))
	{
		for (const FDSStatusUIInfo& Status : UIData->GetStatuses())
		{
			Tags.AddTag(Status.StatusTag);
		}
	}
	return Tags;
}

FGameplayTagContainer UDSPartyMemberWidget::GetDownedTags()
{
	FGameplayTagContainer Tags;
	Tags.AddTag(DSTags::State_Downed);
	Tags.AddTag(DSTags::State_Dead);
	return Tags;
}

void UDSPartyMemberWidget::BindToPlayer()
{
	ADSPlayerState* PS = PlayerState.Get();
	if (!PS)
	{
		SetVisibility(ESlateVisibility::Collapsed);
		return;
	}
	SetVisibility(ESlateVisibility::SelfHitTestInvisible);

	PS->OnHeroSelectionChanged.AddDynamic(this, &ThisClass::HandleHeroSelectionChanged);

	UAbilitySystemComponent* ASC = DSUIState::GetAbilitySystem(PS);
	AbilitySystem = ASC;
	if (ASC)
	{
		for (const FGameplayAttribute& Attribute : GetWatchedAttributes())
		{
			ASC->GetGameplayAttributeValueChangeDelegate(Attribute).AddUObject(this, &ThisClass::HandleAttributeChanged);
		}
		for (const FGameplayTag& Tag : GetDownedTags())
		{
			ASC->RegisterGameplayTagEvent(Tag, EGameplayTagEventType::NewOrRemoved).AddUObject(this, &ThisClass::HandleStateTagChanged);
		}
		BoundStatusTags = GetWatchedStatusTags();
		for (const FGameplayTag& Tag : BoundStatusTags)
		{
			// AnyCountChange para atualizar o número de acúmulos.
			ASC->RegisterGameplayTagEvent(Tag, EGameplayTagEventType::AnyCountChange).AddUObject(this, &ThisClass::HandleStatusTagChanged);
		}
	}

	const APlayerState* LocalPlayerState = GetOwningPlayerState();
	BP_OnPlayerChanged(LocalPlayerState == PS);

	RefreshIdentity();
	RefreshVitals();
	RefreshDowned();
	RefreshStatuses();
}

void UDSPartyMemberWidget::UnbindFromPlayer()
{
	if (ADSPlayerState* PS = PlayerState.Get())
	{
		PS->OnHeroSelectionChanged.RemoveDynamic(this, &ThisClass::HandleHeroSelectionChanged);
	}
	if (UAbilitySystemComponent* ASC = AbilitySystem.Get())
	{
		for (const FGameplayAttribute& Attribute : GetWatchedAttributes())
		{
			ASC->GetGameplayAttributeValueChangeDelegate(Attribute).RemoveAll(this);
		}
		for (const FGameplayTag& Tag : GetDownedTags())
		{
			ASC->RegisterGameplayTagEvent(Tag, EGameplayTagEventType::NewOrRemoved).RemoveAll(this);
		}
		for (const FGameplayTag& Tag : BoundStatusTags)
		{
			ASC->RegisterGameplayTagEvent(Tag, EGameplayTagEventType::AnyCountChange).RemoveAll(this);
		}
	}
	BoundStatusTags.Reset();
	PlayerState.Reset();
	AbilitySystem.Reset();
	bWasDowned = false;
}

void UDSPartyMemberWidget::HandleAttributeChanged(const FOnAttributeChangeData& Data)
{
	RefreshVitals();
}

void UDSPartyMemberWidget::HandleStateTagChanged(const FGameplayTag Tag, int32 NewCount)
{
	RefreshDowned();
}

void UDSPartyMemberWidget::HandleStatusTagChanged(const FGameplayTag Tag, int32 NewCount)
{
	RefreshStatuses();
}

void UDSPartyMemberWidget::HandleHeroSelectionChanged()
{
	RefreshIdentity();
}

void UDSPartyMemberWidget::RefreshVitals()
{
	const UAbilitySystemComponent* ASC = AbilitySystem.Get();
	if (!ASC)
	{
		return;
	}
	const float Health = ASC->GetNumericAttribute(UDSAttributeSet::GetHealthAttribute());
	const float MaxHealth = ASC->GetNumericAttribute(UDSAttributeSet::GetMaxHealthAttribute());
	const float Shield = ASC->GetNumericAttribute(UDSAttributeSet::GetShieldAttribute());
	const float Resource = ASC->GetNumericAttribute(UDSAttributeSet::GetResourceAttribute());
	const float MaxResource = ASC->GetNumericAttribute(UDSAttributeSet::GetMaxResourceAttribute());

	HealthBar->SetPercent(DSUI::SafeRatio(Health, MaxHealth));
	if (ShieldBar)
	{
		// Escudo é exibido em proporção à vida máxima.
		ShieldBar->SetPercent(DSUI::SafeRatio(Shield, MaxHealth));
		DSUI::SetWidgetShown(ShieldBar, Shield > 0.f);
	}
	if (ResourceBar)
	{
		ResourceBar->SetPercent(DSUI::SafeRatio(Resource, MaxResource));
	}
	if (HealthText)
	{
		HealthText->SetText(FText::Format(LOCTEXT("HealthFormat", "{0} / {1}"),
			FText::AsNumber(FMath::CeilToInt(Health)), FText::AsNumber(FMath::CeilToInt(MaxHealth))));
	}
}

void UDSPartyMemberWidget::RefreshIdentity()
{
	const ADSPlayerState* PS = PlayerState.Get();
	if (!PS)
	{
		return;
	}
	const EDSHeroClass HeroClass = DSUIState::GetHeroClass(*PS);
	const UDSUIDataAsset* UIData = DSUI::GetUIData(this);

	if (PlayerNameText)
	{
		PlayerNameText->SetText(FText::FromString(PS->GetPlayerName()));
	}
	if (ClassNameText)
	{
		ClassNameText->SetText(UIData
			? UIData->GetHeroClassName(HeroClass)
			: StaticEnum<EDSHeroClass>()->GetDisplayNameTextByValue(static_cast<int64>(HeroClass)));
	}
	const FDSClassUIInfo* ClassInfo = UIData ? UIData->FindClassInfo(HeroClass) : nullptr;
	if (ClassIcon)
	{
		DSUI::SetImageFromSoftTexture(ClassIcon, ClassInfo ? ClassInfo->Icon : TSoftObjectPtr<UTexture2D>());
	}
	if (ResourceBar && ClassInfo)
	{
		ResourceBar->SetFillColorAndOpacity(ClassInfo->ResourceColor);
	}
}

void UDSPartyMemberWidget::RefreshDowned()
{
	const UAbilitySystemComponent* ASC = AbilitySystem.Get();
	const bool bIsDowned = ASC && ASC->HasAnyMatchingGameplayTags(GetDownedTags());
	DSUI::SetWidgetShown(DownedOverlay, bIsDowned);
	if (bIsDowned != bWasDowned)
	{
		bWasDowned = bIsDowned;
		BP_OnDownedChanged(bIsDowned);
	}
}

void UDSPartyMemberWidget::RefreshStatuses()
{
	const UAbilitySystemComponent* ASC = AbilitySystem.Get();
	const UDSUIDataAsset* UIData = DSUI::GetUIData(this);
	if (!StatusContainer || !StatusIconClass || !ASC || !UIData)
	{
		return;
	}

	// Poucos status por jogador: recriar é mais simples que fazer pool.
	StatusContainer->ClearChildren();
	for (const FDSStatusUIInfo& Status : UIData->GetStatuses())
	{
		const int32 Stacks = ASC->GetTagCount(Status.StatusTag);
		if (Stacks <= 0)
		{
			continue;
		}
		if (UDSStatusIconWidget* Icon = CreateWidget<UDSStatusIconWidget>(this, StatusIconClass))
		{
			Icon->SetStatus(Status, Stacks);
			StatusContainer->AddChild(Icon);
		}
	}
}

#undef LOCTEXT_NAMESPACE
