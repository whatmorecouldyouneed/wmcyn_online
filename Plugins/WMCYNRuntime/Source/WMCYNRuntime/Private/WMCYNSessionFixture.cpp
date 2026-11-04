#include "WMCYNSessionFixture.h"

#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Engine/World.h"
#include "EngineUtils.h"

namespace
{
// Tall enough to catch a joint or the papers lying on the tray.
const FVector CarrySpaceMargin(1.0f, 1.0f, 8.0f);
}

AWMCYNSessionFixture::AWMCYNSessionFixture()
{
    bGrabbable = false;
}

void AWMCYNSessionFixture::SetCarriesItems(const bool bCarries)
{
    bCarriesItems = bCarries;
    bGrabbable = bCarries;
}

void AWMCYNSessionFixture::HandleTaken()
{
    const UStaticMesh* StaticMesh = Mesh->GetStaticMesh();
    if (!bCarriesItems || !StaticMesh) return;
    const FBox Space = StaticMesh->GetBoundingBox().ExpandBy(CarrySpaceMargin);
    for (TActorIterator<AWMCYNSessionItem> It(GetWorld()); It; ++It)
    {
        AWMCYNSessionItem* Item = *It;
        if (Item == this || Item->GetHolder() || Item->GetCarrier() == this) continue;
        if (Space.IsInsideOrOn(Mesh->GetComponentTransform().InverseTransformPosition(Item->GetGripLocation())))
        {
            Item->ServerSetCarrier(this);
        }
    }
}

bool AWMCYNSessionFixture::ContainsPoint(const FVector& WorldPoint) const
{
    const UStaticMesh* StaticMesh = Mesh->GetStaticMesh();
    if (!StaticMesh) return false;
    const FVector Local = Mesh->GetComponentTransform().InverseTransformPosition(WorldPoint);
    return StaticMesh->GetBoundingBox().ExpandBy(1.0f).IsInsideOrOn(Local);
}
