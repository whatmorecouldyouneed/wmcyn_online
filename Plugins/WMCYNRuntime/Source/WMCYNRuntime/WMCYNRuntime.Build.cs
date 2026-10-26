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
            "UMG",
            "Slate",
            // Grooms (hair, eyebrows, goatee) are UGroomComponents. The groom
            // assets carry card LODs, so this is not strand-only cost.
            "HairStrandsCore",
            "SlateCore"
        });

        if (Target.Platform == UnrealTargetPlatform.Win64)
        {
            PublicSystemLibraries.Add("Advapi32.lib");
        }

        if (Target.Platform == UnrealTargetPlatform.Android)
        {
            PrivateDependencyModuleNames.Add("AndroidPermission");
        }
    }
}
