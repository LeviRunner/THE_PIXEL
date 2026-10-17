#include "Core/DSGameInstance.h"

#include "DungeonStance.h"
#include "Kismet/GameplayStatics.h"

void UDSGameInstance::Init()
{
	Super::Init();
	LoadLocalProfile();
}

bool UDSGameInstance::SaveLocalProfile()
{
	UDSLocalProfileSave* Save = Cast<UDSLocalProfileSave>(UGameplayStatics::CreateSaveGameObject(UDSLocalProfileSave::StaticClass()));
	if (!Save)
	{
		UE_LOG(LogDungeonStance, Error, TEXT("Não foi possível criar o objeto de perfil local."));
		return false;
	}
	Save->LocalPlayerId = LocalPlayerId;
	Save->LastClass = LastClass;
	Save->LastStance = LastStance;
	Save->DesiredPartySize = DesiredPartySize;

	const bool bSaved = UGameplayStatics::SaveGameToSlot(Save, ProfileSlotName, ProfileUserIndex);
	if (!bSaved)
	{
		UE_LOG(LogDungeonStance, Warning, TEXT("Falha ao gravar o perfil local no slot %s."), *ProfileSlotName);
	}
	return bSaved;
}

bool UDSGameInstance::LoadLocalProfile()
{
	if (!UGameplayStatics::DoesSaveGameExist(ProfileSlotName, ProfileUserIndex))
	{
		return false;
	}
	const UDSLocalProfileSave* Save = Cast<UDSLocalProfileSave>(UGameplayStatics::LoadGameFromSlot(ProfileSlotName, ProfileUserIndex));
	if (!Save)
	{
		UE_LOG(LogDungeonStance, Warning, TEXT("Perfil local corrompido no slot %s; usando padrões."), *ProfileSlotName);
		return false;
	}
	LocalPlayerId = Save->LocalPlayerId;
	LastClass = Save->LastClass;
	LastStance = Save->LastStance;
	// Dado vindo do disco: só aceita tamanhos de party válidos.
	const int32 PartySize = static_cast<int32>(Save->DesiredPartySize);
	const bool bValidPartySize = PartySize >= DSConstants::MinPartySize && PartySize <= DSConstants::MaxPartySize;
	DesiredPartySize = bValidPartySize ? Save->DesiredPartySize : EDSPartySize::Solo;
	return true;
}
