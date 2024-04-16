/*
===============================================================================
Project: GGPOSteamUETemplate
Author: JacKAsterisK
================================================================================
*/

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "OnlineSubsystem.h"
#include "OnlineSessionSettings.h"
#include "Interfaces\OnlineSessionInterface.h"
#include "SteamHelpers.h"
#include "ggponet.h"

#include "GGPONetSubsystem.generated.h"

USTRUCT(BlueprintType)
struct FSession
{

	GENERATED_BODY()

	uint64 RawId;

	FOnlineSessionSearchResult SearchResult;

	UPROPERTY(BlueprintReadOnly)
	FName Name;

	UPROPERTY(BlueprintReadOnly)
	FName Id;
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

	UFUNCTION(BlueprintCallable, Category = "SessionSearchResult")
	FName GetName() const { return Session.Name; }

	const FOnlineSessionSearchResult& GetSearchResult() const { return Session.SearchResult; }
};

struct GGPOChecksumInfo {
	int Frame;
	int Checksum;
};

enum GGPOPlayerConnectState
{
	Connecting = 0,
	Synchronizing,
	Running,
	Disconnected,
	Disconnecting,
};

struct GGPOPlayerConnectionInfo
{
public:
	bool bIsInitialized;
	GGPOPlayerType Type;
	GGPOPlayerHandle Handle;
	GGPOPlayerConnectState ConnectState;
	int ConnectProgress;
	int DisconnectTimeout;
	int DisconnectStart;

	GGPOPlayerConnectionInfo()
		: Type(GGPOPlayerType::GGPO_PLAYERTYPE_SPECTATOR)
		, Handle(-1)
		, ConnectState(GGPOPlayerConnectState::Disconnected)
		, ConnectProgress(0)
		, DisconnectTimeout(0)
		, DisconnectStart(0)
	{}

	void SetConnectState(GGPOPlayerConnectState NewState)
	{
		ConnectState = NewState;
		ConnectProgress = 0;
	}

	void SetDisconnectTimeout(int When, int Timeout)
	{
		DisconnectStart = When;
		DisconnectTimeout = Timeout;
		ConnectState = GGPOPlayerConnectState::Disconnecting;
	}

	void UpdateConnectProgress(int Progress)
	{
		ConnectProgress = Progress;
	}
};

USTRUCT(BlueprintType)
struct FPlayerInfo
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "GGPOPlayer|Info")
	int32 PlayerId;

	UPROPERTY(BlueprintReadOnly, Category = "GGPOPlayer|Info")
	TArray<uint8> SteamIdBytes;

	CSteamID SteamId;

	UPROPERTY(BlueprintReadOnly, Category = "GGPOPlayer|Info")
	FName NetId;

	UPROPERTY(BlueprintReadOnly, Category = "GGPOPlayer|Info")
	FName Name;

	UPROPERTY(BlueprintReadOnly, Category = "GGPOPlayer|Info")
	bool bIsLocal { false };

	UPROPERTY(BlueprintReadOnly, Category = "GGPOPlayer|Info")
	bool bIsReady { false };

	UPROPERTY(BlueprintReadOnly, Category = "GGPOPlayer|Info")
	bool bIsLoaded { false };
};

UCLASS(BlueprintType)
class GGPOSTEAM_API UGGPOPlayer : public UObject
{
	GENERATED_BODY()

public:
	GGPOPlayerConnectionInfo GGPOState;

protected:
	FPlayerInfo Player;

public:
	void ResolveSteamId()
	{
		Player.SteamId = SteamHelpers::ByteArrayToSteamId(Player.SteamIdBytes);
	}

	void SetPlayerInfo(const FPlayerInfo& InPlayer, bool bIsLocal) 
	{ 
		Player = InPlayer; 
		ResolveSteamId(); 
		Player.bIsLocal = bIsLocal;
	}

	UFUNCTION(BlueprintCallable, Category = "GGPOPlayer")
	const FPlayerInfo& GetPlayerInfo() const { return Player; }

	GGPOPlayerHandle GetGGPOHandle() const { return GGPOState.Handle; }

	UFUNCTION(BlueprintCallable, Category = "GGPOPlayer")
	int32 GetPlayerId() const { return Player.PlayerId; }

	UFUNCTION(BlueprintCallable, Category = "GGPOPlayer")
	FName GetNetId() const { return Player.NetId; }

	CSteamID GetSteamId() const { return Player.SteamId; }

	UFUNCTION(BlueprintCallable, Category = "GGPOPlayer")
	const FName& GetName() const { return Player.Name; }

	UFUNCTION(BlueprintCallable, Category = "GGPOPlayer")
	bool IsLocal() const { return Player.bIsLocal; }

	UFUNCTION(BlueprintCallable, Category = "GGPOPlayer")
	bool IsReady() const { return Player.bIsReady; }

