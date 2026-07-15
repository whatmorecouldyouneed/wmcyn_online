#include "WMCYNJoinTicket.h"

#include "Dom/JsonObject.h"
#include "Misc/Base64.h"
#include "Misc/DateTime.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"

namespace
{
// Minimal FIPS 180-4 SHA-256. UE 5.8 Core ships SHA-1 but no general-purpose
// SHA-256 hasher, and pulling in the SSL module for one HMAC is heavier than
// carrying the standard 60-line block function.
struct FSha256Context
{
    uint32 State[8];
    uint64 BitLength = 0;
    uint8 Buffer[64];
    uint32 BufferLength = 0;

    FSha256Context()
    {
        State[0] = 0x6a09e667u; State[1] = 0xbb67ae85u; State[2] = 0x3c6ef372u; State[3] = 0xa54ff53au;
        State[4] = 0x510e527fu; State[5] = 0x9b05688cu; State[6] = 0x1f83d9abu; State[7] = 0x5be0cd19u;
    }
};

constexpr uint32 GSha256K[64] = {
    0x428a2f98u, 0x71374491u, 0xb5c0fbcfu, 0xe9b5dba5u, 0x3956c25bu, 0x59f111f1u, 0x923f82a4u, 0xab1c5ed5u,
    0xd807aa98u, 0x12835b01u, 0x243185beu, 0x550c7dc3u, 0x72be5d74u, 0x80deb1feu, 0x9bdc06a7u, 0xc19bf174u,
    0xe49b69c1u, 0xefbe4786u, 0x0fc19dc6u, 0x240ca1ccu, 0x2de92c6fu, 0x4a7484aau, 0x5cb0a9dcu, 0x76f988dau,
    0x983e5152u, 0xa831c66du, 0xb00327c8u, 0xbf597fc7u, 0xc6e00bf3u, 0xd5a79147u, 0x06ca6351u, 0x14292967u,
    0x27b70a85u, 0x2e1b2138u, 0x4d2c6dfcu, 0x53380d13u, 0x650a7354u, 0x766a0abbu, 0x81c2c92eu, 0x92722c85u,
    0xa2bfe8a1u, 0xa81a664bu, 0xc24b8b70u, 0xc76c51a3u, 0xd192e819u, 0xd6990624u, 0xf40e3585u, 0x106aa070u,
    0x19a4c116u, 0x1e376c08u, 0x2748774cu, 0x34b0bcb5u, 0x391c0cb3u, 0x4ed8aa4au, 0x5b9cca4fu, 0x682e6ff3u,
    0x748f82eeu, 0x78a5636fu, 0x84c87814u, 0x8cc70208u, 0x90befffau, 0xa4506cebu, 0xbef9a3f7u, 0xc67178f2u
};

FORCEINLINE uint32 RotateRight(uint32 Value, uint32 Bits)
{
    return (Value >> Bits) | (Value << (32u - Bits));
}

void Sha256Transform(FSha256Context& Context, const uint8 Block[64])
{
    uint32 W[64];
    for (int32 Index = 0; Index < 16; ++Index)
    {
        W[Index] = (static_cast<uint32>(Block[Index * 4]) << 24) |
            (static_cast<uint32>(Block[Index * 4 + 1]) << 16) |
            (static_cast<uint32>(Block[Index * 4 + 2]) << 8) |
            static_cast<uint32>(Block[Index * 4 + 3]);
    }
    for (int32 Index = 16; Index < 64; ++Index)
    {
        const uint32 S0 = RotateRight(W[Index - 15], 7) ^ RotateRight(W[Index - 15], 18) ^ (W[Index - 15] >> 3);
        const uint32 S1 = RotateRight(W[Index - 2], 17) ^ RotateRight(W[Index - 2], 19) ^ (W[Index - 2] >> 10);
        W[Index] = W[Index - 16] + S0 + W[Index - 7] + S1;
    }

    uint32 A = Context.State[0], B = Context.State[1], C = Context.State[2], D = Context.State[3];
    uint32 E = Context.State[4], F = Context.State[5], G = Context.State[6], H = Context.State[7];

    for (int32 Index = 0; Index < 64; ++Index)
    {
        const uint32 S1 = RotateRight(E, 6) ^ RotateRight(E, 11) ^ RotateRight(E, 25);
        const uint32 Ch = (E & F) ^ ((~E) & G);
        const uint32 Temp1 = H + S1 + Ch + GSha256K[Index] + W[Index];
        const uint32 S0 = RotateRight(A, 2) ^ RotateRight(A, 13) ^ RotateRight(A, 22);
        const uint32 Maj = (A & B) ^ (A & C) ^ (B & C);
        const uint32 Temp2 = S0 + Maj;

        H = G; G = F; F = E; E = D + Temp1;
        D = C; C = B; B = A; A = Temp1 + Temp2;
    }

    Context.State[0] += A; Context.State[1] += B; Context.State[2] += C; Context.State[3] += D;
    Context.State[4] += E; Context.State[5] += F; Context.State[6] += G; Context.State[7] += H;
}

void Sha256Update(FSha256Context& Context, const uint8* Data, int32 Length)
{
    for (int32 Index = 0; Index < Length; ++Index)
    {
        Context.Buffer[Context.BufferLength++] = Data[Index];
        if (Context.BufferLength == 64u)
        {
            Sha256Transform(Context, Context.Buffer);
            Context.BitLength += 512u;
            Context.BufferLength = 0u;
        }
    }
}

void Sha256Final(FSha256Context& Context, uint8 OutDigest[32])
{
    Context.BitLength += static_cast<uint64>(Context.BufferLength) * 8u;

    Context.Buffer[Context.BufferLength++] = 0x80u;
    if (Context.BufferLength > 56u)
    {
        while (Context.BufferLength < 64u)
        {
            Context.Buffer[Context.BufferLength++] = 0x00u;
        }
        Sha256Transform(Context, Context.Buffer);
        Context.BufferLength = 0u;
    }
    while (Context.BufferLength < 56u)
    {
        Context.Buffer[Context.BufferLength++] = 0x00u;
    }
    for (int32 Index = 7; Index >= 0; --Index)
    {
        Context.Buffer[Context.BufferLength++] = static_cast<uint8>(Context.BitLength >> (Index * 8));
    }
    Sha256Transform(Context, Context.Buffer);

    for (int32 Index = 0; Index < 8; ++Index)
    {
        OutDigest[Index * 4] = static_cast<uint8>(Context.State[Index] >> 24);
        OutDigest[Index * 4 + 1] = static_cast<uint8>(Context.State[Index] >> 16);
        OutDigest[Index * 4 + 2] = static_cast<uint8>(Context.State[Index] >> 8);
        OutDigest[Index * 4 + 3] = static_cast<uint8>(Context.State[Index]);
    }
}

void Sha256(TArrayView<const uint8> Data, uint8 OutDigest[32])
{
    FSha256Context Context;
    Sha256Update(Context, Data.GetData(), Data.Num());
    Sha256Final(Context, OutDigest);
}

bool DecodeBase64Url(const FString& Input, TArray<uint8>& OutBytes)
{
    FString Standard = Input.Replace(TEXT("-"), TEXT("+")).Replace(TEXT("_"), TEXT("/"));
    while (Standard.Len() % 4 != 0)
    {
        Standard.AppendChar(TEXT('='));
    }
    return FBase64::Decode(Standard, OutBytes);
}

TArray<uint8> Utf8Bytes(const FString& Value)
{
    const FTCHARToUTF8 Converted(*Value);
    return TArray<uint8>(reinterpret_cast<const uint8*>(Converted.Get()), Converted.Length());
}
}

