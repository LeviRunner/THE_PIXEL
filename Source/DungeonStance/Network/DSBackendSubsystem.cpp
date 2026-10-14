#include "Network/DSBackendSubsystem.h"

#include "DungeonStance.h"
#include "Core/DSGameInstance.h"
#include "Network/DSBackendService.h"
#include "Network/DSNetworkSettings.h"
#include "Network/Backend/DSBackendProfileSave.h"
#include "Network/Backend/DSNakamaBackend.h"
#include "Network/Backend/DSNullBackend.h"
#include "Engine/Engine.h"
#include "Engine/GameInstance.h"
#include "Engine/World.h"

void UDSBackendSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	CreateBackend();
}

void UDSBackendSubsystem::Deinitialize()
{
	BackendObject = nullptr;
	Super::Deinitialize();
}

void UDSBackendSubsystem::CreateBackend()
{
	const UDSNetworkSettings* Settings = UDSNetworkSettings::Get();
	if (Settings->Backend == EDSBackendType::Nakama)
	{
		UDSNakamaBackend* Nakama = NewObject<UDSNakamaBackend>(this);
		if (Nakama->Configure(*Settings))
		{
			BackendObject = Nakama;
			ActiveBackendType = EDSBackendType::Nakama;
			UE_LOG(LogDungeonStance, Log, TEXT("[Backend] Usando Nakama."));
			return;
		}
		UE_LOG(LogDungeonStance, Error, TEXT("[Backend] Configuração do Nakama inválida; caindo para o backend OFFLINE (progresso não vai para a conta)."));
	}

	BackendObject = CreateNullBackend();
	ActiveBackendType = EDSBackendType::Null;
	UE_LOG(LogDungeonStance, Log, TEXT("[Backend] Usando backend offline (slot '%s')."), *GetProfileSlotName());
}

UObject* UDSBackendSubsystem::CreateNullBackend()
{
	UDSNullBackend* NullBackend = NewObject<UDSNullBackend>(this);
	NullBackend->Configure(GetProfileSlotName());
	return NullBackend;
}

IDSBackendService* UDSBackendSubsystem::GetBackend() const
{
	return Cast<IDSBackendService>(BackendObject.Get());
}

FString UDSBackendSubsystem::GetProfileSlotName() const
{
	// Cada instância de PIE ganha um slot (e device id) próprio para simular jogadores distintos.
	const FString BaseSlot = UDSNetworkSettings::Get()->LocalProfileSlotName;
	const UGameInstance* GameInstance = GetGameInstance();
	const FWorldContext* WorldContext = GameInstance ? GameInstance->GetWorldContext() : nullptr;
	const int32 PIEInstance = WorldContext ? WorldContext->PIEInstance : INDEX_NONE;
	return PIEInstance > 0 ? FString::Printf(TEXT("%s_PIE%d"), *BaseSlot, PIEInstance) : BaseSlot;
}

FString UDSBackendSubsystem::GetOrCreateDeviceId() const
{
	const FString SlotName = GetProfileSlotName();
	UDSBackendProfileSave* Save = UDSBackendProfileSave::LoadOrCreate(SlotName);
	if (Save->EnsureDeviceId())
	{
		Save->SaveToSlot(SlotName);
	}
	return Save->DeviceId;
}

bool UDSBackendSubsystem::CanSubmitResults() const
{
	const UGameInstance* GameInstance = GetGameInstance();
	const UWorld* World = GameInstance ? GameInstance->GetWorld() : nullptr;
	return World && World->GetNetMode() != NM_Client;
}

void UDSBackendSubsystem::SyncGameInstancePlayerId() const
{
	if (UDSGameInstance* GameInstance = Cast<UDSGameInstance>(GetGameInstance()))
	{
		GameInstance->LocalPlayerId = GetPlayerId();
	}
}

bool UDSBackendSubsystem::IsAuthenticated() const
{
	const IDSBackendService* Backend = GetBackend();
	return Backend && Backend->IsAuthenticated();
}

