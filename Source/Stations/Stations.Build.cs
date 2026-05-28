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
			
			"CoreGameplay"
		});

		PrivateDependencyModuleNames.AddRange(new string[]
		{
			"Activities",
			"Items",
			"Recipes"
		});
		
		if (Target.bBuildEditor)
		{
			PrivateDependencyModuleNames.AddRange(new string[] { "UnrealEd" });
		}
	}
}