#include "UI/Menus/DSLobbyWidget.h"

#include "CommonButtonBase.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "Core/DSPlayerController.h"
#include "Core/DSPlayerState.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "UI/DSUIDataAsset.h"
#include "UI/DSUIStateAccess.h"
#include "UI/DSUIStatics.h"

#define LOCTEXT_NAMESPACE "DungeonStanceUI"

namespace
{
	void BindLobbyClick(UCommonButtonBase* Button, UDSLobbyWidget* Owner, TFunction<void()> Handler)
	{
		if (Button)
		{
			Button->OnClicked().AddWeakLambda(Owner, MoveTemp(Handler));
		}
	}
}

void UDSLobbyWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();
	BindButtons();
	LoadInitialSelection();
	RefreshSelectionDisplay();
	RefreshReadyDisplay();
	RefreshActionButtons();
	SetStatus(FText::GetEmpty());
}

void UDSLobbyWidget::InitWithPlayerState(ADSPlayerState* InPlayerState)
{
	PlayerState = InPlayerState;
	if (InPlayerState && DSUIState::GetHeroClass(*InPlayerState) != EDSHeroClass::None)
	{
		// Em partida, a escolha replicada (ex.: reconexão) prevalece sobre a preferência local.
		SelectedClass = DSUIState::GetHeroClass(*InPlayerState);
		SelectedStance = DSUIState::GetStance(*InPlayerState);
	}
	RefreshSelectionDisplay();
	RefreshReadyDisplay();
	RefreshActionButtons();
}

void UDSLobbyWidget::NativeOnActivated()
{
	Super::NativeOnActivated();
	SetVisibility(ESlateVisibility::Visible);
	BindSessionDelegates();
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimer(ReadyPollTimer, this, &ThisClass::RefreshReadyDisplay, ReadyPollInterval, true);
	}
	// Garante que o servidor conheça a escolha atual ao entrar no lobby da partida.
	ApplySelection();
}

void UDSLobbyWidget::NativeOnDeactivated()
{
	UnbindSessionDelegates();
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(ReadyPollTimer);
	}
	SetVisibility(ESlateVisibility::Collapsed);
	Super::NativeOnDeactivated();
}

void UDSLobbyWidget::NativeDestruct()
{
	UnbindSessionDelegates();
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(ReadyPollTimer);
	}
	Super::NativeDestruct();
}

UWidget* UDSLobbyWidget::NativeGetDesiredFocusTarget() const
{
	return Btn_ClassNext.Get();
}

TOptional<FUIInputConfig> UDSLobbyWidget::GetDesiredInputConfig() const
{
	return FUIInputConfig(ECommonInputMode::Menu, EMouseCaptureMode::NoCapture);
}

void UDSLobbyWidget::BindButtons()
{
	BindLobbyClick(Btn_ClassPrev, this, [this]() { HandleClassPrev(); });
	BindLobbyClick(Btn_ClassNext, this, [this]() { HandleClassNext(); });
	BindLobbyClick(Btn_StancePrev, this, [this]() { HandleStancePrev(); });
	BindLobbyClick(Btn_StanceNext, this, [this]() { HandleStanceNext(); });
	BindLobbyClick(Btn_PartySizePrev, this, [this]() { HandlePartySizePrev(); });
	BindLobbyClick(Btn_PartySizeNext, this, [this]() { HandlePartySizeNext(); });
	BindLobbyClick(Btn_Ready, this, [this]() { HandleReadyClicked(); });
	BindLobbyClick(Btn_StartSolo, this, [this]() { HandleStartSoloClicked(); });
	BindLobbyClick(Btn_FindMatch, this, [this]() { HandleFindMatchClicked(); });
	BindLobbyClick(Btn_Host, this, [this]() { HandleHostClicked(); });
}

void UDSLobbyWidget::LoadInitialSelection()
{
	DSUIState::LoadHeroPreference(this, SelectedClass, SelectedStance, SelectedPartySize);

	const TArray<EDSHeroClass> Classes = DSUI::GetEnumValues<EDSHeroClass>(true);
	const TArray<EDSStance> Stances = DSUI::GetEnumValues<EDSStance>(true);
	if (SelectedClass == EDSHeroClass::None && Classes.Num() > 0)
	{
		SelectedClass = Classes[0];
	}
	if (SelectedStance == EDSStance::None && Stances.Num() > 0)
	{
		SelectedStance = Stances[0];
	}
}

void UDSLobbyWidget::CycleClass(int32 Direction)
{
	SelectedClass = DSUI::CycleValue(DSUI::GetEnumValues<EDSHeroClass>(true), SelectedClass, Direction);
	ApplySelection();
}

