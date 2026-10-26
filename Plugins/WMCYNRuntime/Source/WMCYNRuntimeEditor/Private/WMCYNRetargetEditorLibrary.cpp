#include "WMCYNRetargetEditorLibrary.h"

#include "AnimGraphNode_RetargetPoseFromMesh.h"
#include "AnimGraphNode_Root.h"
#include "AnimGraphNode_CopyPoseFromMesh.h"
#include "Animation/AnimBlueprint.h"
#include "EdGraph/EdGraph.h"
#include "EdGraph/EdGraphNode.h"
#include "Kismet2/BlueprintEditorUtils.h"
#include "Kismet2/KismetEditorUtilities.h"
#include "Animation/Skeleton.h"
#include "Engine/SkeletalMesh.h"
#include "Rendering/SkeletalMeshRenderData.h"
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

bool UWMCYNRetargetEditorLibrary::BuildAttachedCopyPoseAnimGraph(UAnimBlueprint* AnimBlueprint)
{
    if (!AnimBlueprint || !AnimBlueprint->GetPathName().StartsWith(TEXT("/Game/WMCYN/"))) return false;
    TArray<UEdGraph*> Graphs;
    AnimBlueprint->GetAllGraphs(Graphs);
    for (UEdGraph* Graph : Graphs)
    {
        if (!Graph || Graph->GetFName() != TEXT("AnimGraph")) continue;
        UAnimGraphNode_Root* Root = FBlueprintEditorUtils::GetAnimGraphRoot(Graph);
        if (!Root) return false;
        TArray<UAnimGraphNode_CopyPoseFromMesh*> Nodes;
        Graph->GetNodesOfClass(Nodes);
        Graph->Modify();
        auto* Node = Nodes.IsEmpty() ? NewObject<UAnimGraphNode_CopyPoseFromMesh>(Graph) : Nodes[0];
        if (Nodes.IsEmpty())
        {
            Node->SetFlags(RF_Transactional);
            Node->CreateNewGuid();
            Graph->AddNode(Node, true, false);
            Node->PostPlacedNewNode();
        }
        Node->Node.bUseAttachedParent = true;
        Node->ReconstructNode();
        auto* Output = Node->FindPin(TEXT("Pose"), EGPD_Output);
        auto* Input = Root->FindPin(TEXT("Result"), EGPD_Input);
        if (!Output || !Input) return false;
        Input->BreakAllPinLinks();
        Output->MakeLinkTo(Input);
        FBlueprintEditorUtils::MarkBlueprintAsStructurallyModified(AnimBlueprint);
        FKismetEditorUtilities::CompileBlueprint(AnimBlueprint);
        return AnimBlueprint->Status != BS_Error;
    }
    return false;
}

bool UWMCYNRetargetEditorLibrary::AssignSkeletalMeshSkeleton(USkeletalMesh* Mesh, USkeleton* Skeleton)
{
    if (!Mesh || !Skeleton)
    {
        UE_LOG(LogTemp, Error, TEXT("WMCYN skeleton assign needs both a mesh and a skeleton."));
        return false;
    }

    if (Mesh->GetSkeleton() == Skeleton)
    {
        UE_LOG(LogTemp, Display, TEXT("WMCYN skeleton assign: %s already uses %s."),
            *Mesh->GetPathName(), *Skeleton->GetPathName());
        return true;
    }

    // The skeleton has to recognise this mesh's bones before the link means
    // anything. For a mesh that already matches the skeleton's hierarchy this
    // is a no-op that still has to be asked for.
    if (!Skeleton->MergeAllBonesToBoneTree(Mesh))
    {
        UE_LOG(LogTemp, Error,
            TEXT("WMCYN skeleton assign: %s could not accept the bone tree of %s."),
            *Skeleton->GetPathName(), *Mesh->GetPathName());
        return false;
    }

    Mesh->SetSkeleton(Skeleton);
    Mesh->MarkPackageDirty();
    Skeleton->MarkPackageDirty();

    UE_LOG(LogTemp, Display, TEXT("WMCYN skeleton assign: %s now uses %s."),
        *Mesh->GetPathName(), *Skeleton->GetPathName());
    return true;
}

FString UWMCYNRetargetEditorLibrary::DescribeReferencePose(
    USkeletalMesh* Mesh, const TArray<FString>& BoneNames)
{
    if (!Mesh)
    {
        return TEXT("{\"error\":\"null_mesh\"}");
    }

    const FReferenceSkeleton& RefSkeleton = Mesh->GetRefSkeleton();
    const TArray<FTransform>& LocalPose = RefSkeleton.GetRefBonePose();

    // Compose each requested bone up to the root. The reference pose is stored
    // parent-relative, and only component space is comparable between two
    // different meshes.
    const auto ComponentSpace = [&RefSkeleton, &LocalPose](int32 BoneIndex)
    {
        FTransform Accumulated = FTransform::Identity;
        for (int32 Index = BoneIndex; Index != INDEX_NONE; Index = RefSkeleton.GetParentIndex(Index))
        {
            Accumulated = Accumulated * LocalPose[Index];
        }
        return Accumulated;
    };

    TArray<FString> Entries;
    for (const FString& BoneName : BoneNames)
    {
        const int32 BoneIndex = RefSkeleton.FindBoneIndex(FName(*BoneName));
        if (BoneIndex == INDEX_NONE)
        {
            Entries.Add(FString::Printf(TEXT("\"%s\":{\"missing\":true}"), *BoneName));
            continue;
        }
        const FTransform Bone = ComponentSpace(BoneIndex);
        const FVector Location = Bone.GetLocation();
        const FRotator Rotation = Bone.Rotator();
        Entries.Add(FString::Printf(
            TEXT("\"%s\":{\"loc\":[%.2f,%.2f,%.2f],\"rot\":[%.2f,%.2f,%.2f]}"),
            *BoneName,
            Location.X, Location.Y, Location.Z,
            Rotation.Pitch, Rotation.Yaw, Rotation.Roll));
    }

    return FString::Printf(TEXT("{\"mesh\":\"%s\",\"bones\":%d,\"refpose\":{%s}}"),
        *Mesh->GetPathName(),
        RefSkeleton.GetNum(),
        *FString::Join(Entries, TEXT(",")));
}

int32 UWMCYNRetargetEditorLibrary::GetLODVertexCount(USkeletalMesh* Mesh, int32 LODIndex)
{
    if (!Mesh)
    {
        return -1;
    }

    const FSkeletalMeshRenderData* RenderData = Mesh->GetResourceForRendering();
    if (!RenderData || !RenderData->LODRenderData.IsValidIndex(LODIndex))
    {
        return -1;
    }

    return RenderData->LODRenderData[LODIndex].GetNumVertices();
}
