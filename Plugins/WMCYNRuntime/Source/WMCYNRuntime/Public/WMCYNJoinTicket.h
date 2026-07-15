#pragma once

#include "CoreMinimal.h"

/**
 * Server-side verification of the short-lived signed join ticket issued by
 * the WMCYN backend. Ticket format: v1.<base64url payload JSON>.<base64url
 * HMAC-SHA256 signature>. The signing secret is shared only between the
 * backend and the authoritative Unreal server; clients treat the ticket as
 * opaque and never hold the secret.
 */
struct WMCYNRUNTIME_API FWMCYNJoinTicketPayload
{
    FString TicketId;
    FString Uid;
    FString WorldId;
    FString LocationId;
    FString RuntimeId;
    FString BuildId;
    int32 ProtocolVersion = 0;
    FString Mode;
    FString Slot;
    TArray<FString> Capabilities;
    int64 IssuedAtUnix = 0;
    int64 ExpiresAtUnix = 0;
};

namespace WMCYNJoinTicket
{
/** HMAC-SHA256 over the UTF-8 message; exposed for tests. */
WMCYNRUNTIME_API void HmacSha256(
    TArrayView<const uint8> Key,
    TArrayView<const uint8> Message,
    uint8 OutDigest[32]);

/**
 * Verifies signature, structure, and expiry. Binding checks against the
 * current runtime (runtimeId, buildId, protocol) and replay tracking are the
 * caller's responsibility.
 */
WMCYNRUNTIME_API bool VerifyJoinTicket(
    const FString& Secret,
    const FString& Ticket,
    FWMCYNJoinTicketPayload& OutPayload);
}
