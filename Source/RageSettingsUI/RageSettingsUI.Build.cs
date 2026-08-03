// Copyright (c) 2026 Abdallah Boutrif
// SPDX-License-Identifier: MIT

using UnrealBuildTool;

public class RageSettingsUI : ModuleRules
{
	public RageSettingsUI(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[]
		{
			"Core",
			"CoreUObject",
			"Engine",
			"InputCore",
			"UMG",
			"Slate",
			"SlateCore",
			"DeveloperSettings",
			"RageSettings",
			"RageSettingsShared",
			"GameplayTags"
		});
	}
}
