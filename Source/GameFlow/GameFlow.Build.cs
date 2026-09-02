using UnrealBuildTool;

public class GameFlow : ModuleRules
{
	public GameFlow(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[]
		{
			"Core",
			"CoreUObject",
			"Engine",
			
			"Stations",
			"CoreGameplay"
		});


		PrivateDependencyModuleNames.AddRange(new string[]
		{
			"Recipes",
			"Items",
			"Activities"
		});
	}
}