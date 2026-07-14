#include "Modules/ModuleManager.h"

#if PLATFORM_WINDOWS && defined(__clang__)
// Launcher UE binaries expose the Win32 C import while Clang headers reference UE's namespaced alias.
#pragma comment(linker, "/alternatename:__imp_?GetCurrentThreadId@Windows@@YAKXZ=__imp_GetCurrentThreadId")
#endif

IMPLEMENT_MODULE(FDefaultModuleImpl, WMCYNRuntime)
