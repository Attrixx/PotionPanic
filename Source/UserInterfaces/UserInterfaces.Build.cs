using UnrealBuildTool;

public class UserInterfaces : ModuleRules
{
	public UserInterfaces(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[]
		{
			"Core",
			"CoreUObject",
			"Engine",
			"UMG",
			"CommonUI",
			"CommonInput",
		});

		PrivateDependencyModuleNames.AddRange(new string[]
		{
			"ApplicationCore",
			"DeveloperSettings",
			"EnhancedInput",
			"InputCore",
			"Slate",
			"SlateCore",

			"CoreGameplay",
		});
	}
}
