// Fill out your copyright notice in the Description page of Project Settings.


#include "GGPOGameState.h"
#include "GGPOManager.h"
#include "GGPONetSubsystem.h"
#include "GGPOPlayerState.h"
#include "GSim/GSim.h"
#include "EngineUtils.h"

void AGGPOGameState::ServerPlayerLoaded_Implementation(AGGPOPlayerState* PlayerState)
{
	UGGPONetSubsystem* NetSubsystem = UGGPONetSubsystem::Get(this);
	UGGPOPlayer* GPlayer = NetSubsystem->GetPlayerFromState(PlayerState);
	if (GPlayer)
	{
		GPlayer->SetIsLoaded(true);
	}

	// Check if all players are loaded
	bool AllPlayersLoaded = true;
	for (UGGPOPlayer* Player : NetSubsystem->GetPlayerList())
	{
		if (!Player->IsLoaded())
		{
			AllPlayersLoaded = false;
			break;
		}
	}

	if (AllPlayersLoaded)
	{
		MulticastBeginGame(NetSubsystem->GetPlayerInfo());
	}
}

bool AGGPOGameState::ServerPlayerLoaded_Validate(AGGPOPlayerState* PlayerState)
{
	return true;
}

void AGGPOGameState::MulticastBeginGame_Implementation(const TArray<FPlayerInfo>& InPlayerInfo)
{
	if (GetLocalRole() != ROLE_Authority)
	{
		UGGPONetSubsystem* NetSubsystem = UGGPONetSubsystem::Get(this);
		TArray<UGGPOPlayer*> PlayerList = NetSubsystem->GetPlayerList();

		for (int i = 0; i < PlayerList.Num(); ++i)
		{
			UGGPOPlayer* Player = PlayerList[i];

			for (const FPlayerInfo& PlayerInfo : InPlayerInfo)
			{
				if (Player->GetPlayerId() == PlayerInfo.PlayerId)
				{
					Player->SetPlayerInfo(PlayerInfo, Player->IsLocal());
					break;
				}
			}
		}
	}

	// Find the AGSim actor in the world
	AGSim* Sim = nullptr;
	for (TActorIterator<AGSim> It(GetWorld()); It; ++It)
	{
		Sim = *It;
		break;
	}

	if (Sim)
	{
		Sim->InitPlayers();
		GGPOManager::InitializeGGPOSession(false, Sim);
	}
}

bool AGGPOGameState::MulticastBeginGame_Validate(const TArray<FPlayerInfo>& InPlayerInfo)
{
	return true;
}
