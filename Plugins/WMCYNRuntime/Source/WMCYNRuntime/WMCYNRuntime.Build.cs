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
            "OnlineSubsystem",
            "OnlineSubsystemUtils",
            "UMG"
        });
    }
}
