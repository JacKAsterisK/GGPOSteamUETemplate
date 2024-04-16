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
			AGSLobbyPlayerState* LobbyPlayerState = Cast<AGSLobbyPlayerState>(PlayerController->PlayerState);
			if (LobbyPlayerState)
			{
				return LobbyPlayerState;
			}
		}
	}

	return nullptr;
}

void AGSLobbyPlayerState::BeginPlay()
{
	Super::BeginPlay();

	APlayerController* PlayerController = GetPlayerController();
	if (PlayerController && PlayerController->IsLocalPlayerController())
	{
		Server_RequestPlayerList();
	}

	Server_RequestPlayerList();
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