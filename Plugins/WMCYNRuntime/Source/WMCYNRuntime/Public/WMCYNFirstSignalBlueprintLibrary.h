#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "WMCYNFirstSignalBlueprintLibrary.generated.h"

UCLASS()
class WMCYNRUNTIME_API UWMCYNFirstSignalBlueprintLibrary : public UBlueprintFunctionLibrary
{
    GENERATED_BODY()

public:
    UFUNCTION(BlueprintCallable, Category = "WMCYN|Identity", meta = (WorldContext = "WorldContextObject"))
    static bool SubmitLocalFirstSignalIdentity(
        const UObject* WorldContextObject,
        const FString& Username,
        const FString& DisplayName);
};
