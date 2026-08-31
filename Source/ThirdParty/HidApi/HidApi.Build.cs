using System.IO;
using UnrealBuildTool;

public class HidApi : ModuleRules
{
	public HidApi(ReadOnlyTargetRules Target) : base(Target)
	{
		// Vendored third-party C library (hidapi 0.15.0): no PCH, no unity, relaxed warnings.
		PCHUsage = PCHUsageMode.NoPCHs;
		bUseUnity = false;
		IWYUSupport = IWYUSupport.None;
		bTreatAsEngineModule = true;
		bWarningsAsErrors = false;

		PrivateDependencyModuleNames.Add("Core");

		PublicIncludePaths.Add(Path.Combine(ModuleDirectory, "Public"));
		PrivateIncludePaths.AddRange(new string[]
		{
			Path.Combine(ModuleDirectory, "Private", "windows"),
			Path.Combine(ModuleDirectory, "Private", "mac"),
			Path.Combine(ModuleDirectory, "Private", "linux"),
		});

		if (Target.Platform == UnrealTargetPlatform.Mac)
		{
			PublicFrameworks.AddRange(new string[] { "IOKit", "CoreFoundation", "AppKit" });
		}
		else if (Target.Platform == UnrealTargetPlatform.Linux)
		{
			// hidapi's hidraw backend resolves device info through libudev.
			PublicSystemLibraries.Add("udev");
		}
		// Windows: hidapi loads hid.dll / cfgmgr32.dll at runtime, so no import libraries are needed.

		if (Target.Platform != UnrealTargetPlatform.Win64)
		{
			// UE compiles modules with hidden symbol visibility; force default visibility so the
			// hid_* symbols are exported from this module's shared library on Mac/Linux.
			PrivateDefinitions.Add("HID_API_EXPORT=__attribute__((visibility(\"default\")))");
		}
	}
}
