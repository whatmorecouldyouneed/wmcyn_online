#include "WMCYNVideoResolver.h"

#include "Async/Async.h"
#include "HAL/PlatformProcess.h"
#include "Misc/Paths.h"

#if PLATFORM_ANDROID
#include "Android/AndroidApplication.h"
#include "Android/AndroidJNI.h"
#include "Android/AndroidJavaEnv.h"
#endif

namespace
{
#if PLATFORM_WINDOWS
constexpr double ResolveTimeoutSeconds = 30.0;
// Electra plays H.264 and AAC in one stream. HLS keeps long videos and live streams adaptive.
const TCHAR* const FormatSelector =
    TEXT("best[vcodec^=avc1][acodec!=none][protocol*=m3u8][height<=1080]/best[vcodec^=avc1][acodec!=none]/best[acodec!=none][vcodec!=none]");

void ResolveOnPlatform(const FString& PageUrl, FString& Output, FString& Error)
{
    const FString Tool = FPaths::Combine(FPaths::ProjectDir(), TEXT("ThirdParty/yt-dlp/yt-dlp.exe"));
    if (!FPaths::FileExists(Tool))
    {
        Error = TEXT("Link resolver isn't installed");
        return;
    }
    void* ReadPipe = nullptr;
    void* WritePipe = nullptr;
    FPlatformProcess::CreatePipe(ReadPipe, WritePipe);
    // WMCYNVideoPolicy has already refused quotes, so the link can't break out of its argument.
    const FString Arguments = FString::Printf(
        TEXT("--ignore-config --no-playlist --no-warnings --no-cache-dir --socket-timeout 15 -g -f \"%s\" -- \"%s\""),
        FormatSelector, *PageUrl);
    FProcHandle Process = FPlatformProcess::CreateProc(*Tool, *Arguments, false, true, true, nullptr, 0, nullptr, WritePipe);
    if (!Process.IsValid())
    {
        FPlatformProcess::ClosePipe(ReadPipe, WritePipe);
        Error = TEXT("Link resolver couldn't start");
        return;
    }
    const double Started = FPlatformTime::Seconds();
    while (FPlatformProcess::IsProcRunning(Process))
    {
        Output += FPlatformProcess::ReadPipe(ReadPipe);
        if (FPlatformTime::Seconds() - Started > ResolveTimeoutSeconds)
        {
            FPlatformProcess::TerminateProc(Process, true);
            Error = TEXT("Link took too long to resolve");
            break;
        }
        FPlatformProcess::Sleep(0.1f);
    }
    Output += FPlatformProcess::ReadPipe(ReadPipe);
    FPlatformProcess::CloseProc(Process);
    FPlatformProcess::ClosePipe(ReadPipe, WritePipe);
}
#elif PLATFORM_ANDROID
void ResolveOnPlatform(const FString& PageUrl, FString& Output, FString& Error)
{
    JNIEnv* Env = FAndroidApplication::GetJavaEnv();
    // Optional: without the resolver dependency, links report that instead of asserting.
    static jmethodID Method = FJavaWrapper::FindMethod(Env, FJavaWrapper::GameActivityClassID,
        "AndroidThunkJava_WMCYNResolveVideo", "(Ljava/lang/String;)Ljava/lang/String;", true);
    if (!Env || !Method)
    {
        Error = TEXT("YouTube and Twitch aren't available on Quest yet");
        return;
    }
    jstring Url = Env->NewStringUTF(TCHAR_TO_UTF8(*PageUrl));
    auto Result = (jstring)FJavaWrapper::CallObjectMethod(Env, FJavaWrapper::GameActivityThis, Method, Url);
    Env->DeleteLocalRef(Url);
    Output = FJavaHelper::FStringFromLocalRef(Env, Result);
}
#else
void ResolveOnPlatform(const FString& PageUrl, FString& Output, FString& Error)
{
    Error = TEXT("Link resolver isn't available here");
}
#endif
}

FString WMCYNVideoResolver::ParseDirectUrl(const FString& Output)
{
    TArray<FString> Lines;
    Output.ParseIntoArrayLines(Lines);
    for (const FString& Line : Lines)
    {
        const FString Trimmed = Line.TrimStartAndEnd();
        if (Trimmed.StartsWith(TEXT("https://")) || Trimmed.StartsWith(TEXT("http://"))) return Trimmed;
    }
    return FString();
}

void WMCYNVideoResolver::Resolve(const FString& PageUrl, TFunction<void(const FString&, const FString&)> Done)
{
    Async(EAsyncExecution::ThreadPool, [PageUrl, Done = MoveTemp(Done)]() mutable
    {
        FString Output;
        FString Error;
        ResolveOnPlatform(PageUrl, Output, Error);
        const FString Direct = ParseDirectUrl(Output);
        if (Direct.IsEmpty() && Error.IsEmpty()) Error = TEXT("Couldn't load that link");
        AsyncTask(ENamedThreads::GameThread, [Direct, Error, Done = MoveTemp(Done)]() { Done(Direct, Error); });
    });
}
