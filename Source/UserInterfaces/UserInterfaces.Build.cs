using UnrealBuildTool;

public class UserInterfaces : ModuleRules
{
	public UserInterfaces(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

<<<<<<< HEAD
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
			"InputCore",
			
			"CoreGameplay",
		});
	}
=======
        PublicDependencyModuleNames.AddRange(new string[]
        {
            "Core",
            "CoreUObject",
            "Engine",
            "CoreGameplay",
            "UMG",
            "AudioMixer",
            "InputCore",
            "EnhancedInput"
        });
    }
>>>>>>> 4ee7636 (Add missing module to be able to build the project)
}
