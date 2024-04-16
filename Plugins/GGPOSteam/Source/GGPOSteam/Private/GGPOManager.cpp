#include "GGPOManager.h"
#include "GSim/GSim.h"
#include "GGPONetSubsystem.h"

#include "ProfilingDebugging/ScopedTimers.h"

//DECLARE_CYCLE_STAT(TEXT("GGPO Tick"), STAT_GGPO_Tick, STATGROUP_FightSim);

bool GGPOManager::InitializeGGPOSession(bool bInSyncTest, AGSim* InGSim)
{
    if (GSession)
	{
		UE_LOG(LogTemp, Error, TEXT("GGPOManager::InitializeGGPOSession: Session already exists"));
		return false;
	}

	GSim = InGSim;
	GNetSubsystem = UGGPONetSubsystem::Get(InGSim);

	if (!InGSim || !GNetSubsystem)
	{
		UE_LOG(LogTemp, Error, TEXT("GGPOManager::InitializeGGPOSession: Invalid GSim or GNetSubsystem"));
		return false;
	}

    bSyncTest = bInSyncTest;
    bIsInputFrame = true;

    // Set up GGPO session callbacks
    GGPOSessionCallbacks cb = {0};
    FMemory::Memset(&cb, 0, sizeof(cb));
    cb.begin_game = GGPOManager::BeginGameCallback;
    cb.advance_frame = GGPOManager::AdvanceFrameCallback;
    cb.load_game_state = GGPOManager::LoadGameStateCallback;
    cb.save_game_state = GGPOManager::SaveGameStateCallback;
    cb.log_game_state = GGPOManager::LogGameStateCallback;
    cb.free_buffer = GGPOManager::FreeBufferCallback;
    cb.on_event = GGPOManager::OnEventCallback;

	TArray<UGGPOPlayer*> Players = GNetSubsystem->GetPlayerList();
    NumPlayers = Players.Num();
    bLocalOnly = !bSyncTest && GNetSubsystem->GetLocalPlayers().Num() == NumPlayers;

    int InputDelay = 2;// TODO:UFightSimSubsystem::Get(InNetManager->GetWorld())->InputDelayFrames;

    if (!bLocalOnly)
    {
        // Initialize GGPO session
        int MaxSerializedSize = 1;// TODO: FFightInputState::GetMaxSerializedSize();

		char gameName[] = "GGPOSteam";
        if (bSyncTest)
		{
            GGPOErrorCode result = ggpo_start_synctest(&GSession, &cb, gameName, NumPlayers, MaxSerializedSize, 1);
		}
        else
        {
            GGPOErrorCode result = ggpo_start_session(&GSession, &cb, gameName, NumPlayers, MaxSerializedSize, 7000);
        }

        // automatically disconnect clients after 3000 ms and start our count-down timer
        // for disconnects after 1000 ms.   To completely disable disconnects, simply use
        // a value of 0 for ggpo_set_disconnect_timeout.
        ggpo_set_disconnect_timeout(GSession, 3000);
        ggpo_set_disconnect_notify_start(GSession, 1000);

        //ggpo_set_steam_legacy_messages(GSession, true);

        // Add players to the GGPO session
        for (int i = 0; i < Players.Num(); ++i)
        {
			UGGPOPlayer* Player = Players[i];

            GGPOPlayer player = {0};
            FMemory::Memset(&player, 0, sizeof(player));
            player.size = sizeof(player);
            player.player_num = i + 1;//NetManager->GetPlayerIndex(PlayerInfo->GetPlayerId()) + 1;
            if (Player->IsLocal())// && !PlayerInfo.IsSpectator())
            {
                player.type = GGPOPlayerType::GGPO_PLAYERTYPE_LOCAL;
            }
            else if (!Player->IsLocal())// && !PlayerInfo->IsSpectator())
		    {
                player.type = GGPOPlayerType::GGPO_PLAYERTYPE_REMOTE;
                player.u.remote.steam_id = Player->GetSteamId().ConvertToUint64();
                //player.u.remote.ip_address = /* Retrieve IP from SessionInterface and PlayerNetIds */;
                //player.u.remote.port = /* Retrieve Port from SessionInterface and PlayerNetIds */;
		    }
		    //else
		    //{
      //          player.type = GGPOPlayerType::GGPO_PLAYERTYPE_SPECTATOR;
		    //}

            Player->GGPOState.Type = player.type;

            GGPOErrorCode addPlayerResult = ggpo_add_player(GSession, &player, &Player->GGPOState.Handle);
            if (FAILED(addPlayerResult))
            {
                // Handle error in adding player
            }

            switch (Player->GGPOState.Type)
			{
				case GGPO_PLAYERTYPE_LOCAL:
				{
					Player->GGPOState.ConnectProgress = 100;
					Player->GGPOState.SetConnectState(GGPOPlayerConnectState::Connecting);
                    ggpo_set_frame_delay(GSession, Player->GetGGPOHandle(), InputDelay);
					break;
				}
				case GGPO_PLAYERTYPE_REMOTE:
				{
					//PlayerInfo->GGPOState.SetConnectState(GGPOPlayerConnectState::Initializing);
					break;
				}
				case GGPO_PLAYERTYPE_SPECTATOR:
				{
					//PlayerInfo->GGPOState.SetConnectState(GGPOPlayerConnectState::Connecting);
					break;
				}
			}

			Player->GGPOState.bIsInitialized = true;
        }

        // More initialization code here (e.g., setting initial game state)
        LastTick = FPlatformTime::Seconds();
        WaitFrames = 0;
    }

    GSim->BeginSim();
    //GSim->StoreState(0);

    return true;
}

