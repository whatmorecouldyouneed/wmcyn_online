#pragma once

#include "Kismet/BlueprintFunctionLibrary.h"

#include "WMCYNRetargetEditorLibrary.generated.h"

class UAnimBlueprint;
class UIKRetargeter;
class USkeletalMesh;
class USkeleton;

UCLASS()
class WMCYNRUNTIMEEDITOR_API UWMCYNRetargetEditorLibrary : public UBlueprintFunctionLibrary
{
    GENERATED_BODY()

public:
    UFUNCTION(BlueprintCallable, Category="WMCYN|Avatar|Retarget")
    static bool BuildAttachedCopyPoseAnimGraph(UAnimBlueprint* AnimBlueprint);

    UFUNCTION(BlueprintCallable, Category="WMCYN|Avatar|Retarget")
    static bool BuildMetaHumanRetargetAnimGraph(UAnimBlueprint* AnimBlueprint, UIKRetargeter* Retargeter);

    /**
     * Point a skeletal mesh at a skeleton, merging the mesh's bone tree in first.
     *
     * USkeletalMesh::Skeleton is read-only to both Python and the editor
     * property bridge, so a mesh that was duplicated without its skeleton link
     * cannot be repaired from script. Unreal then refuses to animate it at all
     * ("has no skeleton. This needs to be fixed before an animation can be
     * set"), which is fatal for an avatar that is driven by a retarget
     * AnimBlueprint.
     *
     * Returns false and changes nothing when either argument is null or when
     * the skeleton cannot accept the mesh's bone hierarchy.
     */
    UFUNCTION(BlueprintCallable, Category="WMCYN|Avatar|Retarget")
    static bool AssignSkeletalMeshSkeleton(USkeletalMesh* Mesh, USkeleton* Skeleton);

    /**
     * Describe a mesh's own reference pose as JSON, for the named bones.
     *
     * Read-only diagnostic. A follower driven by SetLeaderPoseComponent takes
     * the leader's bone transforms but skins with its *own* inverse-bind
     * matrices, so a garment whose reference pose does not share the body's
     * orientation will be worn rotated. Asset bounds hint at that but cannot
     * prove it - bounds can be stale from an earlier import, whereas the
     * reference skeleton is what the skinning actually uses.
     *
     * Component-space translations are returned, composed up the parent chain.
     * Bones the mesh does not have are reported as missing rather than skipped,
     * because an absent bone is itself the answer to a question.
     */
    UFUNCTION(BlueprintCallable, Category="WMCYN|Avatar|Retarget")
    static FString DescribeReferencePose(USkeletalMesh* Mesh, const TArray<FString>& BoneNames);

    /**
     * Vertex count of one LOD, or -1 when it cannot be read.
     *
     * Needed to prove two meshes are topological duplicates before a groom
     * binding built against one is retargeted onto the other. UE 5.8 exposes no
     * such accessor to Python, and without it that check silently does nothing.
     */
    UFUNCTION(BlueprintCallable, Category="WMCYN|Avatar|Retarget")
    static int32 GetLODVertexCount(USkeletalMesh* Mesh, int32 LODIndex);
};
