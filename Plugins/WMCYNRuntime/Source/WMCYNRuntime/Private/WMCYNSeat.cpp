#include "WMCYNSeat.h"
#include "Components/CapsuleComponent.h"
#include "Components/PoseableMeshComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/SkeletalMesh.h"
#include "Components/TextRenderComponent.h"
#include "Engine/World.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Net/UnrealNetwork.h"

AWMCYNSeat::AWMCYNSeat()
{
    bReplicates = true;
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.bStartWithTickEnabled = false;
    PrimaryActorTick.TickGroup = TG_PostUpdateWork;
    SetRootComponent(CreateDefaultSubobject<USceneComponent>(TEXT("Cushion")));
    Prompt = CreateDefaultSubobject<UTextRenderComponent>(TEXT("LocalPrompt"));
    Prompt->SetupAttachment(RootComponent);
    Prompt->SetRelativeLocation(FVector(4,0,45));
    Prompt->SetHorizontalAlignment(EHTA_Center);
    Prompt->SetWorldSize(4);
    Prompt->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    Prompt->SetCastShadow(false);
    Prompt->SetHiddenInSceneCapture(true);
    Prompt->SetVisibility(false);
}

void AWMCYNSeat::ShowPrompt(bool Visible, bool Seated)
{
    if (Visible) Prompt->SetText(FText::FromString(Seated ? TEXT("Right trigger to stand") : TEXT("Right trigger to sit")));
    Prompt->SetVisibility(Visible);
}

bool AWMCYNSeat::TrySit(ACharacter* User)
{
    if (!HasAuthority() || !IsValid(User) || IsValid(Occupant) || !User->GetController() ||
        FVector::DistSquared(GetActorLocation(),User->GetActorLocation()) > FMath::Square(150.f)) return false;
    auto* Move = User->GetCharacterMovement();
    // Keep the floor reference while moving horizontally into the cushion.
    // Raising the capsule to the cushion surface would also raise the headset.
    FVector Target = GetActorLocation();
    FHitResult Floor;
    const FVector Front = Target + 100 * GetActorForwardVector();
    if (!GetWorld()->LineTraceSingleByChannel(Floor, Front+FVector(0,0,35), Front-FVector(0,0,145),
            ECC_Visibility,FCollisionQueryParams(SCENE_QUERY_STAT(SeatFloor),false,User)) || !Move->IsWalkable(Floor)) return false;
    Target.Z = Floor.ImpactPoint.Z + User->GetCapsuleComponent()->GetScaledCapsuleHalfHeight()+2;
    if (!User->TeleportTo(Target,GetActorRotation(),false,true)) return false;
    OldMode = Move->MovementMode;
    OldCustomMode = Move->CustomMovementMode;
    Move->StopMovementImmediately();
    Move->DisableMovement();
    Occupant = User;
    OnRep_Occupant();
    ForceNetUpdate();
    return true;
}

bool AWMCYNSeat::TryStand(ACharacter* User)
{
    if (!HasAuthority() || !User || Occupant != User) return false;
    auto* Capsule = User->GetCapsuleComponent();
    FCollisionQueryParams Query(SCENE_QUERY_STAT(SeatExit),false,User);
    for (float Side : {0.f,60.f,-60.f})
    {
        const FVector Start = GetActorLocation()+100*GetActorForwardVector()+Side*GetActorRightVector()+FVector(0,0,35);
        FHitResult Floor;
        if (!GetWorld()->LineTraceSingleByChannel(Floor,Start,Start-FVector(0,0,145),ECC_Visibility,Query) ||
            !User->GetCharacterMovement()->IsWalkable(Floor)) continue;
        const FVector Exit = Floor.ImpactPoint + FVector(0,0,Capsule->GetScaledCapsuleHalfHeight()+3);
        if (GetWorld()->OverlapBlockingTestByProfile(Exit,FQuat::Identity,Capsule->GetCollisionProfileName(),
                Capsule->GetCollisionShape(),Query)) continue;
        if (User->TeleportTo(Exit,User->GetActorRotation(),false,false)) { Release(User); return true; }
    }
    return false;
}

void AWMCYNSeat::Release(ACharacter* User)
{
    if (!HasAuthority() || !User || Occupant != User) return;
    Occupant = nullptr;
    OnRep_Occupant();
    User->GetCharacterMovement()->SetMovementMode(static_cast<EMovementMode>(OldMode),OldCustomMode);
    ForceNetUpdate();
}

void AWMCYNSeat::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME(AWMCYNSeat,Occupant);
}

