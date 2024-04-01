// Copyright Epic Games, Inc. All Rights Reserved.


#include "GGPOSteamUETemplateGameModeBase.h"
#include "GGPONetSubsystem.h"

void AGGPOSteamUETemplateGameModeBase::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);

	UGGPONetSubsystem* GGPONetSubsystem = GetGameInstance()->GetSubsystem<UGGPONetSubsystem>();
	if (GGPONetSubsystem)
	{
		GGPONetSubsystem->AddPlayer(NewPlayer);
	}
}
