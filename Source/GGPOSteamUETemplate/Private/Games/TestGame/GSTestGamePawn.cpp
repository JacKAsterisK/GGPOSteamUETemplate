// Fill out your copyright notice in the Description page of Project Settings.


#include "Games/TestGame/GSTestGamePawn.h"

// Sets default values
AGSTestGamePawn::AGSTestGamePawn()
{
 	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void AGSTestGamePawn::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AGSTestGamePawn::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

// Called to bind functionality to input
void AGSTestGamePawn::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

