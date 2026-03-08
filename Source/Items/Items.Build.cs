using UnrealBuildTool;

public class Items : ModuleRules
{
	public Items(ReadOnlyTargetRules Target) : base(Target)
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
			"Niagara",

			"CoreGameplay"
		});
	}
}