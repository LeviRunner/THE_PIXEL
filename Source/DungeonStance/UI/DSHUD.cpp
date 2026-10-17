#include "UI/DSHUD.h"

#include "DungeonStance.h"
#include "GameFramework/PlayerController.h"
#include "UI/DSUIDataAsset.h"
#include "UI/HUD/DSHUDLayoutWidget.h"

void ADSHUD::BeginPlay()
{
	Super::BeginPlay();
	CreateLayout();
}

void ADSHUD::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (Layout)
	{
		Layout->RemoveFromParent();
		Layout = nullptr;
	}
	Super::EndPlay(EndPlayReason);
}

void ADSHUD::CreateLayout()
{
	APlayerController* PlayerController = GetOwningPlayerController();
	if (!PlayerController || !PlayerController->IsLocalController())
	{
		return;
	}
	if (!LayoutClass)
	{
		UE_LOG(LogDungeonStance, Warning, TEXT("ADSHUD '%s' sem LayoutClass; HUD não será criado."), *GetName());
		return;
	}
	if (!UIData)
	{
		UE_LOG(LogDungeonStance, Warning, TEXT("ADSHUD '%s' sem UIData; ícones e nomes usarão fallback."), *GetName());
	}

	Layout = CreateWidget<UDSHUDLayoutWidget>(PlayerController, LayoutClass);
	if (!Layout)
	{
		return;
	}
	Layout->AddToViewport(LayoutZOrder);
	Layout->ActivateWidget();
}