bool GGPOManager::InitializeGGPOSpectatorSession()
{
    LastTick = FPlatformTime::Seconds();

    return true;
}

bool GGPOManager::DestroyGGPOSession()
{
	if (GSession)
	{
		GGPOErrorCode result = ggpo_close_session(GSession);
		if (FAILED(result))
		{
			// Handle GGPO session destruction failure
			return false;
		}
		else
		{
            GSession = nullptr;
			return true;
		}
	}
	else
	{
		// Handle GGPO session destruction failure
		return false;
	}
}

GGPOErrorCode GGPOManager::SetLocalInputs(GGPOPlayerHandle PlayerHandle, byte* Buffer, int BufferSize)
{
    if (bLocalOnly)
    {
		return GGPO_OK;
    }

    GGPOErrorCode Result = ggpo_add_local_input(GSession, PlayerHandle, (void*)Buffer, BufferSize);

    return Result;
}

// functions for GGPO
bool GGPOManager::BeginGameCallback(const char* game)
{
    //FightSim->BeginSim();

    return true;
}

void GGPOManager::Tick(float DeltaSeconds)
{
    //SCOPE_CYCLE_COUNTER(STAT_GGPO_Tick);

 //   //double TimeSeconds = FPlatformTime::Seconds();
 //   //float DeltaSeconds = (float)(TimeSeconds - LastTick);
 //   //LastTick = TimeSeconds;
 //   float OverTimeAllowance = 1.25f;
 //   bool bBehindFrame = DeltaSeconds > ((1.f / UFightSimSubsystem::GetFPS()) * OverTimeAllowance);

 //   bLoadedGameState = false;

 //   if (bLocalOnly)
	//{
	//	//FightSim->GetInputs();

	//	//FightSim->ResolveInputs();

	//	//FightSim->Tick();


	//	return;
	//}

 //   //if (DeltaSeconds < FightSim->SimTimeStep)
 //   //{
 //       ggpo_idle(GSession, 0);//(int)(DeltaSeconds * 1000));
 //   //}

 //   if (WaitFrames > 0)
	//{
	//	--WaitFrames;
	//	return;
	//}

 //   GGPOErrorCode Result = GGPO_OK;

 //   //Result = FightSim->GetInputs();

 //   if (GGPO_SUCCEEDED(Result))
	//{
 //       bool FrameAdvanced = AdvanceFrame(false);

 //       if (FrameAdvanced && !bLoadedGameState)
 //       {
	//		if (!bRenderOffFrame)
	//		{
 //               bIsInputFrame = false;
	//			//FightSim->Tick();
 //               bIsInputFrame = true;
	//		}
	//		else
	//		{
	//			bIsInputFrame = false;
	//		}
 //       }
 //   }
}

