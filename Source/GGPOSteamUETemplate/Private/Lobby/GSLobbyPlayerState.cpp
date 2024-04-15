// Fill out your copyright notice in the Description page of Project Settings.


#include "Lobby/GSLobbyPlayerState.h"
#include "Lobby/GSLobbyGameState.h"

AGSLobbyPlayerState* AGSLobbyPlayerState::GetFirstLocalLobbyPlayerState(UObject* WorldContextObject)
{
	UWorld* World = GEngine->GetWorldFromContextObjectChecked(WorldContextObject);
	if (!World)
	{
		UE_LOG(LogTemp, Error, TEXT("AGSLobbyPlayerState::GetFirstLocalPlayerState: World is nullptr"));
		return nullptr;
	}

	for (FConstPlayerControllerIterator It = World->GetPlayerControllerIterator(); It; ++It)
	{
		APlayerController* PlayerController = It->Get();
		if (PlayerController && PlayerController->IsLocalPlayerController())
		{
			return Cast<AGSLobbyPlayerState>(PlayerController->PlayerState);
		}
	}

	return nullptr;
}

void AGSLobbyPlayerState::Server_RequestPlayerList_Implementation()
{
	AGSLobbyGameState* LobbyGameState = GetWorld()->GetGameState<AGSLobbyGameState>();

	if (LobbyGameState)
	{
		LobbyGameState->RequestPlayerList();
	}
}

bool AGSLobbyPlayerState::Server_RequestPlayerList_Validate()
{
	return true;
}