#pragma once

#include "CoreMinimal.h"

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

class SteamHelpers
{
public:
    static TArray<uint8> SteamIdToByteArray(const CSteamID& SteamId)
    {
        TArray<uint8> Bytes;
        uint64 Id = SteamId.ConvertToUint64();
        for (int i = 0; i < sizeof(uint64); ++i)
        {
            Bytes.Add((Id >> (i * 8)) & 0xFF);
        }
        return Bytes;
    }

    static CSteamID ByteArrayToSteamId(const TArray<uint8>& Bytes)
    {
        if (Bytes.Num() != sizeof(uint64))
        {
            // Handle error
            return CSteamID();
        }

        uint64 Id = 0;
        for (int i = 0; i < sizeof(uint64); ++i)
        {
            Id |= ((uint64)Bytes[i]) << (i * 8);
        }
        return CSteamID(Id);
    }

    static CSteamID ConvertToSteamID(FUniqueNetIdPtr InUniqueNetId)
    {
        return *(uint64*)InUniqueNetId->GetBytes();
    }
};