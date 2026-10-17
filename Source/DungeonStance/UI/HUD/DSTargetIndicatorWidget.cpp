#include "UI/HUD/DSTargetIndicatorWidget.h"

#include "AbilitySystemComponent.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "Engine/World.h"
#include "GameplayEffectTypes.h"
#include "Gameplay/Attributes/DSAttributeSet.h"
#include "Gameplay/Characters/DSHeroCharacter.h"
#include "Gameplay/Characters/DSTargetLockComponent.h"
#include "TimerManager.h"
#include "UI/DSUIStateAccess.h"
#include "UI/DSUIStatics.h"

namespace
{
	TArray<FGameplayAttribute> GetTargetAttributes()
	{
		return {
			UDSAttributeSet::GetHealthAttribute(),
			UDSAttributeSet::GetMaxHealthAttribute(),
			UDSAttributeSet::GetShieldAttribute()
		};
	}
}

void UDSTargetIndicatorWidget::NativeConstruct()
{
	Super::NativeConstruct();
	SetVisibility(ESlateVisibility::Collapsed);
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimer(PollTimer, this, &ThisClass::PollTarget, TargetPollInterval, true);
	}
}

void UDSTargetIndicatorWidget::NativeDestruct()
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(PollTimer);
	}
	UnbindTargetAbilitySystem();
	Super::NativeDestruct();
}

AActor* UDSTargetIndicatorWidget::ResolveLockedTarget_Implementation() const
{
	const ADSHeroCharacter* Hero = Cast<ADSHeroCharacter>(GetOwningPlayerPawn());
	const UDSTargetLockComponent* TargetLock = Hero ? Hero->GetTargetLock() : nullptr;
	return TargetLock ? TargetLock->GetLockedTarget() : nullptr;
}

void UDSTargetIndicatorWidget::PollTarget()
{
	AActor* Target = ResolveLockedTarget();
	if (Target != CurrentTarget.Get())
	{
		SetTarget(Target);
	}
}

void UDSTargetIndicatorWidget::SetTarget(AActor* NewTarget)
{
	UnbindTargetAbilitySystem();
	CurrentTarget = NewTarget;

	UAbilitySystemComponent* ASC = DSUIState::GetAbilitySystem(NewTarget);
	if (ASC && ASC->HasAttributeSetForAttribute(UDSAttributeSet::GetHealthAttribute()))
	{
		TargetAbilitySystem = ASC;
		for (const FGameplayAttribute& Attribute : GetTargetAttributes())
		{
			ASC->GetGameplayAttributeValueChangeDelegate(Attribute).AddUObject(this, &ThisClass::HandleTargetAttributeChanged);
		}
	}

	SetVisibility(NewTarget ? ESlateVisibility::SelfHitTestInvisible : ESlateVisibility::Collapsed);
	RefreshTargetVitals();
	BP_OnTargetChanged(NewTarget);
}

void UDSTargetIndicatorWidget::UnbindTargetAbilitySystem()
{
	if (UAbilitySystemComponent* ASC = TargetAbilitySystem.Get())
	{
		for (const FGameplayAttribute& Attribute : GetTargetAttributes())
		{
			ASC->GetGameplayAttributeValueChangeDelegate(Attribute).RemoveAll(this);
		}
	}
	TargetAbilitySystem.Reset();
}

void UDSTargetIndicatorWidget::HandleTargetAttributeChanged(const FOnAttributeChangeData& Data)
{
	RefreshTargetVitals();
}

void UDSTargetIndicatorWidget::RefreshTargetVitals()
{
	const UAbilitySystemComponent* ASC = TargetAbilitySystem.Get();
	const float Health = ASC ? ASC->GetNumericAttribute(UDSAttributeSet::GetHealthAttribute()) : 0.f;
	const float MaxHealth = ASC ? ASC->GetNumericAttribute(UDSAttributeSet::GetMaxHealthAttribute()) : 0.f;
	const float Shield = ASC ? ASC->GetNumericAttribute(UDSAttributeSet::GetShieldAttribute()) : 0.f;

	TargetHealthBar->SetPercent(DSUI::SafeRatio(Health, MaxHealth));
	if (TargetShieldBar)
	{
		TargetShieldBar->SetPercent(DSUI::SafeRatio(Shield, MaxHealth));
		DSUI::SetWidgetShown(TargetShieldBar, Shield > 0.f);
	}
	if (TargetHealthText)
	{
		TargetHealthText->SetText(FText::AsNumber(FMath::CeilToInt(Health)));
	}
}
