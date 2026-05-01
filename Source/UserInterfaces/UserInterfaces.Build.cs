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
			"GameplayTags",
			"AudioMixer",
      		"EnhancedInput",
			"InputCore",
			"Slate",
			"SlateCore",

			"CoreGameplay",

			"HidApi",
			"Lobby"
		});

		if (Target.Platform == UnrealTargetPlatform.Win64)
		{
			// hidapi.h defaults its export macro to dllexport; as a consumer of HidApi.dll we
			// need the hid_* symbols declared as dllimport instead.
			PrivateDefinitions.Add("HID_API_NO_EXPORT_DEFINE=1");
			PrivateDefinitions.Add("HID_API_EXPORT=__declspec(dllimport)");
		}
	}
}
