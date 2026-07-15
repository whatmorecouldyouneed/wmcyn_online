#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "WMCYNFirstSignalBlueprintLibrary.generated.h"

class UWidgetComponent;
class UUserWidget;
class USoundBase;

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

    UFUNCTION(
        BlueprintCallable,
        Category = "WMCYN|Backend",
        meta = (WorldContext = "WorldContextObject", DisplayName = "Submit First Signal Login"))
    static bool SubmitFirstSignalLogin(
        const UObject* WorldContextObject,
        UUserWidget* LoginWidget,
        const FString& Identifier,
        const FString& Password,
        bool bAllowPIEDevelopmentFallback = true);

    UFUNCTION(BlueprintCallable, Category = "WMCYN|UI", meta = (WorldContext = "WorldContextObject"))
    static bool PrepareLocalFirstSignalWidgetInteraction(
        const UObject* WorldContextObject,
        UWidgetComponent* WidgetHost);

    UFUNCTION(BlueprintCallable, Category = "WMCYN|Presence", meta = (WorldContext = "WorldContextObject"))
    static bool RequestLocalFirstSignalRespawn(const UObject* WorldContextObject);

    UFUNCTION(BlueprintCallable, Category = "WMCYN|Audio", meta = (WorldContext = "WorldContextObject"))
    static void PlayFirstSignalFootstep(
        const UObject* WorldContextObject,
        bool bRightFoot,
        float Intensity,
        const FTransform& WorldTransform,
        USoundBase* LeftFootstep,
        USoundBase* RightFootstep);
};
