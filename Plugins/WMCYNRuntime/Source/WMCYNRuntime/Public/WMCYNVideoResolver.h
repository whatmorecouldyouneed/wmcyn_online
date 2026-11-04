#pragma once

#include "CoreMinimal.h"

/**
 * Turns a YouTube or Twitch page link into a direct media URL, the way VRChat does: yt-dlp on each
 * client. Direct URLs are short-lived and tied to the requesting IP, so they are never replicated.
 */
namespace WMCYNVideoResolver
{
    /** Runs off the game thread. Done gets (DirectUrl, Error) back on the game thread. */
    WMCYNRUNTIME_API void Resolve(const FString& PageUrl, TFunction<void(const FString&, const FString&)> Done);
    /** First http or https line of yt-dlp output; empty when there isn't one. */
    WMCYNRUNTIME_API FString ParseDirectUrl(const FString& Output);
}
