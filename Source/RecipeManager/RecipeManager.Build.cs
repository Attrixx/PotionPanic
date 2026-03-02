using UnrealBuildTool;

public class RecipeManager : ModuleRules
{
    public RecipeManager(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicDependencyModuleNames.AddRange(new string[]
        {
            "Core",
            "CoreUObject",
            "CoreGameplay",
            "Engine",
            "Items",
            "Recipes",
            "Stations"
        });
    }
}
