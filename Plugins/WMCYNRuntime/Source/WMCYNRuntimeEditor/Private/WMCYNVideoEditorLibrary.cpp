#include "WMCYNVideoEditorLibrary.h"
#include "UObject/UnrealType.h"

FString UWMCYNVideoEditorLibrary::InspectClass(const FString& Path)
{
    UClass* Class = LoadObject<UClass>(nullptr, *Path);
    if (!Class) return TEXT("MISSING");
    FString Result;
    for (TFieldIterator<FProperty> P(Class); P; ++P) Result += FString::Printf(TEXT("PROPERTY %s %s\n"), *P->GetName(), *P->GetCPPType());
    for (TFieldIterator<UFunction> F(Class); F; ++F)
    {
        Result += TEXT("FUNCTION ") + F->GetName() + TEXT("(");
        for (TFieldIterator<FProperty> P(*F); P && P->HasAnyPropertyFlags(CPF_Parm); ++P)
            Result += FString::Printf(TEXT("%s %s flags=%llu;"), *P->GetName(), *P->GetCPPType(), uint64(P->GetPropertyFlags()));
        Result += TEXT(")\n");
    }
    return Result;
}
