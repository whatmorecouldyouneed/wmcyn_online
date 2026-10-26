#include "Misc/AutomationTest.h"
#include "WMCYNSecureSignInStore.h"

#if WITH_DEV_AUTOMATION_TESTS && PLATFORM_WINDOWS
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FWMCYNSecureSignInStoreTest,
    "WMCYN.Auth.SecureCredentialRoundTrip",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FWMCYNSecureSignInStoreTest::RunTest(const FString& Parameters)
{
    // Unique test-only target: never reads or deletes the user's saved sign-in.
    const FString Scope = TEXT("WMCYN/Automation/") + FGuid::NewGuid().ToString();
    FString Token, Label;
    TestFalse(TEXT("Missing entry"), WMCYNSecureSignInStore::Read(Scope, Token, Label));
    TestTrue(TEXT("Store fixture"), WMCYNSecureSignInStore::Write(Scope, TEXT("synthetic-fixture-1"), TEXT("Test User")));
    TestTrue(TEXT("Read fixture"), WMCYNSecureSignInStore::Read(Scope, Token, Label));
    TestTrue(TEXT("Round trip"), Token == TEXT("synthetic-fixture-1") && Label == TEXT("Test User"));
    TestTrue(TEXT("Rotate fixture"), WMCYNSecureSignInStore::Write(Scope, TEXT("synthetic-fixture-2"), TEXT("Test User")));
    TestTrue(TEXT("Read rotated fixture"), WMCYNSecureSignInStore::Read(Scope, Token, Label));
    TestTrue(TEXT("Rotation preserved"), Token == TEXT("synthetic-fixture-2"));
    TestFalse(TEXT("Reject empty token"), WMCYNSecureSignInStore::Write(Scope, TEXT(""), TEXT("Test User")));
    TestTrue(TEXT("Delete fixture"), WMCYNSecureSignInStore::Remove(Scope));
    TestFalse(TEXT("Deleted entry unavailable"), WMCYNSecureSignInStore::Read(Scope, Token, Label));
    TestTrue(TEXT("Read clears outputs"), Token.IsEmpty() && Label.IsEmpty());
    TestTrue(TEXT("Idempotent delete"), WMCYNSecureSignInStore::Remove(Scope));
    return true;
}
#endif
