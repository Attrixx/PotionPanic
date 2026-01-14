using UnrealBuildTool;

public class Stations : ModuleRules
{
    public Stations(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicDependencyModuleNames.AddRange(new string[]
        {
            "Core",
            "CoreUObject",
            "Engine",
            "CoreGameplay",
            "Items"
        });

        PrivateDependencyModuleNames.AddRange(new string[] 
        {
            "Interactions"
        });
    }
}
