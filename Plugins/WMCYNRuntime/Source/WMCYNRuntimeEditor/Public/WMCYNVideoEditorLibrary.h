#pragma once
#include "Kismet/BlueprintFunctionLibrary.h"
#include "WMCYNVideoEditorLibrary.generated.h"

UCLASS()
class UWMCYNVideoEditorLibrary : public UBlueprintFunctionLibrary
{
    GENERATED_BODY()
public:
    UFUNCTION(BlueprintCallable) static FString InspectClass(const FString& Path);
};
