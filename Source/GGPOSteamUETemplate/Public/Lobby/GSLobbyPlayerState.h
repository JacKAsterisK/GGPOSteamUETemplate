// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "GSLobbyPlayerState.generated.h"

/**
 * 
 */
UCLASS()
class GGPOSTEAMUETEMPLATE_API AGSLobbyPlayerState : public APlayerState
{
	GENERATED_BODY()
	
public:
	UFUNCTION(BlueprintCallable, Meta = (WorldContext = "WorldContextObject"), Category = "Lobby")
	static AGSLobbyPlayerState* GetFirstLocalLobbyPlayerState(UObject* WorldContextObject);

	UFUNCTION(BlueprintCallable, Server, Reliable, WithValidation, Category = "Lobby")
	void Server_RequestPlayerList();

protected:
	virtual void BeginPlay() override;
};
