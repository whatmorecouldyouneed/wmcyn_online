#pragma once

#include "CoreMinimal.h"

/**
 * Shared constants for the First Signal world runtime contract.
 * See Docs/FIRST_SIGNAL_WORLD_RUNTIME_CONTRACT.md.
 */
namespace WMCYNFirstSignalWorldContract
{
inline const TCHAR* WorldId = TEXT("wmcyn-online");
inline const TCHAR* LocationId = TEXT("wmcyn-crib");
inline const TCHAR* DefaultBuildId = TEXT("first-signal-001");
constexpr int32 ProtocolVersion = 1;
constexpr int32 Capacity = 4;
constexpr float HeartbeatIntervalSeconds = 10.0f;

/** Travel-URL option carrying the short-lived signed join ticket. */
inline const TCHAR* JoinTicketOption = TEXT("WMCYNTicket");

/** Returns the build id, honoring a -WMCYNBuildId= command-line override. */
WMCYNRUNTIME_API FString GetBuildId();

/** Quest on Android, PCVR elsewhere; -WMCYNPresenceMode= overrides. */
WMCYNRUNTIME_API FString GetPresenceModeHint();
}
