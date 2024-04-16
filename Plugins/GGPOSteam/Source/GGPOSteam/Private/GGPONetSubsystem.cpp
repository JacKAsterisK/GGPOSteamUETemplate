/*
===============================================================================
Project: GGPOSteamUETemplate
Author: JacKAsterisK
================================================================================
*/


#include "GGPONetSubsystem.h"
#include "Engine/Engine.h"
#include "OnlineSessionSettings.h"
#include "OnlineSubsystem.h"
#include "OnlineSubsystemUtils.h"
#include "GameFramework/PlayerState.h"
#include "Online/OnlineSessionNames.h"

UGGPONetSubsystem* UGGPONetSubsystem::Get(UObject* WorldContextObject)
{
	UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::Assert);
	UGameInstance* GameInstance = World ? World->GetGameInstance() : nullptr;
	return GameInstance ? GameInstance->GetSubsystem<UGGPONetSubsystem>() : nullptr;
}

void UGGPONetSubsystem::UpdatePlayerList(const TArray<FPlayerInfo>& InPlayers)
{
	Players.Empty();

	for (const FPlayerInfo& PlayerInfo : InPlayers)
	{
		// Loop through player controllers and check if the player is local
		bool bIsLocal = false;
		for (FConstPlayerControllerIterator Iterator = GetWorld()->GetPlayerControllerIterator(); Iterator; ++Iterator)
		{
			APlayerController* PlayerController = Iterator->Get();
			if (PlayerController)
			{
				APlayerState* PlayerState = PlayerController->PlayerState;
				if (PlayerState)
				{
					FName PlayerId = FName(PlayerState->GetUniqueId().GetUniqueNetId()->ToString());
					if (PlayerId == PlayerInfo.NetId)
					{
						bIsLocal = true;
						break;
					}
				}
			}
		}

		UGGPOPlayer* PlayerInfoObj = NewObject<UGGPOPlayer>();
		PlayerInfoObj->SetPlayerInfo(PlayerInfo, bIsLocal);

		Players.Add(PlayerInfoObj);
	}

	OnPlayersChanged.Broadcast(Players);
}

void UGGPONetSubsystem::CreateSession(const FString& Name)
{
	FOnlineSessionSettings SessionSettings;

	bIsServer = false;
	bool bIsLAN = false;

	SessionSettings.bIsLANMatch = bIsLAN;
	SessionSettings.NumPublicConnections = 4;
	SessionSettings.NumPrivateConnections = 4;
	SessionSettings.bShouldAdvertise = true;
	SessionSettings.bAllowJoinInProgress = true;
	SessionSettings.bAllowInvites = true;
	SessionSettings.bAllowJoinViaPresence = true;
	//SessionSettings.bAllowJoinViaPresenceFriendsOnly = true;
	SessionSettings.bIsDedicated = false;
	SessionSettings.bUsesPresence = !bIsLAN;
	SessionSettings.bUseLobbiesIfAvailable = true;

	FOnlineSessionSetting ExtraSetting;
	ExtraSetting.AdvertisementType = EOnlineDataAdvertisementType::ViaOnlineService;
	ExtraSetting.Data = FVariantData(*Name);
	SessionSettings.Settings.Add(FName("name"), ExtraSetting);

	if (SessionInterface)
	{
		CurrentSessionName = FName(Name);
		SessionInterface->CreateSession(*NetId, GameSessionName, SessionSettings);
	}
}

void UGGPONetSubsystem::FindSessions()
{
	if (SessionInterface)
	{
		SessionSearch = MakeShareable(new FOnlineSessionSearch());
		SessionSearch->bIsLanQuery = false; // Example: searching for non-LAN sessions
		SessionSearch->MaxSearchResults = 20; // Example: maximum number of results
		SessionSearch->QuerySettings.Set(SEARCH_PRESENCE, true, EOnlineComparisonOp::Equals); // Example: searching for sessions with presence

		// Start the session search
		SessionInterface->FindSessions(*NetId, SessionSearch.ToSharedRef());
	}
}

void UGGPONetSubsystem::JoinSession(USessionSearchResult* Session)
{
	bIsServer = false;

	if (SessionInterface)
	{
		const FUniqueNetId& NetIDRef = *NetId.ToSharedRef();
		CurrentSession = Session;
		SessionInterface->JoinSession(NetIDRef, GameSessionName, Session->GetSearchResult());
	}
}

