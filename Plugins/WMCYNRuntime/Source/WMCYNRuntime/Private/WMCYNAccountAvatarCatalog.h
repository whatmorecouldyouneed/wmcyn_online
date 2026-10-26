#pragma once

#include "WMCYNBackendSubsystem.h"

namespace WMCYNAccountAvatarCatalog
{
// Owner-approved imported Jared, not the older test account's scan. This is
// a cooked-in PCVR selection, not a claim that a cloud avatar job is published.
inline bool Resolve(const FString& VerifiedUid, const FString& Platform, FWMCYNAvatarManifest& Out)
{
    if (VerifiedUid != TEXT("lej1Rl5bzxScm9j3ph3VBOedTDC3") || Platform != TEXT("pcvr"))
    {
        return false;
    }
    const FString Root = TEXT("/Game/WMCYN/AvatarPipeline/Runtime/Jared/Preview/");
    Out = FWMCYNAvatarManifest();
    Out.bIsDefaultAvatar = false;
    Out.AvatarId = TEXT("jared-import-pcvr");
    Out.AvatarVersion = 1;
    Out.Platform = Platform;
    Out.DisplayName = TEXT("Jared");
    Out.CacheKey = TEXT("jared-import-pcvr:1");
    Out.DeliveryType = TEXT("runtime_mesh");
    Out.DeliveryAssetRoot = Root;
    Out.DeliverySkeletalMeshPath = Root + TEXT("SKM_Jared_Body_Preview.SKM_Jared_Body_Preview");
    Out.DeliveryHeadSkeletalMeshPath = Root + TEXT("SKM_Jared_Head_Preview.SKM_Jared_Head_Preview");
    Out.DeliveryAnimClassPath = TEXT("/Game/WMCYN/AvatarPipeline/Runtime/WMCYNAvatar_52b64e54/ABP_WMCYN_MetaHuman_Presentation.ABP_WMCYN_MetaHuman_Presentation_C");
    return true;
}
}
