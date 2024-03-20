// Fill out your copyright notice in the Description page of Project Settings.


#include "NetSubsystem.h"
#include "OnlineSessionSettings.h"

void UNetSubsystem::CreateSession(const FString& Name)
{
	FOnlineSessionSettings SessionSettings;

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
	SessionSettings.Settings.Add("name", FVariantData(*Name));

	if (SessionInterface)
	{
		CurrentSessionName = FName(Name);
		SessionInterface->CreateSession(0, GameSessionName, SessionSettings);
	}

	//if (SteamAPI_Init())
	//{
	//	//// Create a new session
	//	//UE_LOG(LogTemp, Log, TEXT("Creating a new session..."));

	//	//// Create a new session
	//	//SteamMatchmaking()->CreateLobby(k_ELobbyTypePrivate, 2);
	//}
	//else
	//{
	//	UE_LOG(LogTemp, Error, TEXT("Steam is not initialized!"));
	//}
}

void UNetSubsystem::FindSessions()
{
	if (SessionInterface)
	{
		SessionSearch = MakeShareable(new FOnlineSessionSearch());
		SessionSearch->bIsLanQuery = false; // Example: searching for non-LAN sessions
		SessionSearch->MaxSearchResults = 20; // Example: maximum number of results
		SessionSearch->QuerySettings.Set(SEARCH_PRESENCE, true, EOnlineComparisonOp::Equals); // Example: searching for sessions with presence

		// Start the session search
		SessionInterface->FindSessions(0, SessionSearch.ToSharedRef());
	}

	//if (bUseSteam)
	//{
	//	if (bSteamInitialized)
	//	{
	//		// Find sessions
	//		UE_LOG(LogTemp, Log, TEXT("Finding sessions..."));

	//		// Find sessions
	//		SteamMatchmaking()->AddRequestLobbyListDistanceFilter(k_ELobbyDistanceFilterWorldwide);//k_ELobbyDistanceFilterClose);
	//		SteamMatchmaking()->AddRequestLobbyListStringFilter("game", "GGPOSteam", k_ELobbyComparisonEqual);
	//		SteamAPICall_t LobbyAPICall = SteamMatchmaking()->RequestLobbyList();

	//		SteamCallResultLobbies.Set(LobbyAPICall, this, &UNetSubsystem::OnLobbyMatchList);
	//	}
	//	else
	//	{
	//		UE_LOG(LogTemp, Error, TEXT("Steam is not initialized!"));
	//	}
	//}
	//else
	//{
	//	UE_LOG(LogTemp, Error, TEXT("Non-Steam sessions not yet implemented."));
	//	//UE_LOG(LogTemp, Log, TEXT("Finding sessions..."));
	//}
}

void UNetSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	OnlineSub = IOnlineSubsystem::Get();
	if (OnlineSub)
	{
		FName SubsystemName = OnlineSub->GetSubsystemName();
		UE_LOG(LogTemp, Log, TEXT("Current online subsystem: %s"), *SubsystemName.ToString());

		SessionInterface = OnlineSub->GetSessionInterface();
		if (SessionInterface.IsValid())
		{
			// Add all the delegates we need
			SessionInterface->OnCreateSessionCompleteDelegates.AddUObject(this, &UNetSubsystem::OnCreateSessionComplete);
			SessionInterface->OnStartSessionCompleteDelegates.AddUObject(this, &UNetSubsystem::OnStartSessionComplete);

			SessionInterface->OnFindSessionsCompleteDelegates.AddUObject(this, &UNetSubsystem::OnFindSessionsComplete);
			SessionInterface->OnJoinSessionCompleteDelegates.AddUObject(this, &UNetSubsystem::OnJoinSessionComplete);
			SessionInterface->OnSessionUserInviteAcceptedDelegates.AddUObject(this, &UNetSubsystem::OnSessionUserInviteAccepted);

			SessionInterface->OnDestroySessionCompleteDelegates.AddUObject(this, &UNetSubsystem::OnDestroySessionComplete);
			SessionInterface->OnEndSessionCompleteDelegates.AddUObject(this, &UNetSubsystem::OnEndSessionComplete);
			SessionInterface->OnSessionFailureDelegates.AddUObject(this, &UNetSubsystem::OnSessionFailure);

			SessionInterface->OnRegisterPlayersCompleteDelegates.AddUObject(this, &UNetSubsystem::OnRegisterPlayersComplete);
			SessionInterface->OnUnregisterPlayersCompleteDelegates.AddUObject(this, &UNetSubsystem::OnUnregisterPlayersComplete);
		}
	}

	bIsInitialized = true;
}

void UNetSubsystem::Deinitialize()
{
	if (SteamAPI_Init())
	{
		SteamAPI_Shutdown();
	}

	bIsInitialized = false;
	//bSteamInitialized = false;
}

//void UNetSubsystem::OnLobbyMatchList(LobbyMatchList_t* pParam, bool bIOFailure)
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
//		Session.RawID = LobbySteamID.ConvertToUint64();
//		Session.ID = FString::Printf(TEXT("%llu"), Session.RawID);
//		Session.Name = SteamMatchmaking()->GetLobbyData(LobbySteamID, "name");
//
//		Sessions.Add(Session);
//	}
//
//	OnSessionsFound.Broadcast(Sessions);
//}


void UNetSubsystem::OnCreateSessionComplete(FName InSessionName, bool bWasSuccessful)
{
	OnSessionCreated.Broadcast(bWasSuccessful);
}

void UNetSubsystem::OnStartSessionComplete(FName InSessionName, bool bWasSuccessful)
{

}

void UNetSubsystem::OnFindSessionsComplete(bool bWasSuccessful)
{
	// Populate the sessions array
	TArray<FSession> Sessions;

	if (bWasSuccessful)
	{
		// Get the search results
		TArray<FOnlineSessionSearchResult> SearchResults = SessionSearch->SearchResults;
		for (FOnlineSessionSearchResult& SearchResult : SearchResults)
		{
			FSession Session;
			FString SessionID = SearchResult.Session.SessionInfo->GetSessionId().ToString();
			Session.RawID = FCString::Atoi64(*SessionID);
			Session.ID = SessionID;
			Session.Name = SearchResult.Session.SessionSettings.Settings.FindRef("name").Data.ToString();

			Sessions.Add(Session);
		}
	}
}

void UNetSubsystem::OnJoinSessionComplete(FName InSessionName, EOnJoinSessionCompleteResult::Type Result)
{

}

void UNetSubsystem::OnSessionUserInviteAccepted(const bool bWasSuccessful, const int32 ControllerId, TSharedPtr<const FUniqueNetId> UserId, const FOnlineSessionSearchResult& InviteResult)
{

}

void UNetSubsystem::OnDestroySessionComplete(FName InSessionName, bool bWasSuccessful)
{

}

void UNetSubsystem::OnEndSessionComplete(FName InSessionName, bool bWasSuccessful)
{

}

void UNetSubsystem::OnSessionFailure(const FUniqueNetId& SessionId, ESessionFailure::Type FailureType)
{

}

void UNetSubsystem::OnRegisterPlayersComplete(FName InSessionName, const TArray<TSharedRef<const FUniqueNetId>>& PlayerIds, bool bWasSuccessful)
{

}

void UNetSubsystem::OnUnregisterPlayersComplete(FName InSessionName, const TArray<TSharedRef<const FUniqueNetId>>& PlayerIds, bool bWasSuccessful)
{

}
