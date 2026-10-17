#include "UI/HUD/DSCastBarWidget.h"

#include "AbilitySystemComponent.h"
#include "Core/DSGameplayTags.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "UI/HUD/DSCastSlotWidget.h"

void UDSCastBarWidget::InitWithAbilitySystem(UAbilitySystemComponent* InAbilitySystem)
{
	AssignDefaultInputTags();
	for (UDSCastSlotWidget* CastSlot : GetSlots())
	{
		CastSlot->BindAbilitySystem(InAbilitySystem);
	}

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimer(RefreshTimer, this, &ThisClass::RefreshSlots, RefreshInterval, true);
	}
}

void UDSCastBarWidget::NativeDestruct()
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(RefreshTimer);
	}
	Super::NativeDestruct();
}

TArray<UDSCastSlotWidget*> UDSCastBarWidget::GetSlots() const
{
	TArray<UDSCastSlotWidget*> Slots;
	for (UDSCastSlotWidget* CastSlot : { Slot_Attack.Get(), Slot_ClassSkill.Get(), Slot_Dodge.Get(),
		Slot_Support1.Get(), Slot_Support2.Get(), Slot_Support3.Get(), Slot_Support4.Get() })
	{
		if (CastSlot)
		{
			Slots.Add(CastSlot);
		}
	}
	return Slots;
}

void UDSCastBarWidget::AssignDefaultInputTags()
{
	// Só preenche quando a instância no WBP não definiu InputTag.
	auto AssignDefault = [](UDSCastSlotWidget* CastSlot, const FGameplayTag& Tag)
	{
		if (CastSlot)
		{
			CastSlot->SetDefaultInputTag(Tag);
		}
	};
	AssignDefault(Slot_Attack, DSTags::Input_Attack);
	AssignDefault(Slot_ClassSkill, DSTags::Input_ClassSkill);
	AssignDefault(Slot_Dodge, DSTags::Input_Dodge);
	AssignDefault(Slot_Support1, DSTags::Input_Support1);
	AssignDefault(Slot_Support2, DSTags::Input_Support2);
	AssignDefault(Slot_Support3, DSTags::Input_Support3);
	AssignDefault(Slot_Support4, DSTags::Input_Support4);
}

void UDSCastBarWidget::RefreshSlots()
{
	if (!IsVisible())
	{
		return;
	}
	for (UDSCastSlotWidget* CastSlot : GetSlots())
	{
		CastSlot->RefreshSlot();
	}
}
