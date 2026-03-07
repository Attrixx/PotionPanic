using UnrealBuildTool;

public class Recipes : ModuleRules
{
    public Recipes(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicDependencyModuleNames.AddRange(new string[]
        {
            "Core",
            "CoreUObject",
            "Engine",
            "GameplayTags",
            
            "CoreGameplay",
            "Items",
            "Activities"
        });
    }
}
