#pragma once

#include "CoreMinimal.h"
#include "WMCYNSessionItem.h"
#include "WMCYNSessionFixture.generated.h"

UENUM()
enum class EWMCYNFixtureKind : uint8
{
    Decor,
    Ashtray,
    Papers
};

/** Tray, grinder, ashtray and papers. Only the source's ashtray is also a pickup. */
UCLASS()
class WMCYNRUNTIME_API AWMCYNSessionFixture : public AWMCYNSessionItem
{
    GENERATED_BODY()
public:
    AWMCYNSessionFixture();
    virtual bool IsCarrier() const override { return bCarriesItems; }
    void SetCarriesItems(bool bCarries);
    EWMCYNFixtureKind GetKind() const { return Kind; }
    bool ContainsPoint(const FVector& WorldPoint) const;
protected:
    virtual void HandleTaken() override;
private:
    UPROPERTY(EditAnywhere, Category="WMCYN|Item") EWMCYNFixtureKind Kind = EWMCYNFixtureKind::Decor;
    /** Not in the source: a tray that lifts everything on it, so nobody has to bend to the table. */
    UPROPERTY(EditAnywhere, Category="WMCYN|Item") bool bCarriesItems = false;
};
