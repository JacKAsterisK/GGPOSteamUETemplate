// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "GGPOGameState.generated.h"

class AGGPOPlayerState;

/**
 * 
 */
UCLASS()
class GGPOSTEAM_API AGGPOGameState : public AGameStateBase
{
	GENERATED_BODY()
	
public:
	UFUNCTION(Server, Reliable, WithValidation)
	void ServerPlayerLoaded(AGGPOPlayerState* PlayerState);

	UFUNCTION(NetMulticast, Reliable, WithValidation)
	void MulticastBeginGame(const TArray<FPlayerInfo>& InPlayerInfo);
};
