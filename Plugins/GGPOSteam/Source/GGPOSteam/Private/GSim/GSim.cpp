// Fill out your copyright notice in the Description page of Project Settings.


#include "GSim/GSim.h"
#include "GGPOManager.h"
#include "GGPONetSubsystem.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerStart.h"

// Sets default values
AGSim::AGSim()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
}

void AGSim::InitPlayers()
{
	const TArray<UGGPOPlayer*>& PlayerList = UGGPONetSubsystem::Get(this)->GetPlayerList();

	if (!PlayerPawnClass)
	{
		UE_LOG(LogTemp, Error, TEXT("PlayerPawnClass is not set!"));
		return;
	}

	if (!PlayerPawnClass->ImplementsInterface(UGSimPlayer::StaticClass()))
	{
		UE_LOG(LogTemp, Error, TEXT("PlayerPawnClass does not implement IGSimPlayer interface!"));
		return;
	}

	// Find player starts
	TArray<AActor*> PlayerStarts;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), APlayerStart::StaticClass(), PlayerStarts);

	if (PlayerList.Num() > PlayerStarts.Num())
	{
		UE_LOG(LogTemp, Error, TEXT("Not enough player starts!"));
		return;
	}

	// Spawn players
	for (int i = 0; i < PlayerList.Num(); i++)
	{
		const UGGPOPlayer* PlayerInfo = PlayerList[i];
		APlayerStart* PlayerStart = Cast<APlayerStart>(PlayerStarts[i]);

		FActorSpawnParameters SpawnParams;
		SpawnParams.Owner = this;
		//SpawnParams.Instigator = Instigator;

		FVector SpawnLocation = GetActorLocation() + FVector(0, 0, 100 * i);

		IGSimPlayer* Player = Cast<IGSimPlayer>(GetWorld()->SpawnActor<APawn>(
			PlayerPawnClass, 
			PlayerStart->GetActorLocation(), 
			PlayerStart->GetActorRotation(), 
			SpawnParams
		));

		//Player->SetPlayerInfo(PlayerInfo[i]);
		//Player->SetPlayerIndex(i);
		//Player->SetPlayerName(PlayerInfo[i].PlayerName);
		//Player->SetPlayerColor(PlayerInfo[i].PlayerColor);
		//Player->SetPlayerType(PlayerInfo[i].PlayerType);
		//Player->SetPlayerController(PlayerInfo[i].PlayerController);

		Players.Add(Player);
	}
}

void AGSim::BeginSim()
{
}

void AGSim::EndSim()
{
}

// Called when the game starts or when spawned
void AGSim::BeginPlay()
{
	Super::BeginPlay();
}

// Called every frame
void AGSim::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

