// Copyright (c) 2026 Abdallah Boutrif
// SPDX-License-Identifier: MIT

using System.IO;
using UnrealBuildTool;

public class RageSettings : ModuleRules
{
	private bool IsPluginPresent(string PluginName)
	{
		return Directory.Exists(Path.Combine(Path.GetDirectoryName(PluginDirectory), PluginName));
	}

	public RageSettings(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[]
		{
			"Core",
			"CoreUObject",
			"Engine",
			"InputCore",
			"EnhancedInput",
			"GameplayTags",
			"DeveloperSettings"
		});

		PrivateDependencyModuleNames.AddRange(new string[]
		{
			"RHI",
			"RageSettingsShared"
		});

		/* Although the plugin works with upscaler plugins, it should also not require them; hence we set up guards and macros. */

		/* WITH_DLSS comes from DLSSBlueprint. */
		if (IsPluginPresent("DLSS"))
		{
			PrivateDependencyModuleNames.AddRange(new string[]
			{
				"DLSS",
				"DLSSBlueprint"
			});
		}
		else
		{
			PublicDefinitions.Add("WITH_DLSS=0");
		}

		/* WITH_STREAMLINE comes from StreamlineBlueprint, which StreamlineDLSSGBlueprint re-exports. */
		if (IsPluginPresent("Streamline") && IsPluginPresent("StreamlineDLSSG"))
		{
			PrivateDependencyModuleNames.AddRange(new string[]
			{
				"Streamline",
				"StreamlineDLSSGBlueprint"
			});
		}
		else
		{
			PublicDefinitions.Add("WITH_STREAMLINE=0");
		}

		/* WITH_XESS / WITH_XEFG / WITH_XELL come from the matching XeSS Blueprint modules. */
		if (IsPluginPresent("XeSS"))
		{
			PrivateDependencyModuleNames.AddRange(new string[]
			{
				"XeSSCore",
				"XeSSBlueprint",
				"XeLLBlueprint",
				"XeFGBlueprint"
			});
		}
		else
		{
			PublicDefinitions.Add("WITH_XESS=0");
			PublicDefinitions.Add("WITH_XEFG=0");
			PublicDefinitions.Add("WITH_XELL=0");
		}

		/* The FSR plugin publishes no WITH_* define of its own, so this one is ours to set. Public so
		 * projects subclassing URageVideoSettings can guard their own code the same way. */
		if (IsPluginPresent("FSR"))
		{
			PrivateDependencyModuleNames.Add("FFXFSRSettings");
			PublicDefinitions.Add("WITH_FSR=1");
		}
		else
		{
			PublicDefinitions.Add("WITH_FSR=0");
		}
	}
}
