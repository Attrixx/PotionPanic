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
            "Interactions"
        });

        PrivateDependencyModuleNames.AddRange(new string[] 
        {
            "Items"
        });
    }
}
