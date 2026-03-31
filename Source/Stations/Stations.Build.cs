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
			"GameplayTags",

			"Activities",
			"CoreGameplay"
		});

		PrivateDependencyModuleNames.AddRange(new string[]
		{
			"Items",
			"Recipes"
		});
		
		if (Target.bBuildEditor)
		{
			PrivateDependencyModuleNames.AddRange(new string[] { "UnrealEd" });
		}
	}
}
