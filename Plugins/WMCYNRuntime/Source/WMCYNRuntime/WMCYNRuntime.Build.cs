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
            "OnlineSubsystem",
            "OnlineSubsystemUtils",
            "UMG"
        });
    }
}