void AWMCYNSeat::ClearPresentation()
{
    if (IsValid(SeatBody)) SeatBody->DestroyComponent();
    SeatBody = nullptr;
    if (IsValid(SourceBody))
    {
        SourceBody->SetVisibility(true,false);
        SourceBody->VisibilityBasedAnimTickOption = static_cast<EVisibilityBasedAnimTickOption>(PreviousAnimTick);
        RemoveTickPrerequisiteComponent(SourceBody);
    }
    SourceBody = nullptr;
}

void AWMCYNSeat::OnRep_Occupant()
{
    ClearPresentation();
    RestoreCapsule();
    if (IsValid(Occupant))
    {
        CollisionUser = Occupant;
        PreviousCollision = Occupant->GetCapsuleComponent()->GetCollisionEnabled();
        Occupant->GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    }
    SetActorTickEnabled(Occupant && GetNetMode() != NM_DedicatedServer);
}

void AWMCYNSeat::RestoreCapsule()
{
    if (CollisionUser.IsValid()) CollisionUser->GetCapsuleComponent()->SetCollisionEnabled(PreviousCollision);
    CollisionUser.Reset();
}

void AWMCYNSeat::EndPlay(const EEndPlayReason::Type Reason)
{
    RestoreCapsule();
    ClearPresentation();
    Super::EndPlay(Reason);
}

void AWMCYNSeat::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);
    if (!IsValid(Occupant)) { OnRep_Occupant(); return; }
    if (!SeatBody)
    {
        for (auto* Body : TInlineComponentArray<USkeletalMeshComponent*>(Occupant.Get()))
        {
            if (!Body->IsVisible() || !Body->GetSkeletalMeshAsset() ||
                Body->GetSkeletalMeshAsset()->GetFName()!=TEXT("SKM_Manny_Simple")) continue;
            SourceBody = Body;
            PreviousAnimTick = static_cast<uint8>(Body->VisibilityBasedAnimTickOption);
            Body->VisibilityBasedAnimTickOption = EVisibilityBasedAnimTickOption::AlwaysTickPoseAndRefreshBones;
            SeatBody = NewObject<UPoseableMeshComponent>(Occupant);
            SeatBody->SetupAttachment(Body);
            SeatBody->SetSkinnedAssetAndUpdate(Body->GetSkeletalMeshAsset());
            SeatBody->SetCollisionEnabled(ECollisionEnabled::NoCollision);
            SeatBody->SetCanEverAffectNavigation(false);
            SeatBody->SetOwnerNoSee(Body->bOwnerNoSee);
            SeatBody->SetOnlyOwnerSee(Body->bOnlyOwnerSee);
            for (int32 I=0; I<Body->GetNumMaterials(); ++I) SeatBody->SetMaterial(I,Body->GetMaterial(I));
            Occupant->AddInstanceComponent(SeatBody);
            SeatBody->RegisterComponent();
            AddTickPrerequisiteComponent(Body);
            Body->SetVisibility(false,false);
            break;
        }
    }
    if (!SeatBody || !IsValid(SourceBody)) return;
    SeatBody->CopyPoseFromSkeletalComponent(SourceBody);
    const auto Space = EBoneSpaces::WorldSpace;
    SeatBody->SetBoneLocationByName(TEXT("pelvis"),GetActorLocation()+FVector(0,0,10),Space);
    // Preserve tracked hand/head transforms at the spine attachment. Only the legs
    // and pelvis are seated; the original animation graph continues running.
    SeatBody->SetBoneTransformByName(TEXT("spine_01"),SourceBody->GetSocketTransform(TEXT("spine_01")),Space);
    for (bool Left : {false,true})
    {
        const FName Hip = Left ? TEXT("thigh_l") : TEXT("thigh_r");
        const FName Knee = Left ? TEXT("calf_l") : TEXT("calf_r");
        const FName Foot = Left ? TEXT("foot_l") : TEXT("foot_r");
        auto Align = [&](FName Bone,FName Child,FVector Direction)
        {
            auto T = SeatBody->GetBoneTransformByName(Bone,Space);
            const FVector Axis = (SeatBody->GetBoneLocationByName(Child,Space)-T.GetLocation()).GetSafeNormal();
            if (!Axis.IsNearlyZero())
            {
                T.SetRotation((FQuat::FindBetweenNormals(Axis,Direction)*T.GetRotation()).GetNormalized());
                SeatBody->SetBoneTransformByName(Bone,T,Space);
            }
        };
        Align(Hip,Knee,GetActorForwardVector());
        Align(Knee,Foot,-FVector::UpVector);
        SeatBody->SetBoneRotationByName(Foot,SourceBody->GetSocketRotation(Foot),Space);
    }
}
