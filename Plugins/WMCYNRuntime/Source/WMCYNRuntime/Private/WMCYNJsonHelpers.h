#pragma once

#include "CoreMinimal.h"
#include "Dom/JsonObject.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"

// Shared by the backend subsystem and the First Signal pairing action. These
// live in a named namespace rather than an anonymous one because the module
// builds with unity enabled: anonymous-namespace copies in two .cpp files land
// in the same translation unit and collide.
namespace WMCYNJson
{
inline bool ParseJsonObject(const FString& Body, TSharedPtr<FJsonObject>& OutObject)
{
    const TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Body);
    return FJsonSerializer::Deserialize(Reader, OutObject) && OutObject.IsValid();
}

inline FString ReadString(const TSharedPtr<FJsonObject>& Object, const TCHAR* Field)
{
    FString Value;
    return Object.IsValid() && Object->TryGetStringField(Field, Value) ? Value : FString();
}
}
