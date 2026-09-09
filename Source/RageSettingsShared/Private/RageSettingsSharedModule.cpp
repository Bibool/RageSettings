// Copyright (c) 2026 Abdallah Boutrif
// SPDX-License-Identifier: MIT

#include "Modules/ModuleManager.h"

#include "RageFrameGenerationProvider.h"

/** Module loads at PostConfigInit to manage swap chain gate before vendor RHI 
 * modules register their providers at PostSplashScreen */
class FRageSettingsSharedModule final : public IModuleInterface
{
public:
	virtual void StartupModule() override
	{
		RageFrameGeneration::ApplyStartupGate();
	}

	virtual void ShutdownModule() override
	{
		RageFrameGeneration::ReleaseStartupGate();
	}
};

IMPLEMENT_MODULE(FRageSettingsSharedModule, RageSettingsShared);
