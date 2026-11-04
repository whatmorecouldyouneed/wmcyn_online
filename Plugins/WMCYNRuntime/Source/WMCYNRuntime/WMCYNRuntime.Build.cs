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
            "ApplicationCore",
            "InputCore",
            "Json",
            "MediaAssets",
            "AudioMixer",
            "NetCore",
            "Niagara",
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
            PrivateDependencyModuleNames.Add("Launch");
            // The Quest resolver is opt-in: this file exists only once its dependency is approved.
            string ResolverPlugin = System.IO.Path.Combine(ModuleDirectory, "WMCYNRuntime_UPL.xml");
            if (System.IO.File.Exists(ResolverPlugin)) AdditionalPropertiesForReceipt.Add("AndroidPlugin", ResolverPlugin);
        }
    }
}