FString UDSBackendSubsystem::GetPlayerId() const
{
	const IDSBackendService* Backend = GetBackend();
	return Backend ? Backend->GetPlayerId() : FString();
}

FString UDSBackendSubsystem::GetSessionToken() const
{
	const IDSBackendService* Backend = GetBackend();
	return Backend ? Backend->GetSessionToken() : FString();
}

// --- Operações ----------------------------------------------------------

void UDSBackendSubsystem::Authenticate()
{
	IDSBackendService* Backend = GetBackend();
	if (!Backend || bAuthInFlight)
	{
		UE_LOG(LogDungeonStance, Warning, TEXT("[Backend] Authenticate ignorado (sem backend ou login em andamento)."));
		if (!Backend)
		{
			OnAuthenticated.Broadcast(false, FString());
		}
		return;
	}

	bAuthInFlight = true;
	Backend->Authenticate(GetOrCreateDeviceId(), FDSOnBackendAuthComplete::CreateUObject(this, &ThisClass::HandleAuthComplete));
}

void UDSBackendSubsystem::FetchProfile()
{
	IDSBackendService* Backend = GetBackend();
	if (!Backend || bProfileInFlight)
	{
		if (!Backend)
		{
			OnProfileFetched.Broadcast(false, CachedProfile);
		}
		return;
	}

	bProfileInFlight = true;
	Backend->FetchProfile(FDSOnBackendProfileComplete::CreateUObject(this, &ThisClass::HandleProfileComplete));
}

void UDSBackendSubsystem::SubmitRunResults(const TArray<FDSRunResult>& Results)
{
	IDSBackendService* Backend = GetBackend();
	if (!Backend || !CanSubmitResults())
	{
		UE_LOG(LogDungeonStance, Error, TEXT("[Backend] SubmitRunResults rejeitado: só o servidor (ou Standalone) pode persistir resultados."));
		OnRunResultsSubmitted.Broadcast(false);
		return;
	}
	if (Results.IsEmpty())
	{
		OnRunResultsSubmitted.Broadcast(true);
		return;
	}
	Backend->SubmitRunResults(Results, FDSOnBackendSubmitComplete::CreateUObject(this, &ThisClass::HandleSubmitComplete));
}

void UDSBackendSubsystem::VerifyPlayerToken(const FString& SessionToken, FDSOnBackendTokenVerified OnComplete)
{
	IDSBackendService* Backend = GetBackend();
	if (!Backend || !CanSubmitResults())
	{
		OnComplete.ExecuteIfBound(false, FString(), TEXT("VerifyPlayerToken só roda no servidor."));
		return;
	}
	Backend->VerifyPlayerToken(SessionToken, MoveTemp(OnComplete));
}

// --- Callbacks ----------------------------------------------------------

void UDSBackendSubsystem::HandleAuthComplete(bool bSuccess, const FString& ErrorMessage)
{
	bAuthInFlight = false;
	if (!bSuccess)
	{
		UE_LOG(LogDungeonStance, Warning, TEXT("[Backend] Login falhou: %s"), *ErrorMessage);
	}
	else
	{
		SyncGameInstancePlayerId();
	}
	OnAuthenticated.Broadcast(bSuccess, GetPlayerId());
}

void UDSBackendSubsystem::HandleProfileComplete(bool bSuccess, const FDSPlayerProfile& Profile, const FString& ErrorMessage)
{
	bProfileInFlight = false;
	if (bSuccess)
	{
		CachedProfile = Profile;
	}
	else
	{
		UE_LOG(LogDungeonStance, Warning, TEXT("[Backend] FetchProfile falhou: %s"), *ErrorMessage);
	}
	OnProfileFetched.Broadcast(bSuccess, CachedProfile);
}

void UDSBackendSubsystem::HandleSubmitComplete(bool bSuccess, const FString& ErrorMessage)
{
	if (!bSuccess)
	{
		UE_LOG(LogDungeonStance, Error, TEXT("[Backend] Resultados da run NÃO foram salvos: %s"), *ErrorMessage);
	}
	OnRunResultsSubmitted.Broadcast(bSuccess);
}