UGGPOPlayer* UGGPONetSubsystem::GetPlayerFromState(const APlayerState* PlayerState) const
{
	if (PlayerState)
	{
		FName PlayerNetId = FName(PlayerState->GetUniqueId().GetUniqueNetId()->ToString());
		for (UGGPOPlayer* Player : Players)
		{
			if (Player->GetPlayerInfo().NetId == PlayerNetId)
			{
				return Player;
			}
		}
	}

	return nullptr;
}

void UGGPONetSubsystem::AddPlayer(APlayerController* PlayerController)
{
	FName PlayerNetId = FName(PlayerController->PlayerState->GetUniqueId().GetUniqueNetId()->ToString());
	CSteamID PlayerSteamId;
	GetPlayerIdFromController(PlayerController, PlayerSteamId);

	FPlayerInfo NewPlayerInfo;
	NewPlayerInfo.PlayerId = PlayerIdCounter++;
	NewPlayerInfo.NetId = PlayerNetId;
	NewPlayerInfo.SteamId = PlayerSteamId;
	NewPlayerInfo.SteamIdBytes = SteamHelpers::SteamIdToByteArray(PlayerSteamId);

	if (bSteamInitialized)
	{
		NewPlayerInfo.Name = SteamFriends()->GetFriendPersonaName(PlayerSteamId);
	}
	else
	{
		NewPlayerInfo.Name = FName(FString::Printf(TEXT("Player %s"), *NewPlayerInfo.NetId.ToString()));
	}

	UGGPOPlayer* NewPlayer = nullptr;
	for (UGGPOPlayer* Player : Players)
	{
		if (Player->GetPlayerInfo().NetId == PlayerNetId)
		{
			NewPlayer = Player;
			break;
		}
	}

	if (!NewPlayer)
	{
		NewPlayer = NewObject<UGGPOPlayer>();
		Players.Add(NewPlayer);
		OnPlayerLoggedIn.Broadcast(NewPlayer);
	}

	NewPlayer->SetPlayerInfo(NewPlayerInfo, PlayerController->IsLocalController());
	OnPlayersChanged.Broadcast(Players);
}

FName UGGPONetSubsystem::GetPlayerIdFromController(const APlayerController* PlayerController, CSteamID& SteamId)
{
	FName UniqueID = NAME_None;

	if (PlayerController)
	{
		if (PlayerController->PlayerState)
		{
			FUniqueNetIdPtr NetId = PlayerController->PlayerState->GetUniqueId().GetUniqueNetId();
			UniqueID = FName(NetId->ToString());
			SteamId = SteamHelpers::ConvertToSteamID(NetId);
		}

		if (UniqueID.IsNone())
		{
			ULocalPlayer* LocalPlayer = PlayerController->GetLocalPlayer();
			if (LocalPlayer)
			{
				UniqueID = FName(LocalPlayer->GetPreferredUniqueNetId()->ToString());
			}
		}
	}

	return UniqueID;
}

void UGGPONetSubsystem::ServerTravel(const FString& MapName)
{
	GetWorld()->ServerTravel(MapName + FString("?listen"), false, false);
}

void UGGPONetSubsystem::ClientTravel(const FString& MapName)
{
	APlayerController* PlayerController = GetWorld()->GetFirstPlayerController();
	if (PlayerController)
	{
		PlayerController->ClientTravel(MapName, ETravelType::TRAVEL_Absolute);
	}
}

void UGGPONetSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	OnlineSub = IOnlineSubsystem::Get();
	if (OnlineSub)
	{
		FName SubsystemName = OnlineSub->GetSubsystemName();
		UE_LOG(LogTemp, Log, TEXT("Current online subsystem: %s"), *SubsystemName.ToString());

		// Login to the online subsystem
		OnlineSub->GetIdentityInterface()->AddOnLoginCompleteDelegate_Handle(0, FOnLoginCompleteDelegate::CreateUObject(this, &UGGPONetSubsystem::OnLoginComplete));
		OnlineSub->GetIdentityInterface()->AutoLogin(0);

		SessionInterface = Online::GetSessionInterface(GetWorld());//OnlineSub->GetSessionInterface();
		if (SessionInterface.IsValid())
		{
			// Add all the delegates we need
			SessionInterface->OnCreateSessionCompleteDelegates.AddUObject(this, &UGGPONetSubsystem::OnCreateSessionComplete);
			SessionInterface->OnStartSessionCompleteDelegates.AddUObject(this, &UGGPONetSubsystem::OnStartSessionComplete);

			SessionInterface->OnFindSessionsCompleteDelegates.AddUObject(this, &UGGPONetSubsystem::OnFindSessionsComplete);
			SessionInterface->OnJoinSessionCompleteDelegates.AddUObject(this, &UGGPONetSubsystem::OnJoinSessionComplete);
			SessionInterface->OnSessionUserInviteAcceptedDelegates.AddUObject(this, &UGGPONetSubsystem::OnSessionUserInviteAccepted);

			SessionInterface->OnDestroySessionCompleteDelegates.AddUObject(this, &UGGPONetSubsystem::OnDestroySessionComplete);
			SessionInterface->OnEndSessionCompleteDelegates.AddUObject(this, &UGGPONetSubsystem::OnEndSessionComplete);
			SessionInterface->OnSessionFailureDelegates.AddUObject(this, &UGGPONetSubsystem::OnSessionFailure);

			SessionInterface->OnRegisterPlayersCompleteDelegates.AddUObject(this, &UGGPONetSubsystem::OnRegisterPlayersComplete);
			SessionInterface->OnUnregisterPlayersCompleteDelegates.AddUObject(this, &UGGPONetSubsystem::OnUnregisterPlayersComplete);
		}
	}

	bSteamInitialized = SteamAPI_Init();

	if (bSteamInitialized)
	{
		SteamId = SteamUser()->GetSteamID();
		UE_LOG(LogTemp, Log, TEXT("Steam initialized!"));
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Failed to initialize Steam!"));
	}

	bIsInitialized = true;
	bIsServer = false;
}

void UGGPONetSubsystem::Deinitialize()
{
	if (SessionInterface)
	{
		SessionInterface->EndSession(GameSessionName);
	}

	bIsServer = false;
	bIsInitialized = false;
	bSteamInitialized = false;
}

void UGGPONetSubsystem::OnLoginComplete(int32 LocalUserNum, bool bWasSuccessful, const FUniqueNetId& UserId, const FString& Error)
{
	if (bWasSuccessful)
	{
		NetId = UserId.AsShared();

		//// --------------------------------------------------------
		//// Add players through game mode login instead?
		//// --------------------------------------------------------
		//FPlayerInfo PlayerInfo;
		//PlayerInfo.NetId = FName(UserId.ToString());
		//if (bSteamInitialized)
		//{
		//	PlayerInfo.Name = SteamFriends()->GetPersonaName();
		//}
		//else
		//{
		//	PlayerInfo.Name = FName(FString::Printf(TEXT("Player %s"), *PlayerInfo.Id.ToString()));
		//}

		//UGGPOPlayer* PlayerInfoObj = NewObject<UGGPOPlayer>();
		//PlayerInfoObj->SetPlayerInfo(PlayerInfo);

		//Players.Add(PlayerInfoObj);

		//OnPlayerLoggedIn.Broadcast(PlayerInfoObj);
		//OnPlayersChanged.Broadcast(Players);
		//// --------------------------------------------------------

		UE_LOG(LogTemp, Log, TEXT("Logged in successfully!"));
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to log in!"));
	}
}

//void UGGPONetSubsystem::OnLobbyMatchList(LobbyMatchList_t* pParam, bool bIOFailure)
//{
//	if (bIOFailure)
//	{
//		UE_LOG(LogTemp, Error, TEXT("Failed to find sessions!"));
//		return;
//	}
//
//	TArray<FSession> Sessions;
//
//	for (int32 i = 0; i < pParam->m_nLobbiesMatching; i++)
//	{
//		CSteamID LobbySteamID = SteamMatchmaking()->GetLobbyByIndex(i);
//
//		FSession Session;
//		Session.RawId = LobbySteamID.ConvertToUint64();
//		Session.Id = FString::Printf(TEXT("%llu"), Session.RawId);
//		Session.Name = SteamMatchmaking()->GetLobbyData(LobbySteamID, "name");
//
//		Sessions.Add(Session);
//	}
//
//	OnSessionsFound.Broadcast(Sessions);
//}


void UGGPONetSubsystem::OnCreateSessionComplete(FName InSessionName, bool bWasSuccessful)
{
	bIsServer = true;
	SessionInterface->StartSession(InSessionName);
}

