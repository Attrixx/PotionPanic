// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class PotionPanic : ModuleRules
{
	public PotionPanic(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[]
		{
			"Core"
		});

		PrivateDependencyModuleNames.AddRange(new string[] { });
	}
}