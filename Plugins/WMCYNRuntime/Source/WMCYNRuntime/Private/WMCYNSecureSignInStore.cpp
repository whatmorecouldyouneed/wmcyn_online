#include "WMCYNSecureSignInStore.h"

#if PLATFORM_WINDOWS
#include "Windows/WindowsHWrapper.h"
#include <wincred.h>
#endif

bool WMCYNSecureSignInStore::Read(const FString& Scope, FString& Token, FString& DisplayLabel)
{
    Token.Reset();
    DisplayLabel.Reset();
#if PLATFORM_WINDOWS
    PCREDENTIALW Credential = nullptr;
    if (!CredReadW(*Scope, CRED_TYPE_GENERIC, 0, &Credential)) return false;
    const DWORD Size = Credential->CredentialBlobSize;
    const bool bValid = Size > 0 && Size <= CRED_MAX_CREDENTIAL_BLOB_SIZE && Size % sizeof(TCHAR) == 0;
    if (bValid)
    {
        Token = FString(Size / sizeof(TCHAR), reinterpret_cast<const TCHAR*>(Credential->CredentialBlob));
        DisplayLabel = Credential->UserName ? Credential->UserName : TEXT("");
    }
    if (Credential->CredentialBlob) FMemory::Memzero(Credential->CredentialBlob, Size);
    CredFree(Credential);
    return bValid;
#else
    return false;
#endif
}

bool WMCYNSecureSignInStore::Write(const FString& Scope, const FString& Token, const FString& DisplayLabel)
{
#if PLATFORM_WINDOWS
    const int32 Bytes = Token.Len() * sizeof(TCHAR);
    if (Bytes <= 0 || Bytes > CRED_MAX_CREDENTIAL_BLOB_SIZE) return false;
    CREDENTIALW Credential{};
    Credential.Type = CRED_TYPE_GENERIC;
    Credential.TargetName = const_cast<TCHAR*>(*Scope);
    Credential.UserName = const_cast<TCHAR*>(*DisplayLabel);
    Credential.CredentialBlobSize = Bytes;
    Credential.CredentialBlob = reinterpret_cast<LPBYTE>(const_cast<TCHAR*>(*Token));
    Credential.Persist = CRED_PERSIST_LOCAL_MACHINE;
    return CredWriteW(&Credential, 0) != 0;
#else
    return false;
#endif
}

bool WMCYNSecureSignInStore::Remove(const FString& Scope)
{
#if PLATFORM_WINDOWS
    return CredDeleteW(*Scope, CRED_TYPE_GENERIC, 0) != 0 || GetLastError() == ERROR_NOT_FOUND;
#else
    return true; // Unsupported platforms never persist credentials.
#endif
}
