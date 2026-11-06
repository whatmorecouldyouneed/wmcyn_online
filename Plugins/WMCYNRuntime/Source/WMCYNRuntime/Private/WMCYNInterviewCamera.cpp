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
    static ConstructorHelpers::FObjectFinder<UStaticMesh> Cylinder(TEXT("/Engine/BasicShapes/Cylinder.Cylinder"));
    auto Disc = [&](const TCHAR* Name, double X, double Diameter, double Depth)
    {
        auto* Part = AddBodyPart(Name, FVector(X,0,0), FVector(Diameter,Diameter,Depth));
        Part->SetStaticMesh(Cylinder.Object);
        Part->SetRelativeRotation(FRotator(90,0,0));
        return Part;
    };
    LensHousing = Disc(TEXT("LensHousing"), 3.6, 9, 3.3);
    Disc(TEXT("FocusRing"), 5.5, 9.6, 1.4);
    Disc(TEXT("LensGlass"), 6.25, 7.8, .2);
    AddBodyPart(TEXT("TopPlate"), FVector(0,0,6.75), FVector(4.3,21,.55));
    AddBodyPart(TEXT("Shutter"), FVector(0,10,7.3), FVector(2,2.8,.8));
    for (int32 Index = 0; Index < 3; ++Index)
        AddBodyPart(*FString::Printf(TEXT("GripRib%d"), Index), FVector(0,15,-5+Index*2), FVector(4.2,.6,.65));
    Lens = CreateDefaultSubobject<USceneComponent>(TEXT("Lens"));
    Lens->SetupAttachment(RootComponent);
    Lens->SetRelativeLocation(FVector(6.6,0,0));
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
    TInlineComponentArray<UStaticMeshComponent*> Parts(this);
    for (UStaticMeshComponent* Part : Parts)
    {
        const FLinearColor Color = Part->GetFName() == TEXT("LensGlass") ? FLinearColor(.025,.16,.2) :
            Part->GetFName() == TEXT("Shutter") ? FLinearColor(.68,.055,.03) :
            Part->GetFName() == TEXT("TopPlate") ? FLinearColor(.22,.24,.25) :
            Part->GetFName() == TEXT("FocusRing") ? FLinearColor(.09,.10,.11) : FLinearColor(.032,.035,.04);
        Part->SetVectorParameterValueOnMaterials(TEXT("Color"), FVector(Color.R,Color.G,Color.B));
    }
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
