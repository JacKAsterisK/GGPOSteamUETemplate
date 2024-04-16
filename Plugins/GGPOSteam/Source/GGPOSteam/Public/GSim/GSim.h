// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GSim/GSimPlayer.h"
#include "GSim.generated.h"

class IGSimPlayer;

UCLASS()
class GGPOSTEAM_API AGSim : public AActor
{
	GENERATED_BODY()
	
protected:
	TArray<IGSimPlayer*> Players;

public:
	UPROPERTY(EditDefaultsOnly, Category = "GSim")
	TSubclassOf<APawn> PlayerPawnClass;

public:
	AGSim();
	
	const TArray<IGSimPlayer*>& GetPlayers() const { return Players; }

	virtual void InitPlayers();
	virtual void BeginSim();
	virtual void EndSim();

	// Called every frame
	virtual void Tick(float DeltaTime) override;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
};