bool GGPOManager::AdvanceFrame(bool GGPOCallback)
{
 //   int BufferSize = FFightInputState::GetMaxSerializedSize() * GGPO_MAX_PLAYERS;
 //   byte* InputBuffer = new byte[BufferSize];
 //   int DisconnectFlags;
 //   GGPOErrorCode Result = ggpo_synchronize_input(GSession, (void*)InputBuffer, BufferSize, &DisconnectFlags);

 //   if (GGPO_SUCCEEDED(Result))
 //   {
 //       FightSim->ReceiveInputs(InputBuffer, BufferSize);

 //       FightSim->Tick();
 //       FightSim->ResolveInputs();

 //       FightSim->StoreState(ggpo_get_current_frame(GSession) + 1);

	//	if (bLoadedGameState)
	//	{
	//		bIsInputFrame = false;
	//		FightSim->Tick();
 //           //FightSim->ResolveInputs();
	//		bIsInputFrame = true;
	//	}

 //       ggpo_advance_frame(GSession);

 //       int LogId = 600;
 //       GEngine->AddOnScreenDebugMessage(LogId++, 0.0f, FColor::Green, FString::Printf(TEXT("Frame: %d"), FightSim->Frame));
 //       GEngine->AddOnScreenDebugMessage(LogId++, 0.0f, FColor::Green, FString::Printf(TEXT("Rollbacks: %d"), Rollbacks));

 //       GGPOPlayerHandle NetPlayerHandle;
 //       for (UFightPlayerInfo* PlayerInfo : NetManager->PlayerInfos)
	//	{
	//		if (!PlayerInfo->bIsLocal)
	//		{
 //               NetPlayerHandle = PlayerInfo->GGPOState.Handle;
	//			break;
	//		}
	//	}

 //       GGPONetworkStats Stats = {0};
 //       ggpo_get_network_stats(GSession, NetPlayerHandle, &Stats);
 //       GEngine->AddOnScreenDebugMessage(LogId++, 0.0f, FColor::Green, FString::Printf(TEXT("send_queue_len: %d"), Stats.network.send_queue_len));
 //       GEngine->AddOnScreenDebugMessage(LogId++, 0.0f, FColor::Green, FString::Printf(TEXT("recv_queue_len: %d"), Stats.network.recv_queue_len));
 //       GEngine->AddOnScreenDebugMessage(LogId++, 0.0f, FColor::Green, FString::Printf(TEXT("ping: %d"), Stats.network.ping));
 //       GEngine->AddOnScreenDebugMessage(LogId++, 0.0f, FColor::Green, FString::Printf(TEXT("kbps_sent: %d"), Stats.network.kbps_sent));
 //       FColor ChecksumColor = LocalChecksum == RemoteChecksum ? FColor::Green : FColor::Red;
 //       GEngine->AddOnScreenDebugMessage(LogId++, 0.0f, ChecksumColor, FString::Printf(TEXT("Remote Checksum: %d"), RemoteChecksum));
 //       GEngine->AddOnScreenDebugMessage(LogId++, 0.0f, ChecksumColor, FString::Printf(TEXT("Local Checksum: %d"), LocalChecksum));
 //       GEngine->AddOnScreenDebugMessage(LogId++, 0.0f, FColor::Green, FString::Printf(TEXT("local_frames_behind: %d"), Stats.timesync.local_frames_behind));
 //       GEngine->AddOnScreenDebugMessage(LogId++, 0.0f, FColor::Green, FString::Printf(TEXT("remote_frames_behind: %d"), Stats.timesync.remote_frames_behind));
 //   }
 //   else
 //   {
 //       return false;
	//}

    return true;
}

bool GGPOManager::AdvanceFrameCallback(int flags)
{
    bool bRes = AdvanceFrame(true);

    return bRes;
}

bool GGPOManager::LoadGameStateCallback(unsigned char* buffer, int len)
{
	//int BufferOffset = 0;
	////FightSimSaveState SaveState = FightSimSaveState::Deserialize((byte*)buffer, BufferOffset);
 //   bLoadedGameState = true;

	////FightSim->RecoverState(SaveState);

 //   if (FightSim->Frame >= 1)
	//{
 //       bIsInputFrame = false;
 //       FightSim->Tick();
 //       //FightSim->ResolveInputs();
 //       bIsInputFrame = true;
	//}

 //   Rollbacks++;

    return true;
}

