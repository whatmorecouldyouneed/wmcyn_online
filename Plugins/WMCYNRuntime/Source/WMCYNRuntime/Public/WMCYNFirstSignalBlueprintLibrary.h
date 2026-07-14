#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "WMCYNFirstSignalBlueprintLibrary.generated.h"

class UWidgetComponent;

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

    UFUNCTION(BlueprintCallable, Category = "WMCYN|UI", meta = (WorldContext = "WorldContextObject"))
    static bool PrepareLocalFirstSignalWidgetInteraction(
        const UObject* WorldContextObject,
        UWidgetComponent* WidgetHost);
};
