using UnrealBuildTool;

public class WMCYNRuntimeEditor : ModuleRules
{
    public WMCYNRuntimeEditor(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

        PrivateDependencyModuleNames.AddRange(new[]
        {
            "AnimGraph",
            "BlueprintGraph",
            "Core",
            "CoreUObject",
            "Engine",
            "IKRig",
            "IKRigDeveloper",
            "Kismet",
            "UnrealEd",
            "WMCYNRuntime"
        });
    }
}
