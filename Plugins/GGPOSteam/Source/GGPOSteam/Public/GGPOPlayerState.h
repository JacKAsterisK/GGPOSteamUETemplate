// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "GGPOPlayerState.generated.h"

/**
 * 
 */
UCLASS()
class GGPOSTEAM_API AGGPOPlayerState : public APlayerState
{
	GENERATED_BODY()
	
public:
	UFUNCTION(Server, Reliable, WithValidation)
	void ServerPlayerLoaded();

protected:
	virtual void BeginPlay() override;
};
