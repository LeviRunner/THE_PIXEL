#include "UI/HUD/DSCastSlotWidget.h"

#include "AbilitySystemComponent.h"
#include "Abilities/GameplayAbility.h"
#include "CommonActionWidget.h"
#include "Components/Image.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "GameplayAbilitySpec.h"
#include "GameplayEffect.h"
#include "InputAction.h"
#include "UI/DSUIDataAsset.h"
#include "UI/DSUIStatics.h"

void UDSCastSlotWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	if (InputHint)
	{
		if (InputAction)
		{
			InputHint->SetEnhancedInputAction(InputAction);
		}
		DSUI::SetWidgetShown(InputHint, InputAction != nullptr);
	}
}

void UDSCastSlotWidget::BindAbilitySystem(UAbilitySystemComponent* InAbilitySystem)
{
	AbilitySystem = InAbilitySystem;
	CachedHandle = FGameplayAbilitySpecHandle();
	ShownAbility.Reset();
	bHasShownIdentity = false;
	RefreshSlot();
}

void UDSCastSlotWidget::SetDefaultInputTag(const FGameplayTag& DefaultInputTag)
{
	if (!InputTag.IsValid())
	{
		InputTag = DefaultInputTag;
	}
}

void UDSCastSlotWidget::RefreshSlot()
{
	const FGameplayAbilitySpec* Spec = FindSpec();
	const UGameplayAbility* Ability = Spec ? Spec->Ability.Get() : nullptr;

	// A habilidade do slot pode mudar (5ª habilidade de apoio substitui uma das 4).
	if (!bHasShownIdentity || ShownAbility.Get() != Ability)
	{
		bHasShownIdentity = true;
		ShownAbility = Ability;
		RefreshIdentity(Ability);
	}
	RefreshCooldown(Spec);
	RefreshCost(Spec);
	RefreshCharges();
}

bool UDSCastSlotWidget::SpecMatchesSlot(const FGameplayAbilitySpec& Spec) const
{
	if (!Spec.Ability)
	{
		return false;
	}
	if (InputTag.IsValid() && Spec.GetDynamicSpecSourceTags().HasTagExact(InputTag))
	{
		return true;
	}
	return AbilityTag.IsValid() && Spec.Ability->GetAssetTags().HasTag(AbilityTag);
}

const FGameplayAbilitySpec* UDSCastSlotWidget::FindSpec()
{
	UAbilitySystemComponent* ASC = AbilitySystem.Get();
	if (!ASC)
	{
		return nullptr;
	}
	if (CachedHandle.IsValid())
	{
		const FGameplayAbilitySpec* Cached = ASC->FindAbilitySpecFromHandle(CachedHandle);
		if (Cached && SpecMatchesSlot(*Cached))
		{
			return Cached;
		}
		CachedHandle = FGameplayAbilitySpecHandle();
	}
	// Poucas habilidades por herói: busca linear é suficiente.
	for (const FGameplayAbilitySpec& Spec : ASC->GetActivatableAbilities())
	{
		if (SpecMatchesSlot(Spec))
		{
			CachedHandle = Spec.Handle;
			return &Spec;
		}
	}
	return nullptr;
}

void UDSCastSlotWidget::RefreshIdentity(const UGameplayAbility* Ability)
{
	const UDSUIDataAsset* UIData = DSUI::GetUIData(this);
	const FDSDisplayUIInfo* Info = (Ability && UIData) ? UIData->FindAbilityInfo(Ability->GetAssetTags()) : nullptr;

	DSUI::SetImageFromSoftTexture(AbilityIcon, Info ? Info->Icon : TSoftObjectPtr<UTexture2D>());
	SetToolTipText(Info ? Info->DisplayName : FText::GetEmpty());
	DSUI::SetWidgetShown(EmptySlotOverlay, Ability == nullptr);
	BP_OnAbilityChanged(Ability != nullptr);
}

void UDSCastSlotWidget::GetCooldown(const FGameplayAbilitySpec* Spec, float& OutRemaining, float& OutDuration) const
{
	OutRemaining = 0.f;
	OutDuration = 0.f;
	const UAbilitySystemComponent* ASC = AbilitySystem.Get();
	if (!ASC)
	{
		return;
	}

	const FGameplayAbilityActorInfo* ActorInfo = ASC->AbilityActorInfo.Get();
	if (Spec && Spec->Ability && ActorInfo && Spec->Ability->GetCooldownTags())
	{
		Spec->Ability->GetCooldownTimeRemainingAndDuration(Spec->Handle, ActorInfo, OutRemaining, OutDuration);
		return;
	}
	if (!FallbackCooldownTag.IsValid())
	{
		return;
	}
	// Sem Cooldown GE na habilidade: procura GEs ativos com a tag de recarga do slot.
	const FGameplayEffectQuery Query = FGameplayEffectQuery::MakeQuery_MatchAnyOwningTags(FGameplayTagContainer(FallbackCooldownTag));
	for (const TPair<float, float>& Entry : ASC->GetActiveEffectsTimeRemainingAndDuration(Query))
	{
		if (Entry.Key > OutRemaining)
		{
			OutRemaining = Entry.Key;
			OutDuration = Entry.Value;
		}
	}
}

void UDSCastSlotWidget::RefreshCooldown(const FGameplayAbilitySpec* Spec)
{
	float Remaining = 0.f;
	float Duration = 0.f;
	GetCooldown(Spec, Remaining, Duration);

	const bool bOnCooldown = Remaining > 0.f;
	if (CooldownBar)
	{
		CooldownBar->SetPercent(DSUI::SafeRatio(Remaining, Duration));
		DSUI::SetWidgetShown(CooldownBar, bOnCooldown);
	}
	if (CooldownText)
	{
		CooldownText->SetText(DSUI::FormatCooldownSeconds(Remaining));
		DSUI::SetWidgetShown(CooldownText, bOnCooldown);
	}
	if (bWasOnCooldown && !bOnCooldown)
	{
		BP_OnCooldownFinished();
	}
	bWasOnCooldown = bOnCooldown;
}

void UDSCastSlotWidget::RefreshCost(const FGameplayAbilitySpec* Spec)
{
	if (!InsufficientResourceOverlay)
	{
		return;
	}
	const UAbilitySystemComponent* ASC = AbilitySystem.Get();
	const FGameplayAbilityActorInfo* ActorInfo = ASC ? ASC->AbilityActorInfo.Get() : nullptr;
	// CheckCost é const e não ativa nada: só pergunta ao ASC se o custo pode ser pago.
	const bool bCanPay = !Spec || !Spec->Ability || !ActorInfo || Spec->Ability->CheckCost(Spec->Handle, ActorInfo);
	DSUI::SetWidgetShown(InsufficientResourceOverlay, !bCanPay);
}

void UDSCastSlotWidget::RefreshCharges()
{
	if (!ChargesText)
	{
		return;
	}
	const UAbilitySystemComponent* ASC = AbilitySystem.Get();
	if (!ChargesTag.IsValid() || !ASC)
	{
		DSUI::SetWidgetShown(ChargesText, false);
		return;
	}
	const int32 Charges = ASC->GetTagCount(ChargesTag);
	ChargesText->SetText(FText::AsNumber(Charges));
	DSUI::SetWidgetShown(ChargesText, true);
}
