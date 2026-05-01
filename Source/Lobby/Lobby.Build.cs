using UnrealBuildTool;

public class Lobby : ModuleRules
{
	public Lobby(ReadOnlyTargetRules Target) : base(Target)
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
			"EnhancedInput",
			"InputCore",
			"LevelSequence",
			"MovieScene",

			"CoreGameplay",
			"Player"
		});
	}
}
