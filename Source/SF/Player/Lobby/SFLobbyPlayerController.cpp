#include "SFLobbyPlayerController.h"

#include "SFLobbyPlayerState.h"
#include "GameModes/Lobby/SFLobbyGameMode.h"
#include "GameModes/Lobby/SFLobbyGameState.h"
#include "System/SFGameInstance.h"

ASFLobbyPlayerController::ASFLobbyPlayerController()
{
	bAutoManageActiveCameraTarget = false;
	bReady = false;
}

void ASFLobbyPlayerController::BeginPlay()
{
	Super::BeginPlay();

	Server_UpdatePlayerInfo();
}


void ASFLobbyPlayerController::Server_RequestPlayerSelectionChange_Implementation(uint8 NewSlotID)
{
	if (!GetWorld())
	{
		return;
	}

	ASFLobbyGameState* SFGameState = GetWorld()->GetGameState<ASFLobbyGameState>();
	if (!SFGameState)
	{
		return;
	}
	SFGameState->RequestPlayerSelectionChange(GetPlayerState<ASFLobbyPlayerState>(), NewSlotID);
}

bool ASFLobbyPlayerController::Server_RequestPlayerSelectionChange_Validate(uint8 NewSlotID)
{
	return true;
}

void ASFLobbyPlayerController::Server_StartHeroSelection_Implementation()
{
	if (!HasAuthority() || !GetWorld())
	{
		return;
	}

	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		ASFLobbyPlayerController* PlayerController = Cast<ASFLobbyPlayerController>(*It);
		if (PlayerController)
		{
			PlayerController->Client_StartHeroSelection();
		}
	}
}

bool ASFLobbyPlayerController::Server_StartHeroSelection_Validate()
{
	return true;
}

void ASFLobbyPlayerController::Server_RequestStartMatch_Implementation()
{
	USFGameInstance* GameInstance = GetGameInstance<USFGameInstance>();
	if (GameInstance)
	{
		GameInstance->StartMatch();
	}
}

bool ASFLobbyPlayerController::Server_RequestStartMatch_Validate()
{
	return true;
}

void ASFLobbyPlayerController::Client_StartHeroSelection_Implementation()
{
	OnSwitchToHeroSelection.ExecuteIfBound();
}

void ASFLobbyPlayerController::UpdatePlayerInfo()
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	// 0.1초 후 서버 RPC 호출
	FTimerHandle DelayTimerHandle;
	World->GetTimerManager().SetTimer(
		DelayTimerHandle,
		this,
		&ASFLobbyPlayerController::Server_UpdatePlayerInfo,
		0.1f,
		false
	);
}

void ASFLobbyPlayerController::Server_UpdatePlayerInfo_Implementation()
{
	ASFLobbyGameMode* LobbyGM = GetWorld()->GetAuthGameMode<ASFLobbyGameMode>();
	if (!LobbyGM)
	{
		return;
	}
	
	LobbyGM->UpdatePlayerInfo(this);
}

void ASFLobbyPlayerController::Server_ToggleReadyStatus_Implementation()
{
	PlayerInfo.bReady = !PlayerInfo.bReady;

	if (ASFLobbyGameMode* LobbyGM = GetWorld()->GetAuthGameMode<ASFLobbyGameMode>())
	{
		LobbyGM->UpdatePlayerInfo(this);
	}
}