bool GGPOManager::SaveGameStateCallback(unsigned char** buffer, int* len, int* checksum, int frame)
{
    //FightSim->SaveState(frame, (byte**)buffer, len, checksum);

    return true;
}

bool GGPOManager::LogGameStateCallback(char* filename, unsigned char* buffer, int len)
{
    // Implementation

    return true;
}

void GGPOManager::FreeBufferCallback(void* buffer)
{
    if (buffer)
    {
        FMemory::Free(buffer);
    }
}

bool GGPOManager::OnEventCallback(GGPOEvent* info)
{
//    int Progress;

   // switch (info->code) {
   //     case GGPO_EVENTCODE_CONNECTED_TO_PEER:
   //     {
   //         UFightPlayerInfo* PlayerInfo = NetManager->GetPlayerInfo(info->u.connected.player);
   //         GGPOPlayerConnectionInfo* ConnectInfo = &PlayerInfo->GGPOState;
   //         ConnectInfo->SetConnectState(GGPOPlayerConnectState::Synchronizing);
   //         break;
   //     }
   //     case GGPO_EVENTCODE_SYNCHRONIZING_WITH_PEER:
   //     {
   //         UFightPlayerInfo* PlayerInfo = NetManager->GetPlayerInfo(info->u.synchronizing.player);
   //         GGPOPlayerConnectionInfo* ConnectInfo = &PlayerInfo->GGPOState;
   //         Progress = 100 * info->u.synchronizing.count / info->u.synchronizing.total;
   //         ConnectInfo->UpdateConnectProgress(Progress);
   //         break;
   //     }
   //     case GGPO_EVENTCODE_SYNCHRONIZED_WITH_PEER:
   //     {
   //         UFightPlayerInfo* PlayerInfo = NetManager->GetPlayerInfo(info->u.synchronized.player);
   //         GGPOPlayerConnectionInfo* ConnectInfo = &PlayerInfo->GGPOState;
   //         ConnectInfo->UpdateConnectProgress(100);
   //         break;
   //     }
   //     case GGPO_EVENTCODE_RUNNING:
   //     {
   //         for (UFightPlayerInfo* PlayerInfo : NetManager->PlayerInfos)
			//{
			//	GGPOPlayerConnectionInfo* ConnectInfo = &PlayerInfo->GGPOState;
			//	ConnectInfo->SetConnectState(GGPOPlayerConnectState::Running);
			//}
   //         break;
   //     }
   //     case GGPO_EVENTCODE_CONNECTION_INTERRUPTED:
   //     {
   //         UFightPlayerInfo* PlayerInfo = NetManager->GetPlayerInfo(info->u.connection_interrupted.player);
   //         GGPOPlayerConnectionInfo* ConnectInfo = &PlayerInfo->GGPOState;
   //         uint64 CyclesNow = FPlatformTime::Cycles();
   //         int64 MillisecondsNow = FPlatformTime::ToMilliseconds64(CyclesNow);
   //         int32 SafeMillisecondsNow = (int32)FMath::Clamp(MillisecondsNow, 0ll, (int64)INT_MAX);
   //         ConnectInfo->SetDisconnectTimeout(SafeMillisecondsNow, info->u.connection_interrupted.disconnect_timeout);
   //         break;
   //     }
   //     case GGPO_EVENTCODE_CONNECTION_RESUMED:
   //     {
   //         UFightPlayerInfo* PlayerInfo = NetManager->GetPlayerInfo(info->u.connection_resumed.player);
   //         GGPOPlayerConnectionInfo* ConnectInfo = &PlayerInfo->GGPOState;
   //         ConnectInfo->SetConnectState(GGPOPlayerConnectState::Running);
   //         break;
   //     }
   //     case GGPO_EVENTCODE_DISCONNECTED_FROM_PEER:
   //     {
   //         UFightPlayerInfo* PlayerInfo = NetManager->GetPlayerInfo(info->u.disconnected.player);
   //         GGPOPlayerConnectionInfo* ConnectInfo = &PlayerInfo->GGPOState;
   //         ConnectInfo->SetConnectState(GGPOPlayerConnectState::Disconnected);
   //         break;
   //     }
   //     case GGPO_EVENTCODE_TIMESYNC:
   //     {
   //         WaitFrames = info->u.timesync.frames_ahead;
   //         break;
   //     }
   // }

    return true;
}