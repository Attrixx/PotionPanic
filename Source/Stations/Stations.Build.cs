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
			"GameplayTags"
		});

		PrivateDependencyModuleNames.AddRange(new string[]
		{
			"CoreGameplay",
			"Activities",
			"Items",
			"Recipes"
		});
	}
}