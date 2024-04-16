// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "GGPOGameMode.generated.h"

/**
 * 
 */
UCLASS()
class GGPOSTEAM_API AGGPOGameMode : public AGameModeBase
{
	GENERATED_BODY()
	
protected:
	virtual void RestartPlayer(AController* Player) override;

	virtual void PostLogin(APlayerController* NewPlayer) override;
};
