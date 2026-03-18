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
		});

		PrivateDependencyModuleNames.AddRange(new string[]
		{
			"AudioMixer",
            "EnhancedInput",
			
			"CoreGameplay",
		});
	}
}
