// Copyright (c) 2026 Abdallah Boutrif
// SPDX-License-Identifier: MIT

#include "RageAudioSettingsPanel.h"

#include "RageAudioSettings.h"
#include "RageMacros.h"
#include "RageSettingsSubsystem.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(RageAudioSettingsPanel)

void URageAudioSettingsPanel::InitializePanel(URageSettingsSubsystem* InSubsystem)
{
	AudioSettings = InSubsystem->GetAudioSettings();
	BuildRows(RowsContainer, AudioSettings->GetPendingSettings());
}

TArray<FRageSettingsRowDescriptor> URageAudioSettingsPanel::GetRowDescriptors() const
{
	FRageSettingsRowDescriptor MasterVolume;
	MasterVolume.PropertyName = GET_MEMBER_NAME_CHECKED(URageAudioSettings, MasterVolume);
	MasterVolume.Label = RAGE_LOC("MasterVolume");
	/* We don't get from DeveloperSettings Min/Max (nor are they present there) since it's a normalized field. */
	MasterVolume.ClampMin = 0.f;
	MasterVolume.ClampMax = 1.f;
	MasterVolume.SliderFormat = ERageSliderDisplayFormat::Percent;

	return { MasterVolume };
}
