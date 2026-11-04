#include "WMCYNHighEffectComponent.h"

#include "Camera/CameraComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "GameFramework/Actor.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Misc/ConfigCacheIni.h"
#include "UObject/ConstructorHelpers.h"

namespace
{
const TCHAR* const ComfortSection = TEXT("WMCYN.Comfort");
const TCHAR* const HighEffectKey = TEXT("bHighEffectEnabled");
}

UWMCYNHighEffectComponent::UWMCYNHighEffectComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.bStartWithTickEnabled = false;
    static ConstructorHelpers::FObjectFinder<UStaticMesh> Sphere(TEXT("/Engine/BasicShapes/Sphere.Sphere"));
    SphereMesh = Sphere.Object;
}

bool UWMCYNHighEffectComponent::IsEnabledByUser()
{
    bool bEnabled = true;
    GConfig->GetBool(ComfortSection, HighEffectKey, bEnabled, GGameUserSettingsIni);
    return bEnabled;
}

void UWMCYNHighEffectComponent::SetEnabledByUser(const bool bEnabled)
{
    GConfig->SetBool(ComfortSection, HighEffectKey, bEnabled, GGameUserSettingsIni);
    GConfig->Flush(false, GGameUserSettingsIni);
}

void UWMCYNHighEffectComponent::Play(UMaterialInterface* Material)
{
    if (!Material || !IsEnabledByUser()) return;
    if (!Shell)
    {
        UCameraComponent* Head = GetOwner()->FindComponentByClass<UCameraComponent>();
        if (!Head) return;
        Shell = NewObject<UStaticMeshComponent>(GetOwner());
        Shell->SetStaticMesh(SphereMesh);
        Shell->SetupAttachment(Head);
        Shell->SetRelativeScale3D(FVector(0.6f));
        Shell->SetCollisionEnabled(ECollisionEnabled::NoCollision);
        Shell->SetCastShadow(false);
        Shell->SetHiddenInSceneCapture(true);
        Shell->RegisterComponent();
        ShellMaterial = Shell->CreateDynamicMaterialInstance(0, Material);
    }
    Remaining = DurationSeconds;
    Shell->SetVisibility(true);
    SetComponentTickEnabled(true);
}

void UWMCYNHighEffectComponent::Stop()
{
    Remaining = 0.0f;
    Opacity = 0.0f;
    if (Shell) Shell->SetVisibility(false);
    SetComponentTickEnabled(false);
}

void UWMCYNHighEffectComponent::TickComponent(const float DeltaTime, const ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    if (!IsEnabledByUser())
    {
        Stop();
        return;
    }
    Remaining -= DeltaTime;
    const float Target = Remaining > FadeSeconds ? MaxOpacity : 0.0f;
    const float Rate = MaxOpacity / (Remaining > FadeSeconds ? RampSeconds : FadeSeconds);
    Opacity = FMath::FInterpConstantTo(Opacity, Target, DeltaTime, Rate);
    if (ShellMaterial) ShellMaterial->SetScalarParameterValue(TEXT("Opacity"), Opacity);
    if (Remaining <= 0.0f && Opacity <= 0.0f) Stop();
}

void UWMCYNHighEffectComponent::EndPlay(const EEndPlayReason::Type Reason)
{
    if (Shell) Shell->DestroyComponent();
    Shell = nullptr;
    Super::EndPlay(Reason);
}