void UGGPONetSubsystem::OnStartSessionComplete(FName InSessionName, bool bWasSuccessful)
{
	OnSessionCreated.Broadcast(bWasSuccessful);
}

void UGGPONetSubsystem::OnFindSessionsComplete(bool bWasSuccessful)
{
	SessionSearchResults.Empty();

	if (bWasSuccessful)
	{
		// Get the search results
		TArray<FOnlineSessionSearchResult> SearchResults = SessionSearch->SearchResults;
		for (FOnlineSessionSearchResult& SearchResult : SearchResults)
		{
			FSession Session;
			FString SessionID = SearchResult.Session.SessionInfo->GetSessionId().ToString();
			Session.RawId = FCString::Atoi64(*SessionID);
			Session.SearchResult = SearchResult;
			Session.Id = FName(SessionID);
			Session.Name = FName(SearchResult.Session.SessionSettings.Settings.FindRef("name").Data.ToString());

			USessionSearchResult* Result = NewObject<USessionSearchResult>();
			Result->SetSession(Session);
			SessionSearchResults.Add(Result);
		}

		OnSessionsFound.Broadcast(SessionSearchResults);
	}
	else
	{
		OnSessionFindFailure.Broadcast(TEXT("Failed to find sessions!"));
	}
}

void UGGPONetSubsystem::OnJoinSessionComplete(FName InSessionName, EOnJoinSessionCompleteResult::Type Result)
{
	if (Result != EOnJoinSessionCompleteResult::Success)
	{
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("Failed to join session!"));
		return;
	}

	// Client travel to CurrentSession
	FString ConnectionInfo;
	SessionInterface->GetResolvedConnectString(InSessionName, ConnectionInfo);
	//FString ConnectionString = FString("/Game/Lobby/MAP_Lobby?" + ConnectionInfo);
	FString ConnectionString = ConnectionInfo;
	ClientTravel(ConnectionString);

	OnSessionJoined.Broadcast(CurrentSession);
	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("Joined session!"));
}

void UGGPONetSubsystem::OnSessionUserInviteAccepted(const bool bWasSuccessful, const int32 ControllerId, TSharedPtr<const FUniqueNetId> UserId, const FOnlineSessionSearchResult& InviteResult)
{

}

void UGGPONetSubsystem::OnDestroySessionComplete(FName InSessionName, bool bWasSuccessful)
{
	bIsServer = false;
}

void UGGPONetSubsystem::OnEndSessionComplete(FName InSessionName, bool bWasSuccessful)
{
	bIsServer = false;
}

void UGGPONetSubsystem::OnSessionFailure(const FUniqueNetId& SessionId, ESessionFailure::Type FailureType)
{
	bIsServer = false;
}

void UGGPONetSubsystem::OnRegisterPlayersComplete(FName InSessionName, const TArray<TSharedRef<const FUniqueNetId>>& PlayerIds, bool bWasSuccessful)
{
	for (TSharedRef<const FUniqueNetId> PlayerId : PlayerIds)
	{
		//// --------------------------------------------------------
		//// Add players through game mode login instead?
		//// --------------------------------------------------------
		//FPlayerInfo PlayerInfo;
		//PlayerInfo.NetId = FName(PlayerId->ToString());
		//if (bSteamInitialized)
		//{
		//	PlayerInfo.Name = SteamFriends()->GetFriendPersonaName(ConvertToSteamID(PlayerId));
		//}
		//else
		//{
		//	PlayerInfo.Name = FName(FString::Printf(TEXT("Player %s"), *PlayerInfo.Id.ToString()));
		//}

		//UGGPOPlayer* PlayerInfoObj = NewObject<UGGPOPlayer>();
		//PlayerInfoObj->SetPlayerInfo(PlayerInfo);

		//Players.Add(PlayerInfoObj);

		//OnPlayerLoggedIn.Broadcast(PlayerInfoObj);
		//OnPlayersChanged.Broadcast(Players);
		//// --------------------------------------------------------

		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, FString::Printf(TEXT("Player registered: %s"), *PlayerId->ToString()));
	}
}

void UGGPONetSubsystem::OnUnregisterPlayersComplete(FName InSessionName, const TArray<TSharedRef<const FUniqueNetId>>& PlayerIds, bool bWasSuccessful)
{

}