void UDSLobbyWidget::CycleStance(int32 Direction)
{
	SelectedStance = DSUI::CycleValue(DSUI::GetEnumValues<EDSStance>(true), SelectedStance, Direction);
	ApplySelection();
}

void UDSLobbyWidget::CyclePartySize(int32 Direction)
{
	SelectedPartySize = DSUI::CycleValue(DSUI::GetEnumValues<EDSPartySize>(false), SelectedPartySize, Direction);
	ApplySelection();
}

void UDSLobbyWidget::ApplySelection()
{
	DSUIState::StoreHeroPreference(this, SelectedClass, SelectedStance, SelectedPartySize);

	// Dentro de uma partida o servidor precisa saber da escolha (ele valida e replica).
	ADSPlayerController* PC = DSUIState::GetDSPlayerController(GetOwningPlayer());
	if (PC && PlayerState.IsValid())
	{
		PC->Server_SelectHero(SelectedClass, SelectedStance);
	}

	RefreshSelectionDisplay();
	RefreshActionButtons();
	BP_OnSelectionChanged(SelectedClass, SelectedStance, SelectedPartySize);
}

void UDSLobbyWidget::RefreshSelectionDisplay()
{
	const UDSUIDataAsset* UIData = DSUI::GetUIData(this);
	const FDSClassUIInfo* ClassInfo = UIData ? UIData->FindClassInfo(SelectedClass) : nullptr;
	const FDSStanceUIInfo* StanceInfo = UIData ? UIData->FindStanceInfo(SelectedStance) : nullptr;

	ClassNameText->SetText(UIData ? UIData->GetHeroClassName(SelectedClass)
		: StaticEnum<EDSHeroClass>()->GetDisplayNameTextByValue(static_cast<int64>(SelectedClass)));
	StanceNameText->SetText(UIData ? UIData->GetStanceName(SelectedStance)
		: StaticEnum<EDSStance>()->GetDisplayNameTextByValue(static_cast<int64>(SelectedStance)));

	if (ClassDescriptionText)
	{
		ClassDescriptionText->SetText(ClassInfo ? ClassInfo->Description : FText::GetEmpty());
	}
	if (StanceDescriptionText)
	{
		StanceDescriptionText->SetText(StanceInfo ? StanceInfo->Description : FText::GetEmpty());
	}
	if (ClassIcon)
	{
		DSUI::SetImageFromSoftTexture(ClassIcon, ClassInfo ? ClassInfo->Icon : TSoftObjectPtr<UTexture2D>());
	}
	if (StanceIcon)
	{
		DSUI::SetImageFromSoftTexture(StanceIcon, StanceInfo ? StanceInfo->Icon : TSoftObjectPtr<UTexture2D>());
	}
	if (PartySizeText)
	{
		PartySizeText->SetText(UIData ? UIData->GetPartySizeName(SelectedPartySize)
			: StaticEnum<EDSPartySize>()->GetDisplayNameTextByValue(static_cast<int64>(SelectedPartySize)));
	}
}

void UDSLobbyWidget::RefreshReadyDisplay()
{
	const ADSPlayerState* PS = PlayerState.Get();
	DSUI::SetWidgetShown(Btn_Ready, PS != nullptr, ESlateVisibility::Visible);
	if (ReadyText && PS)
	{
		ReadyText->SetText(DSUIState::IsReady(*PS) ? LOCTEXT("ReadyOn", "Pronto!") : LOCTEXT("ReadyOff", "Ficar pronto"));
	}
}

void UDSLobbyWidget::RefreshActionButtons()
{
	// Seleção de party só faz sentido antes de entrar numa sessão.
	const bool bShowSession = bShowSessionButtons && !PlayerState.IsValid();
	const bool bIsGroup = SelectedPartySize != EDSPartySize::Solo;

	for (UCommonButtonBase* Button : { Btn_StartSolo.Get(), Btn_FindMatch.Get(), Btn_Host.Get(),
		Btn_PartySizePrev.Get(), Btn_PartySizeNext.Get() })
	{
		DSUI::SetWidgetShown(Button, bShowSession, ESlateVisibility::Visible);
	}
	DSUI::SetWidgetShown(PartySizeText, bShowSession);

	if (Btn_StartSolo)
	{
		Btn_StartSolo->SetIsEnabled(!bSessionRequestInFlight);
	}
	if (Btn_FindMatch)
	{
		Btn_FindMatch->SetIsEnabled(!bSessionRequestInFlight && bIsGroup);
	}
	if (Btn_Host)
	{
		Btn_Host->SetIsEnabled(!bSessionRequestInFlight && bIsGroup);
	}
}