	UFUNCTION(BlueprintCallable, Category = "GGPOPlayer")
	bool SetIsReady(bool bReady) { return Player.bIsReady = bReady; }

	UFUNCTION(BlueprintCallable, Category = "GGPOPlayer")
	bool IsLoaded() const { return Player.bIsLoaded; }

	UFUNCTION(BlueprintCallable, Category = "GGPOPlayer")
	void SetIsLoaded(bool bLoaded) { Player.bIsLoaded = bLoaded; }
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSessionCreated, bool, bWasSuccessful);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSessionsFound, const TArray<USessionSearchResult*>&, Sessions);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSessionFindFailure, const FString&, Error);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSessionJoined, const USessionSearchResult*, SearchResult);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPlayerLoggedIn, const UGGPOPlayer*, Player);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPlayersChanged, const TArray<UGGPOPlayer*>&, Players);

/**
 * 
 */
UCLASS(BlueprintType)
class GGPOSTEAM_API UGGPONetSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintAssignable, Category = "NetSubsystem")
	FOnSessionCreated OnSessionCreated;

	UPROPERTY(BlueprintAssignable, Category = "NetSubsystem")
	FOnSessionsFound OnSessionsFound;

	UPROPERTY(BlueprintAssignable, Category = "NetSubsystem")
	FOnSessionFindFailure OnSessionFindFailure;

	UPROPERTY(BlueprintAssignable, Category = "NetSubsystem")
	FOnSessionJoined OnSessionJoined;

	UPROPERTY(BlueprintAssignable, Category = "NetSubsystem")
	FOnPlayerLoggedIn OnPlayerLoggedIn;

	UPROPERTY(BlueprintAssignable, Category = "NetSubsystem")
	FOnPlayersChanged OnPlayersChanged;

	UPROPERTY(BlueprintReadOnly, Category = "NetSubsystem")
	bool bIsServer { false };

	UPROPERTY(BlueprintReadOnly, Category = "NetSubsystem")
	TArray<UGGPOPlayer*> Players;

protected:
	bool bIsInitialized { false };
	bool bSteamInitialized { false };
	//bool bUseSteam = true;

	CSteamID SteamId;
	FUniqueNetIdPtr NetId;

	int PlayerIdCounter = 0;

	IOnlineSubsystem* OnlineSub;
	IOnlineSessionPtr SessionInterface;
	TSharedPtr<FOnlineSessionSearch> SessionSearch;
	USessionSearchResult* CurrentSession;
	FName GameSessionName = "GGPOSteam";
	FName CurrentSessionName;

	UPROPERTY(BlueprintReadOnly, Category = "NetSubsystem")
	TArray<USessionSearchResult*> SessionSearchResults;

	//CCallResult<UGGPONetSubsystem, LobbyMatchList_t> SteamCallResultLobbies;

public:
	UFUNCTION(BlueprintCallable, Meta = (WorldContext = "WorldContextObject"), Category = "NetSubsystem")
	static UGGPONetSubsystem* Get(UObject* WorldContextObject);

	void UpdatePlayerList(const TArray<FPlayerInfo>& InPlayers);

	UFUNCTION(BlueprintCallable, Category = "NetSubsystem")
	void CreateSession(const FString& Name);

	UFUNCTION(BlueprintCallable, Category = "NetSubsystem")
	void FindSessions();

	UFUNCTION(BlueprintCallable, Category = "NetSubsystem")
	void JoinSession(USessionSearchResult* Session);

	UFUNCTION(BlueprintCallable, Category = "NetSubsystem")
	TArray<UGGPOPlayer*>& GetPlayerList() { return Players; }

	UFUNCTION(BlueprintCallable, Category = "NetSubsystem")
	UGGPOPlayer* GetPlayerFromState(const APlayerState* PlayerState) const;

	TArray<UGGPOPlayer*> GetLocalPlayers() const { return Players.FilterByPredicate([](UGGPOPlayer* Player) { return Player->GetPlayerInfo().bIsLocal; }); }

	TArray<FPlayerInfo> GetPlayerInfo() const
	{
		TArray<FPlayerInfo> PlayerInfoList;
		for (UGGPOPlayer* Player : Players)
		{
			PlayerInfoList.Add(Player->GetPlayerInfo());
		}
		return PlayerInfoList;
	}

	void ClearPlayerList() { Players.Empty(); }

	void AddPlayer(APlayerController* PlayerController);

	static FName GetPlayerIdFromController(const APlayerController* PlayerController, CSteamID& SteamId);

	UFUNCTION(BlueprintCallable, Category = "NetSubsystem")
	void ServerTravel(const FString& MapName);

	UFUNCTION(BlueprintCallable, Category = "NetSubsystem")
	void ClientTravel(const FString& MapName);
	
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
