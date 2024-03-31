// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "OnlineSubsystem.h"
#include "Interfaces\OnlineSessionInterface.h"

// This is taken directly from UE4 - OnlineSubsystemSteamPrivatePCH.h as a fix for the array_count macro
// @todo Steam: Steam headers trigger secure-C-runtime warnings in Visual C++. Rather than mess with _CRT_SECURE_NO_WARNINGS, we'll just
//	disable the warnings locally. Remove when this is fixed in the SDK
#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable:4996)
// #TODO check back on this at some point
#pragma warning(disable:4265) // SteamAPI CCallback< specifically, this warning is off by default but 4.17 turned it on....
#endif

#if PLATFORM_WINDOWS || PLATFORM_MAC || PLATFORM_LINUX

#pragma push_macro("ARRAY_COUNT")
#undef ARRAY_COUNT

#if USING_CODE_ANALYSIS
MSVC_PRAGMA(warning(push))
MSVC_PRAGMA(warning(disable : ALL_CODE_ANALYSIS_WARNINGS))
#endif	// USING_CODE_ANALYSIS

#include <steam/steam_api.h>

#if USING_CODE_ANALYSIS
MSVC_PRAGMA(warning(pop))
#endif	// USING_CODE_ANALYSIS

#include <steam/isteamapps.h>
#include <steam/isteamapplist.h>
//#include <OnlineSubsystemSteamTypes.h>
#pragma pop_macro("ARRAY_COUNT")

// @todo Steam: See above
#ifdef _MSC_VER
#pragma warning(pop)
#endif

// Add other steam stuff here

#endif

#include "NetSubsystem.generated.h"

USTRUCT(BlueprintType)
struct FSession
{
	GENERATED_BODY()

	uint64 RawID;

	UPROPERTY(BlueprintReadOnly)
	FString Name;

	UPROPERTY(BlueprintReadOnly)
	FString ID;
};

UCLASS(BlueprintType)
class GGPOSTEAM_API USessionSearchResult : public UObject
{
	GENERATED_BODY()

protected:
	FSession Session;

public:
	void SetSession(const FSession& InSession) { Session = InSession; }

	UFUNCTION(BlueprintCallable, Category = "SessionSearchResult")
	FSession GetSession() const { return Session; }
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSessionCreated, bool, bWasSuccessful);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSessionsFound, const TArray<USessionSearchResult*>&, Sessions);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSessionFindFailure, const FString&, Error);

/**
 * 
 */
UCLASS(BlueprintType)
class GGPOSTEAM_API UNetSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintAssignable, Category = "NetSubsystem")
	FOnSessionCreated OnSessionCreated;

	UPROPERTY(BlueprintAssignable, Category = "NetSubsystem")
	FOnSessionsFound OnSessionsFound;

	UPROPERTY(BlueprintAssignable, Category = "NetSubsystem")
	FOnSessionFindFailure OnSessionFindFailure;

protected:
	bool bIsInitialized = false;
	bool bSteamInitialized = false;
	//bool bUseSteam = true;

	CSteamID SteamID;
	FUniqueNetIdPtr NetID;

	IOnlineSubsystem* OnlineSub;
	IOnlineSessionPtr SessionInterface;
	TSharedPtr<FOnlineSessionSearch> SessionSearch;
	FName GameSessionName = "GGPOSteam";
	FName CurrentSessionName;

	UPROPERTY(BlueprintReadOnly, Category = "NetSubsystem")
	TArray<USessionSearchResult*> SessionSearchResults;

	//CCallResult<UNetSubsystem, LobbyMatchList_t> SteamCallResultLobbies;

public:
	UFUNCTION(BlueprintCallable, Category = "NetSubsystem")
	void CreateSession(const FString& Name);

	UFUNCTION(BlueprintCallable, Category = "NetSubsystem")
	void FindSessions();
	
protected:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

	//void OnLobbyMatchList(LobbyMatchList_t* pParam, bool bIOFailure);

	void OnLoginComplete(int32 LocalUserNum, bool bWasSuccessful, const FUniqueNetId& UserId, const FString& Error);

	void OnCreateSessionComplete(FName InSessionName, bool bWasSuccessful);
	void OnStartSessionComplete(FName InSessionName, bool bWasSuccessful);

	void OnFindSessionsComplete(bool bWasSuccessful);
	void OnJoinSessionComplete(FName InSessionName, EOnJoinSessionCompleteResult::Type Result);
	void OnSessionUserInviteAccepted(const bool bWasSuccessful, const int32 ControllerId, TSharedPtr<const FUniqueNetId> UserId, const FOnlineSessionSearchResult& InviteResult);

	void OnDestroySessionComplete(FName InSessionName, bool bWasSuccessful);
	void OnEndSessionComplete(FName InSessionName, bool bWasSuccessful);
	void OnSessionFailure(const FUniqueNetId& SessionId, ESessionFailure::Type FailureType);

	void OnRegisterPlayersComplete(FName InSessionName, const TArray<TSharedRef<const FUniqueNetId>>& PlayerIds, bool bWasSuccessful);
	void OnUnregisterPlayersComplete(FName InSessionName, const TArray<TSharedRef<const FUniqueNetId>>& PlayerIds, bool bWasSuccessful);
};
