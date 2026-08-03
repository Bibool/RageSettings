// Copyright (c) 2026 Abdallah Boutrif
// SPDX-License-Identifier: MIT

using UnrealBuildTool;

public class RageSettingsShared : ModuleRules
{
	public RageSettingsShared(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[]
		{
			"Core"
		});
	}
}
