// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "GGPONetSubsystem.h"
#include "GSLobbyGameState.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnLobbyPlayerListUpdated, const TArray<UGGPOPlayer*>&, PlayerList);

/**
 * 
 */
UCLASS()
class GGPOSTEAMUETEMPLATE_API AGSLobbyGameState : public AGameStateBase
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintAssignable, Category = "Lobby")
	FOnLobbyPlayerListUpdated OnPlayerListUpdated;

protected:
	UGGPONetSubsystem* GGPONetSubsystem;

public:
	UFUNCTION(BlueprintCallable, meta = (WorldContext = "WorldContextObject"), Category = "Lobby")
	static AGSLobbyGameState* GetLobbyGameState(UObject* WorldContextObject);

	UFUNCTION(BlueprintCallable, Server, Reliable, WithValidation, Category = "Lobby")
	void Server_RequestPlayerList();

	UFUNCTION(NetMulticast, Reliable, Category = "Lobby")
	void Multicast_UpdatePlayerList(const TArray<FPlayerInfo>& PlayerList);

protected:
	UFUNCTION()
	void OnPlayersChanged(const TArray<UGGPOPlayer*>& Players);

	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
};
