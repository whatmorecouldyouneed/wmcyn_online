using UnrealBuildTool;

public class WMCYNRuntime : ModuleRules
{
    public WMCYNRuntime(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicDependencyModuleNames.AddRange(new[]
        {
            "Core",
            "CoreUObject",
            "Engine",
            "HeadMountedDisplay",
            "HTTP",
            "InputCore",
            "Json",
            "NetCore",
            "OnlineSubsystem",
            "OnlineSubsystemUtils",
            "UMG"
        });

        if (Target.Platform == UnrealTargetPlatform.Android)
        {
            PrivateDependencyModuleNames.Add("AndroidPermission");
        }
    }
}
