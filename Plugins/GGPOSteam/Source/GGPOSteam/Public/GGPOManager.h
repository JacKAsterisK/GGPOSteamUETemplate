// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "ggponet.h"
#include "steam/steam_api.h"
#include "UObject/WeakObjectPtr.h"

#define GGPO_MAX_PLAYERS 4

class UNetManager;
class UFightSim;

typedef unsigned char byte;

struct FGGPOPlayerConnectionInfo
{
	GGPOPlayerType Type;
	CSteamID SteamID;
	//FString IPAddress;
	//int Port;
};

class GGPOManager
{
private:
	inline static GGPOSession* GSession = nullptr;

	//static GGPOPlayerConnectionInfo PlayerConnections[GGPO_MAX_PLAYERS];
	inline static int NumPlayers;

	inline static double LastTick;
	inline static int WaitFrames;
	inline static bool bSyncTest;
	inline static uint32 Rollbacks;

public:
	inline static bool bLocalOnly;
	inline static bool bIsInputFrame;
	inline static bool bLoadedGameState = false;
	inline static int LocalChecksum = 0;
	inline static int RemoteChecksum = 0;

public:
	static bool InitializeGGPOSession(bool bInSyncTest);
	static bool InitializeGGPOSpectatorSession();
	static bool DestroyGGPOSession();

	static GGPOErrorCode SetLocalInputs(GGPOPlayerHandle PlayerHandle, byte* Buffer, int BufferSize);
	static void Tick(float DeltaSeconds);

protected:
	static bool AdvanceFrame(bool GGPOCallback);

	static bool BeginGameCallback(const char* game);
	static bool AdvanceFrameCallback(int flags);
	static bool LoadGameStateCallback(unsigned char* buffer, int len);
	static bool SaveGameStateCallback(unsigned char** buffer, int* len, int* checksum, int frame);
	static bool LogGameStateCallback(char* filename, unsigned char* buffer, int len);
	static void FreeBufferCallback(void* buffer);
	static bool OnEventCallback(GGPOEvent* info);
};