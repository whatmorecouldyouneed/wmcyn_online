#pragma once

#include "CoreMinimal.h"

namespace WMCYNIdentityPolicy
{
inline bool IsUsableName(const FString& Value)
{
    if (Value.IsEmpty() || Value.Len() > 32 || Value != Value.TrimStartAndEnd() ||
        Value.StartsWith(TEXT("AP-"), ESearchCase::IgnoreCase)) return false;
    FGuid Guid;
    if (FGuid::ParseExact(Value, EGuidFormats::Digits, Guid) ||
        FGuid::ParseExact(Value, EGuidFormats::DigitsWithHyphens, Guid)) return false;
    for (TCHAR Character : Value)
        if (Character < 32 || Character == 127) return false;
    return true;
}

inline FString PreferredName(const FString& DisplayName, const FString& Username, const FString& PlayerName)
{
    for (const FString* Name : { &DisplayName, &Username, &PlayerName })
        if (IsUsableName(*Name)) return *Name;
    return FString();
}
}
