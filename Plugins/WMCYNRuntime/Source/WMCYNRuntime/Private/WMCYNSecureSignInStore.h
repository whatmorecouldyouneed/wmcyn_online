#pragma once

#include "CoreMinimal.h"

// Credentials never belong in SaveGame, config or diagnostic output.
namespace WMCYNSecureSignInStore
{
bool Read(const FString& Scope, FString& Token, FString& DisplayLabel);
bool Write(const FString& Scope, const FString& Token, const FString& DisplayLabel);
bool Remove(const FString& Scope);
}
