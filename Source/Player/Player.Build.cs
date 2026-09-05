using UnrealBuildTool;

public class Player : ModuleRules
{
	public Player(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[]
		{
			"Core",
			"CoreUObject",
			"Engine",

			"CoreGameplay",
			"UMG",

			// Public: AlchemistBase implements IActivityInputCapture in its own header.
			"Activities",
			"DeveloperSettings"
		});

		PrivateDependencyModuleNames.AddRange(new string[]
		{
			"EnhancedInput",
			"InputCore"
		});
	}
}
