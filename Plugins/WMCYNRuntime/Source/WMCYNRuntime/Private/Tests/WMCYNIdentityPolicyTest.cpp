#include "Misc/AutomationTest.h"
#include "WMCYNIdentityPolicy.h"

#if WITH_DEV_AUTOMATION_TESTS
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FWMCYNIdentityPolicyTest, "WMCYN.FirstSignal.Identity.PublicNames",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FWMCYNIdentityPolicyTest::RunTest(const FString& Parameters)
{
    using namespace WMCYNIdentityPolicy;
    TestEqual(TEXT("Account username beats generated player name"),
        PreferredName(TEXT("AP-123"), TEXT("jared"), TEXT("AP-456")), FString(TEXT("jared")));
    TestEqual(TEXT("Display name preferred"),
        PreferredName(TEXT("Jared"), TEXT("jared"), TEXT("AP-456")), FString(TEXT("Jared")));
    TestFalse(TEXT("UUID rejected"), IsUsableName(TEXT("00000000-0000-0000-0000-000000000000")));
    TestFalse(TEXT("Control characters rejected"), IsUsableName(TEXT("bad\nname")));
    TestFalse(TEXT("Empty rejected"), IsUsableName(FString()));
    TestTrue(TEXT("Missing names stay blank instead of leaking technical IDs"),
        PreferredName(TEXT("AP-123"), FString(), TEXT("AP-456")).IsEmpty());
    return true;
}
#endif
