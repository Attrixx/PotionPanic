using UnrealBuildTool;

public class CoreGameplay : ModuleRules
{
	public CoreGameplay(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicDependencyModuleNames.AddRange(new string[]
        {
            "Core",
            "CoreUObject",
            "Engine",
            "GameplayTags",
            "OnlineSubsystem",
            "EnhancedInput",
            "CommonUI",
            "AudioMixer"
        });

        PrivateDependencyModuleNames.AddRange(new string[]
        {
            "OnlineSubsystemSteam",
            "SteamSockets",
            "InputCore",
        });
    }
}
