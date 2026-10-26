#include "WMCYNInterviewCamera.h"
#include "Components/StaticMeshComponent.h"
#include "Components/WidgetComponent.h"
#include "Engine/StaticMesh.h"
#include "Engine/TextureRenderTarget2D.h"
#include "UObject/ConstructorHelpers.h"
#include "Widgets/Images/SImage.h"

AWMCYNInterviewCamera::AWMCYNInterviewCamera()
{
    PrimaryActorTick.bCanEverTick = false;
    bReplicates = false;
    SetRootComponent(CreateDefaultSubobject<USceneComponent>(TEXT("CameraRoot")));
    static ConstructorHelpers::FObjectFinder<UStaticMesh> Cube(TEXT("/Engine/BasicShapes/Cube.Cube"));
    auto AddBodyPart = [this](const TCHAR* Name, FVector Position, FVector Size)
    {
        auto* Part = CreateDefaultSubobject<UStaticMeshComponent>(Name);
        Part->SetupAttachment(RootComponent);
        Part->SetStaticMesh(Cube.Object);
        Part->SetRelativeLocation(Position);
        Part->SetRelativeScale3D(Size / 100.0);
        Part->SetCollisionEnabled(ECollisionEnabled::NoCollision);
        Part->SetCastShadow(false);
        Part->SetCanEverAffectNavigation(false);
        return Part;
    };
    Body = AddBodyPart(TEXT("Body"), FVector::ZeroVector, FVector(4,22,13.25));
    Grip = AddBodyPart(TEXT("RightGrip"), FVector(0,13,-3), FVector(5,4,12));
    LensHousing = AddBodyPart(TEXT("LensHousing"), FVector(3.5,0,0), FVector(3,5,5));
    Lens = CreateDefaultSubobject<USceneComponent>(TEXT("Lens"));
    Lens->SetupAttachment(RootComponent);
    Lens->SetRelativeLocation(FVector(5.1,0,0));
    Display = CreateDefaultSubobject<UWidgetComponent>(TEXT("LiveDisplay"));
    Display->SetupAttachment(RootComponent);
    Display->SetRelativeLocation(FVector(-2.1,0,0));
    Display->SetRelativeRotation(FRotator(0,180,0));
    Display->SetRelativeScale3D(FVector(20.0/640.0));
    Display->SetDrawSize(FVector2D(640,360));
    Display->SetPivot(FVector2D(.5,.5));
    Display->SetWidgetSpace(EWidgetSpace::World);
    Display->SetBlendMode(EWidgetBlendMode::Opaque);
    Display->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    Display->SetCastShadow(false);
    Display->SetCanEverAffectNavigation(false);
    Display->SetTickWhenOffscreen(false);
}

void AWMCYNInterviewCamera::SetPreview(UTextureRenderTarget2D* Texture)
{
    PreviewBrush.SetResourceObject(Texture);
    PreviewBrush.ImageSize = FVector2D(640,360);
    Display->SetSlateWidget(SNew(SImage).Image(&PreviewBrush).Visibility(EVisibility::HitTestInvisible));
}

FVector AWMCYNInterviewCamera::GetGripLocation() const
{
    return Grip->GetComponentLocation();
}

void AWMCYNInterviewCamera::EndPlay(const EEndPlayReason::Type Reason)
{
    Display->SetSlateWidget(nullptr);
    PreviewBrush.SetResourceObject(nullptr);
    Super::EndPlay(Reason);
}
