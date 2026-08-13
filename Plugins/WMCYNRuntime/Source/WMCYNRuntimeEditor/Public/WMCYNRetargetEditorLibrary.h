#pragma once

#include "Kismet/BlueprintFunctionLibrary.h"

#include "WMCYNRetargetEditorLibrary.generated.h"

class UAnimBlueprint;
class UIKRetargeter;

UCLASS()
class WMCYNRUNTIMEEDITOR_API UWMCYNRetargetEditorLibrary : public UBlueprintFunctionLibrary
{
    GENERATED_BODY()

public:
    UFUNCTION(BlueprintCallable, Category="WMCYN|Avatar|Retarget")
    static bool BuildMetaHumanRetargetAnimGraph(UAnimBlueprint* AnimBlueprint, UIKRetargeter* Retargeter);
};