namespace WMCYNJoinTicket
{
void HmacSha256(TArrayView<const uint8> Key, TArrayView<const uint8> Message, uint8 OutDigest[32])
{
    constexpr int32 BlockSize = 64;
    uint8 KeyBlock[BlockSize];
    FMemory::Memzero(KeyBlock, BlockSize);

    if (Key.Num() > BlockSize)
    {
        Sha256(Key, KeyBlock);
    }
    else
    {
        FMemory::Memcpy(KeyBlock, Key.GetData(), Key.Num());
    }

    uint8 InnerPad[BlockSize];
    uint8 OuterPad[BlockSize];
    for (int32 Index = 0; Index < BlockSize; ++Index)
    {
        InnerPad[Index] = KeyBlock[Index] ^ 0x36u;
        OuterPad[Index] = KeyBlock[Index] ^ 0x5cu;
    }

    uint8 InnerDigest[32];
    {
        FSha256Context Context;
        Sha256Update(Context, InnerPad, BlockSize);
        Sha256Update(Context, Message.GetData(), Message.Num());
        Sha256Final(Context, InnerDigest);
    }
    {
        FSha256Context Context;
        Sha256Update(Context, OuterPad, BlockSize);
        Sha256Update(Context, InnerDigest, 32);
        Sha256Final(Context, OutDigest);
    }
}

bool VerifyJoinTicket(const FString& Secret, const FString& Ticket, FWMCYNJoinTicketPayload& OutPayload)
{
    if (Secret.IsEmpty() || Ticket.IsEmpty())
    {
        return false;
    }

    TArray<FString> Parts;
    Ticket.ParseIntoArray(Parts, TEXT("."), false);
    if (Parts.Num() != 3 || Parts[0] != TEXT("v1"))
    {
        return false;
    }

    const TArray<uint8> SecretBytes = Utf8Bytes(Secret);
    const TArray<uint8> MessageBytes = Utf8Bytes(Parts[0] + TEXT(".") + Parts[1]);
    uint8 ExpectedDigest[32];
    HmacSha256(SecretBytes, MessageBytes, ExpectedDigest);

    TArray<uint8> ActualDigest;
    if (!DecodeBase64Url(Parts[2], ActualDigest) || ActualDigest.Num() != 32)
    {
        return false;
    }

    // Constant-time comparison.
    uint8 Difference = 0;
    for (int32 Index = 0; Index < 32; ++Index)
    {
        Difference |= static_cast<uint8>(ExpectedDigest[Index] ^ ActualDigest[Index]);
    }
    if (Difference != 0)
    {
        return false;
    }

    TArray<uint8> PayloadBytes;
    if (!DecodeBase64Url(Parts[1], PayloadBytes))
    {
        return false;
    }
    const FUTF8ToTCHAR PayloadText(reinterpret_cast<const ANSICHAR*>(PayloadBytes.GetData()), PayloadBytes.Num());
    const FString PayloadString(PayloadText.Length(), PayloadText.Get());

    TSharedPtr<FJsonObject> Json;
    const TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(PayloadString);
    if (!FJsonSerializer::Deserialize(Reader, Json) || !Json.IsValid())
    {
        return false;
    }

    FWMCYNJoinTicketPayload Payload;
    Payload.TicketId = Json->GetStringField(TEXT("tid"));
    Payload.Uid = Json->GetStringField(TEXT("uid"));
    Payload.WorldId = Json->GetStringField(TEXT("worldId"));
    Payload.LocationId = Json->GetStringField(TEXT("locationId"));
    Payload.RuntimeId = Json->GetStringField(TEXT("runtimeId"));
    Payload.BuildId = Json->GetStringField(TEXT("buildId"));
    Payload.Mode = Json->GetStringField(TEXT("mode"));
    Payload.Slot = Json->GetStringField(TEXT("slot"));

    double ProtocolValue = 0.0;
    Json->TryGetNumberField(TEXT("protocolVersion"), ProtocolValue);
    Payload.ProtocolVersion = FMath::RoundToInt32(ProtocolValue);

    double IssuedValue = 0.0;
    double ExpiresValue = 0.0;
    Json->TryGetNumberField(TEXT("iat"), IssuedValue);
    Json->TryGetNumberField(TEXT("exp"), ExpiresValue);
    Payload.IssuedAtUnix = static_cast<int64>(IssuedValue);
    Payload.ExpiresAtUnix = static_cast<int64>(ExpiresValue);

    const TArray<TSharedPtr<FJsonValue>>* CapabilityValues = nullptr;
    if (Json->TryGetArrayField(TEXT("capabilities"), CapabilityValues) && CapabilityValues)
    {
        for (const TSharedPtr<FJsonValue>& Value : *CapabilityValues)
        {
            FString Capability;
            if (Value.IsValid() && Value->TryGetString(Capability))
            {
                Payload.Capabilities.Add(Capability);
            }
        }
    }

    if (Payload.TicketId.IsEmpty() || Payload.Uid.IsEmpty() || Payload.RuntimeId.IsEmpty())
    {
        return false;
    }
    if (Payload.ExpiresAtUnix <= FDateTime::UtcNow().ToUnixTimestamp())
    {
        return false;
    }

    OutPayload = MoveTemp(Payload);
    return true;
}
}
