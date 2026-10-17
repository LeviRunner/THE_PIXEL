#include "Network/Backend/DSBackendProfileSave.h"

#include "DungeonStance.h"
#include "Kismet/GameplayStatics.h"

namespace DSBackendProfilePrivate
{
	constexpr int32 UserIndex = 0;
}

UDSBackendProfileSave* UDSBackendProfileSave::LoadOrCreate(const FString& SlotName)
{
	if (UGameplayStatics::DoesSaveGameExist(SlotName, DSBackendProfilePrivate::UserIndex))
	{
		USaveGame* Loaded = UGameplayStatics::LoadGameFromSlot(SlotName, DSBackendProfilePrivate::UserIndex);
		if (UDSBackendProfileSave* Profile = Cast<UDSBackendProfileSave>(Loaded))
		{
			return Profile;
		}
		UE_LOG(LogDungeonStance, Error, TEXT("[Backend] Slot '%s' corrompido ou de outro tipo; criando perfil novo."), *SlotName);
	}
	return CastChecked<UDSBackendProfileSave>(UGameplayStatics::CreateSaveGameObject(UDSBackendProfileSave::StaticClass()));
}

bool UDSBackendProfileSave::SaveToSlot(const FString& SlotName)
{
	SaveVersion = CurrentVersion;
	const bool bSaved = UGameplayStatics::SaveGameToSlot(this, SlotName, DSBackendProfilePrivate::UserIndex);
	if (!bSaved)
	{
		UE_LOG(LogDungeonStance, Error, TEXT("[Backend] Falha ao gravar o perfil local no slot '%s'."), *SlotName);
	}
	return bSaved;
}

bool UDSBackendProfileSave::EnsureDeviceId()
{
	if (DSBackendIds::IsSafeIdentifier(DeviceId, 10, 128))
	{
		return false;
	}
	DeviceId = FGuid::NewGuid().ToString(EGuidFormats::Digits);
	return true;
}
