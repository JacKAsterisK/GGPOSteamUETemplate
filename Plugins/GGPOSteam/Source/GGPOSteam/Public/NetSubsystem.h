// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Steam/steam_api.h"
#include "OnlineSubsystem.h"
#include "Interfaces\OnlineSessionInterface.h"
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

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSessionCreated, bool, bWasSuccessful);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSessionsFound, TArray<FSession>, Sessions);

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

protected:
	bool bIsInitialized = false;
	//bool bUseSteam = true;

	IOnlineSubsystem* OnlineSub;
	IOnlineSessionPtr SessionInterface;
	TSharedPtr<FOnlineSessionSearch> SessionSearch;
	FName GameSessionName = "GGPOSteam";
	FName CurrentSessionName;

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
