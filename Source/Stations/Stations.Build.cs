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
            "CoreGameplay"
        });

        PrivateDependencyModuleNames.AddRange(new string[] 
        {
            "Activities",
            "Items",
            "Recipes"
        });
    }
}
