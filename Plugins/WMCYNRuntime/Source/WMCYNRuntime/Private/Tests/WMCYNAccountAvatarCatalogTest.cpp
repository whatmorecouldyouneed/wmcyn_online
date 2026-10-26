#include "Misc/AutomationTest.h"
#include "WMCYNAccountAvatarCatalog.h"

#if WITH_DEV_AUTOMATION_TESTS
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FWMCYNAccountAvatarCatalogTest,
    "WMCYN.Avatar.AccountCatalog", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FWMCYNAccountAvatarCatalogTest::RunTest(const FString&)
{
    FWMCYNAvatarManifest Manifest;
    TestFalse(TEXT("Unknown account keeps default"), WMCYNAccountAvatarCatalog::Resolve(TEXT("other-user"), TEXT("pcvr"), Manifest));
    TestFalse(TEXT("Display name is not identity"), WMCYNAccountAvatarCatalog::Resolve(TEXT("jared"), TEXT("pcvr"), Manifest));
    TestFalse(TEXT("Quest remains gated"), WMCYNAccountAvatarCatalog::Resolve(TEXT("lej1Rl5bzxScm9j3ph3VBOedTDC3"), TEXT("quest"), Manifest));
    TestTrue(TEXT("Approved PCVR account resolves"), WMCYNAccountAvatarCatalog::Resolve(TEXT("lej1Rl5bzxScm9j3ph3VBOedTDC3"), TEXT("pcvr"), Manifest));
    TestFalse(TEXT("Jared is selected"), Manifest.bIsDefaultAvatar);
    TestTrue(TEXT("No fabricated download URL"), Manifest.DeliveryUrl.IsEmpty());
    TestTrue(TEXT("WMCYN-owned mesh"), Manifest.DeliverySkeletalMeshPath.StartsWith(TEXT("/Game/WMCYN/AvatarPipeline/Runtime/Jared/")));
    return true;
}
#endif