void UDSLobbyWidget::SetStatus(const FText& Message)
{
	if (StatusText)
	{
		StatusText->SetText(Message);
		DSUI::SetWidgetShown(StatusText, !Message.IsEmpty());
	}
}

void UDSLobbyWidget::HandleReadyClicked()
{
	ADSPlayerController* PC = DSUIState::GetDSPlayerController(GetOwningPlayer());
	const ADSPlayerState* PS = PlayerState.Get();
	if (PC && PS)
	{
		PC->Server_SetReady(!DSUIState::IsReady(*PS));
	}
}

void UDSLobbyWidget::HandleStartSoloClicked()
{
	UDSSessionSubsystem* Sessions = DSUIState::GetSessionSubsystem(this);
	if (!Sessions || bSessionRequestInFlight)
	{
		return;
	}
	SetStatus(LOCTEXT("StartingSolo", "Iniciando run solo..."));
	Sessions->StartSoloRun();
}

void UDSLobbyWidget::HandleFindMatchClicked()
{
	UDSSessionSubsystem* Sessions = DSUIState::GetSessionSubsystem(this);
	if (!Sessions || bSessionRequestInFlight)
	{
		return;
	}
	bSessionRequestInFlight = true;
	RefreshActionButtons();
	SetStatus(LOCTEXT("Searching", "Procurando grupo..."));
	Sessions->FindSessions(SelectedPartySize);
}

void UDSLobbyWidget::HandleHostClicked()
{
	UDSSessionSubsystem* Sessions = DSUIState::GetSessionSubsystem(this);
	if (!Sessions || bSessionRequestInFlight)
	{
		return;
	}
	bSessionRequestInFlight = true;
	RefreshActionButtons();
	SetStatus(LOCTEXT("Hosting", "Criando sessão..."));
	Sessions->HostSession(SelectedPartySize, false);
}

void UDSLobbyWidget::BindSessionDelegates()
{
	if (UDSSessionSubsystem* Sessions = DSUIState::GetSessionSubsystem(this))
	{
		UnbindSessionDelegates();
		Sessions->OnSessionCreated.AddDynamic(this, &ThisClass::HandleSessionCreated);
		Sessions->OnSessionsFound.AddDynamic(this, &ThisClass::HandleSessionsFound);
		Sessions->OnSessionJoined.AddDynamic(this, &ThisClass::HandleSessionJoined);
	}
}

void UDSLobbyWidget::UnbindSessionDelegates()
{
	if (UDSSessionSubsystem* Sessions = DSUIState::GetSessionSubsystem(this))
	{
		Sessions->OnSessionCreated.RemoveDynamic(this, &ThisClass::HandleSessionCreated);
		Sessions->OnSessionsFound.RemoveDynamic(this, &ThisClass::HandleSessionsFound);
		Sessions->OnSessionJoined.RemoveDynamic(this, &ThisClass::HandleSessionJoined);
	}
}

void UDSLobbyWidget::HandleSessionCreated(bool bWasSuccessful)
{
	bSessionRequestInFlight = false;
	RefreshActionButtons();
	SetStatus(bWasSuccessful ? LOCTEXT("HostOk", "Sessão criada. Aguardando jogadores...")
		: LOCTEXT("HostFail", "Não foi possível criar a sessão."));
}

void UDSLobbyWidget::HandleSessionsFound(const TArray<FDSSessionInfo>& Results)
{
	UDSSessionSubsystem* Sessions = DSUIState::GetSessionSubsystem(this);
	if (Results.Num() == 0 || !Sessions)
	{
		bSessionRequestInFlight = false;
		RefreshActionButtons();
		SetStatus(LOCTEXT("NoSessions", "Nenhum grupo encontrado. Tente hospedar."));
		return;
	}
	// Partida rápida: entra no primeiro resultado (o subsistema já filtra por tamanho).
	// TODO(UI): lista de sessões navegável se o design pedir escolha manual.
	SetStatus(LOCTEXT("Joining", "Entrando no grupo..."));
	Sessions->JoinSession(0);
}

void UDSLobbyWidget::HandleSessionJoined(bool bWasSuccessful)
{
	bSessionRequestInFlight = false;
	RefreshActionButtons();
	SetStatus(bWasSuccessful ? LOCTEXT("JoinOk", "Conectando...") : LOCTEXT("JoinFail", "Falha ao entrar no grupo."));
}

#undef LOCTEXT_NAMESPACE
