// Copyright (c) 2026 Abdallah Boutrif
// SPDX-License-Identifier: MIT

#include "RageAudioSettingsPanel.h"

#include "RageAudioSettings.h"
#include "RageSettingsDeveloperSettings.h"
#include "RageSettingsSubsystem.h"
#include "RageSliderDisplayFormat.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(RageAudioSettingsPanel)

namespace
{
	FRageSettingsRowDescriptor MakeVolumeDescriptor(FName PropertyName)
	{
		FRageSettingsRowDescriptor Descriptor;
		Descriptor.PropertyName = PropertyName;
		Descriptor.ClampMin = 0.f;
		Descriptor.ClampMax = 1.f;
		Descriptor.SliderFormat = ERageSliderDisplayFormat::Percent;

		return Descriptor;
	}
}

void URageAudioSettingsPanel::InitializePanel(URageSettingsSubsystem* InSubsystem)
{
	AudioSettings = InSubsystem->GetAudioSettings();
	BuildRows(RowsContainer, AudioSettings->GetPendingSettings());
}

TArray<FRageSettingsRowDescriptor> URageAudioSettingsPanel::GetRowDescriptors() const
{
	TArray<FRageSettingsRowDescriptor> Descriptors;
	Descriptors.Add(MakeVolumeDescriptor(GET_MEMBER_NAME_CHECKED(URageAudioSettings, MasterVolume)));
	
	for (const TPair<FName, TSoftObjectPtr<USoundClass>>& Entry : SETTINGS->VolumeSoundClasses)
	{
		Descriptors.Add(MakeVolumeDescriptor(Entry.Key));
	}

	return Descriptors;
}
