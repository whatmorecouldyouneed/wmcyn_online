#include "WMCYNRetargetEditorLibrary.h"

#include "AnimGraphNode_RetargetPoseFromMesh.h"
#include "AnimGraphNode_Root.h"
#include "Animation/AnimBlueprint.h"
#include "EdGraph/EdGraph.h"
#include "EdGraph/EdGraphNode.h"
#include "Kismet2/BlueprintEditorUtils.h"
#include "Kismet2/KismetEditorUtilities.h"
#include "Retargeter/IKRetargeter.h"
#include "UObject/UnrealType.h"

namespace
{
    FAnimNode_RetargetPoseFromMesh* GetRetargetRuntimeNode(UAnimGraphNode_RetargetPoseFromMesh* Node)
    {
        if (!Node)
        {
            return nullptr;
        }

        FStructProperty* NodeProperty = FindFProperty<FStructProperty>(
            Node->GetClass(), TEXT("Node"));
        return NodeProperty
            ? NodeProperty->ContainerPtrToValuePtr<FAnimNode_RetargetPoseFromMesh>(Node)
            : nullptr;
    }

    FString PinNames(const UEdGraphNode* Node)
    {
        FString Names;
        if (!Node)
        {
            return Names;
        }

        for (const UEdGraphPin* Pin : Node->Pins)
        {
            if (Pin)
            {
                Names += FString::Printf(TEXT("%s(%d),"), *Pin->PinName.ToString(), static_cast<int32>(Pin->Direction));
            }
        }
        return Names;
    }
}

bool UWMCYNRetargetEditorLibrary::BuildMetaHumanRetargetAnimGraph(
    UAnimBlueprint* AnimBlueprint,
    UIKRetargeter* Retargeter)
{
    if (!AnimBlueprint || !Retargeter)
    {
        UE_LOG(LogTemp, Error, TEXT("WMCYN retarget bridge requires an AnimBlueprint and IK Retargeter."));
        return false;
    }

    TArray<UEdGraph*> Graphs;
    AnimBlueprint->GetAllGraphs(Graphs);

    UEdGraph* AnimGraph = nullptr;
    for (UEdGraph* Graph : Graphs)
    {
        if (Graph && Graph->GetFName() == TEXT("AnimGraph"))
        {
            AnimGraph = Graph;
            break;
        }
    }

    if (!AnimGraph)
    {
        UE_LOG(LogTemp, Error, TEXT("WMCYN retarget bridge could not find AnimGraph in %s."), *AnimBlueprint->GetPathName());
        return false;
    }

    TArray<UAnimGraphNode_RetargetPoseFromMesh*> RetargetNodes;
    AnimGraph->GetNodesOfClass<UAnimGraphNode_RetargetPoseFromMesh>(RetargetNodes);
    UAnimGraphNode_RetargetPoseFromMesh* RetargetNode = RetargetNodes.Num() > 0 ? RetargetNodes[0] : nullptr;

    AnimGraph->Modify();
    if (!RetargetNode)
    {
        RetargetNode = NewObject<UAnimGraphNode_RetargetPoseFromMesh>(AnimGraph);
        RetargetNode->SetFlags(RF_Transactional);
        RetargetNode->CreateNewGuid();
        AnimGraph->AddNode(RetargetNode, true, false);
        RetargetNode->PostPlacedNewNode();
    }

    FAnimNode_RetargetPoseFromMesh* RuntimeNode = GetRetargetRuntimeNode(RetargetNode);
    if (!RuntimeNode)
    {
        UE_LOG(LogTemp, Error, TEXT("WMCYN retarget bridge could not access the retarget node data."));
        return false;
    }

    RuntimeNode->RetargetFrom = ERetargetSourceMode::ParentSkeletalMeshComponent;
    RuntimeNode->IKRetargeterAsset = Retargeter;
    RuntimeNode->bSuppressWarnings = false;
    RetargetNode->ReconstructNode();

    UAnimGraphNode_Root* RootNode = FBlueprintEditorUtils::GetAnimGraphRoot(AnimGraph);
    if (!RootNode)
    {
        UE_LOG(LogTemp, Error, TEXT("WMCYN retarget bridge could not find AnimGraph root."));
        return false;
    }

    UEdGraphPin* SourceOutput = RetargetNode->FindPin(TEXT("Pose"), EGPD_Output);
    UEdGraphPin* RootInput = RootNode->FindPin(TEXT("Result"), EGPD_Input);
    if (!SourceOutput || !RootInput)
    {
        UE_LOG(LogTemp, Error,
            TEXT("WMCYN retarget bridge expected Pose -> Result pins. Retarget pins: %s Root pins: %s"),
            *PinNames(RetargetNode), *PinNames(RootNode));
        return false;
    }

    SourceOutput->BreakAllPinLinks();
    RootInput->BreakAllPinLinks();
    SourceOutput->MakeLinkTo(RootInput);

    FBlueprintEditorUtils::MarkBlueprintAsStructurallyModified(AnimBlueprint);
    FKismetEditorUtilities::CompileBlueprint(AnimBlueprint);
    UE_LOG(LogTemp, Display,
        TEXT("WMCYN retarget bridge compiled %s using %s."),
        *AnimBlueprint->GetPathName(), *Retargeter->GetPathName());
    return true;
}
