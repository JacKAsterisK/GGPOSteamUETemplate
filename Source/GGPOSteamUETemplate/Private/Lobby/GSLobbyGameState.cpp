// Fill out your copyright notice in the Description page of Project Settings.


#include "Lobby/GSLobbyGameState.h"
#include "Lobby/GSLobbyPlayerState.h"
#include "Net/UnrealNetwork.h"

AGSLobbyGameState* AGSLobbyGameState::GetLobbyGameState(UObject* WorldContextObject)
{
	AGameStateBase* GameState = WorldContextObject->GetWorld()->GetGameState();
	return Cast<AGSLobbyGameState>(GameState);
}

void AGSLobbyGameState::RequestPlayerList()
{
	if (GetLocalRole() != ROLE_Authority)
	{
		return;
	}

	Multicast_UpdatePlayerList(GGPONetSubsystem->GetPlayerInfo());
}

void AGSLobbyGameState::Multicast_UpdatePlayerList_Implementation(const TArray<FPlayerInfo>& PlayerList)
{
	if (GetLocalRole() != ROLE_Authority)
	{
		GGPONetSubsystem->UpdatePlayerList(PlayerList);
	}

	OnPlayerListUpdated.Broadcast(GGPONetSubsystem->GetPlayerList());
}

void AGSLobbyGameState::OnPlayersChanged(const TArray<UGGPOPlayer*>& Players)
{
	if (GetLocalRole() == ROLE_Authority)
	{
		Multicast_UpdatePlayerList(GGPONetSubsystem->GetPlayerInfo());
	}
}

void AGSLobbyGameState::BeginPlay()
{
	Super::BeginPlay();

	GGPONetSubsystem = GetGameInstance()->GetSubsystem<UGGPONetSubsystem>();

	// Request player list if client
	if (GetLocalRole() == ROLE_Authority)
	{
		GGPONetSubsystem->OnPlayersChanged.AddDynamic(this, &AGSLobbyGameState::OnPlayersChanged);
	}
	else
	{
		GGPONetSubsystem->ClearPlayerList();
	}
}

void AGSLobbyGameState::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);

	if (GGPONetSubsystem)
	{
		GGPONetSubsystem->OnPlayersChanged.RemoveDynamic(this, &AGSLobbyGameState::OnPlayersChanged);
	}
}
