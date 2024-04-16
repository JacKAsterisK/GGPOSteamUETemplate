// Fill out your copyright notice in the Description page of Project Settings.


#include "GGPOPlayerState.h"
#include "GGPOGameState.h"

void AGGPOPlayerState::ServerPlayerLoaded_Implementation()
{
	AGGPOGameState* GameState = GetWorld()->GetGameState<AGGPOGameState>();
	if (GameState)
	{
		GameState->ServerPlayerLoaded(this);
	}
}

bool AGGPOPlayerState::ServerPlayerLoaded_Validate()
{
	return true;
}

void AGGPOPlayerState::BeginPlay()
{
	Super::BeginPlay();
	
	ServerPlayerLoaded();
}
