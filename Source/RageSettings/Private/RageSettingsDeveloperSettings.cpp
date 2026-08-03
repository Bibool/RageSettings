// Copyright (c) 2026 Abdallah Boutrif
// SPDX-License-Identifier: MIT

#include "RageSettingsDeveloperSettings.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(RageSettingsDeveloperSettings)

URageSettingsDeveloperSettings* URageSettingsDeveloperSettings::Get()
{
	return GetMutableDefault<URageSettingsDeveloperSettings>();
}

#if WITH_EDITOR

#include "RageKeybindConfig.h"
#include "RageSettingsSharedDebug.h"

void URageSettingsDeveloperSettings::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	if (!IsValid(GameSettingsClass))
	{
		GameSettingsClass = URageGameSettings::StaticClass();
	}

	if (!IsValid(AudioSettingsClass))
	{
		AudioSettingsClass = URageAudioSettings::StaticClass();
	}

	if (!IsValid(InputSettingsClass))
	{
		InputSettingsClass = URageInputSettings::StaticClass();
	}
	
	for (int i = 0; i < RemappableActions.Num(); i++)
	{
		const FRageKeybindConfig& RemappableAction = RemappableActions[i];
		
		if (!RemappableAction.MappingName.IsValid())
		{
			S_LOG(Error, "URageSettingsDeveloperSettings::RemappableActions index {num} has an invalid name. Make sure it is valid!", i);
		}
	}
}
#endif