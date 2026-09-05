using UnrealBuildTool;

public class Activities : ModuleRules
{
	public Activities(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[]
		{
			"Core",
			"CoreUObject",
			"Engine",
			"GameplayTags",

			// Public: the step widgets and the display component are part of this module's API.
			// UMG only -- reaching UserInterfaces from here would close a cycle through GameFlow,
			// so the concrete widgets compose those in Blueprint instead.
			"UMG",
			"DeveloperSettings"
		});

		PrivateDependencyModuleNames.AddRange(new string[]
		{
			"CoreGameplay",
			"Items"
		});
	}
}