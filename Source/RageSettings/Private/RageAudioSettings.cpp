// Copyright (c) 2026 Abdallah Boutrif
// SPDX-License-Identifier: MIT

#include "RageAudioSettings.h"

#include "Sound/SoundMix.h"
#include "Sound/SoundClass.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "AudioDevice.h"
#include "RageSettingsDeveloperSettings.h"
#include "RageSettingsReflectionUtils.h"
#include "RageSettingsSharedDebug.h"
#include "Engine/AssetManager.h"
#include "Engine/StreamableManager.h"
#include "Misc/CoreDelegates.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(RageAudioSettings)

void URageAudioSettings::LoadSettings()
{
	Pending = CastChecked<URageAudioSettings>(RageSettings::CreateShadowInstance(this, this));
	Defaults = CastChecked<URageAudioSettings>(RageSettings::CreateShadowInstance(this, GetClass()->GetDefaultObject()));

	FCoreDelegates::ApplicationWillDeactivateDelegate.AddUObject(this, &URageAudioSettings::HandleApplicationDeactivated);
	FCoreDelegates::ApplicationHasReactivatedDelegate.AddUObject(this, &URageAudioSettings::HandleApplicationReactivated);
	
	LoadMasterClassesAsync();
}

void URageAudioSettings::ApplySettings()
{
	RageSettings::CopyObjectProperties(this, Pending);

	ApplyMasterVolume();

	DirtyStateChangedDelegate.Broadcast(ERageSettingsCategory::Audio, false);
}

void URageAudioSettings::SaveSettings()
{
	SaveConfig();
}

void URageAudioSettings::ResetToDefault()
{
	const bool bWasDirty = IsDirty();
	RageSettings::CopyObjectProperties(Pending, Defaults);
	BroadcastDirtyIfChanged(bWasDirty);
}

void URageAudioSettings::RevertPendingChanges()
{
	const bool bWasDirty = IsDirty();
	RageSettings::CopyObjectProperties(Pending, this);
	BroadcastDirtyIfChanged(bWasDirty);
}

bool URageAudioSettings::IsDirty() const
{
	return !RageSettings::AreObjectsEqual(this, Pending);
}

void URageAudioSettings::SetPendingMasterVolume(float NewVolume)
{
	const bool bWasDirty = IsDirty();
	Pending->MasterVolume = FMath::Clamp(NewVolume, 0.f, 1.f);
	BroadcastDirtyIfChanged(bWasDirty);
}

void URageAudioSettings::SetPendingMuteWhenUnfocused(bool bMute)
{
	const bool bWasDirty = IsDirty();
	Pending->bMuteWhenUnfocused = bMute;
	BroadcastDirtyIfChanged(bWasDirty);
}

void URageAudioSettings::BroadcastDirtyIfChanged(bool bWasDirtyBefore)
{
	const bool bIsDirtyNow = IsDirty();
	if (bWasDirtyBefore != bIsDirtyNow)
	{
		DirtyStateChangedDelegate.Broadcast(ERageSettingsCategory::Audio, bIsDirtyNow);
	}
}

void URageAudioSettings::HandleApplicationDeactivated()
{
	if (bMuteWhenUnfocused)
	{
		ApplyMuteState(true);
	}
}

void URageAudioSettings::HandleApplicationReactivated()
{
	ApplyMuteState(false);
}

void URageAudioSettings::ApplyMuteState(bool bShouldMute)
{
	if (IsValid(GEngine))
	{
		if (FAudioDevice* AudioDevice = GEngine->GetMainAudioDeviceRaw())
		{
			AudioDevice->SetTransientPrimaryVolume(!bShouldMute);
		}
	}
}

void URageAudioSettings::LoadMasterClassesAsync()
{
	if (SETTINGS->MasterSoundClass.IsNull() || SETTINGS->MasterSoundMix.IsNull())
	{
		S_LOG(Warning, "URageAudioSettings requires MasterSoundClass and MasterSoundMix to be set (in RageSettings)!");
		return;
	}

	/* A request is already in flight or done - HandleMasterClassesLoaded() owns the result either way. */
	if (MasterClassesLoadHandle.IsValid())
	{
		return;
	}

	FStreamableManager& StreamableManager = UAssetManager::Get().GetStreamableManager();
	FStreamableDelegate LoadDelegate = FStreamableDelegate::CreateUObject(this, &URageAudioSettings::HandleMasterClassesLoaded);
	TArray<FSoftObjectPath> Assets {SETTINGS->MasterSoundClass.ToSoftObjectPath(), SETTINGS->MasterSoundMix.ToSoftObjectPath()};
	MasterClassesLoadHandle = StreamableManager.RequestAsyncLoad(Assets, LoadDelegate);
}

void URageAudioSettings::HandleMasterClassesLoaded()
{
	LoadedMasterSoundClass = SETTINGS->MasterSoundClass.Get();
	LoadedMasterSoundMix = SETTINGS->MasterSoundMix.Get();

	if (!IsValid(LoadedMasterSoundClass) || !IsValid(LoadedMasterSoundMix))
	{
		S_LOG(Error, "URageAudioSettings::HandleMasterClassesLoaded the master sound mix/class finished loading but could not be resolved - master volume will not be applied.");
		return;
	}
	
	ApplyMasterVolume();
}

void URageAudioSettings::ApplyMasterVolume()
{
	if (!IsValid(LoadedMasterSoundClass) || !IsValid(LoadedMasterSoundMix))
	{
		if (SETTINGS->MasterSoundClass.IsNull() || SETTINGS->MasterSoundMix.IsNull())
		{
			/* Nothing configured to push through - LoadMasterClassesAsync() already warned once. */
			return;
		}

		/* The async load hasn't landed yet and someone is waiting on this apply, so block for it rather than leave the change inaudible. */
		S_LOG(Warning, "URageAudioSettings::ApplyMasterVolume the master sound mix/class was still loading, falling back to a synchronous load.");
		LoadedMasterSoundClass = SETTINGS->MasterSoundClass.LoadSynchronous();
		LoadedMasterSoundMix = SETTINGS->MasterSoundMix.LoadSynchronous();

		if (!IsValid(LoadedMasterSoundClass) || !IsValid(LoadedMasterSoundMix))
		{
			S_LOG(Error, "URageAudioSettings::ApplyMasterVolume failed to load the master sound mix/class - master volume will not be applied.");
			return;
		}
	}

	FAudioDevice* AudioDevice = ResolveAudioDevice();
	if (!AudioDevice)
	{
		S_LOG(Verbose, "URageAudioSettings::ApplyMasterVolume no audio device available yet - master volume will apply on the next apply.");
		return;
	}
	
	AudioDevice->SetSoundMixClassOverride(LoadedMasterSoundMix, LoadedMasterSoundClass, MasterVolume, 1.f, 0.f, true);
	AudioDevice->PushSoundMixModifier(LoadedMasterSoundMix);
}

FAudioDevice* URageAudioSettings::ResolveAudioDevice() const
{
	if (const UWorld* World = GetWorld())
	{
		if (FAudioDevice* WorldAudioDevice = World->GetAudioDeviceRaw())
		{
			return WorldAudioDevice;
		}
	}

	return IsValid(GEngine) ? GEngine->GetMainAudioDeviceRaw() : nullptr;
}
