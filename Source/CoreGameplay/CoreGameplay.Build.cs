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
            "OnlineSubsystem"
        });

        PrivateDependencyModuleNames.AddRange(new string[]
        {
            "OnlineSubsystemSteam",
            "SteamSockets",
            "InputCore"
        });
    }
}
