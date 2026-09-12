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
#include "UObject/UnrealType.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(RageAudioSettings)

void URageAudioSettings::LoadSettings()
{
	Pending = CastChecked<URageAudioSettings>(RageSettings::CreateShadowInstance(this, this));
	Defaults = CastChecked<URageAudioSettings>(RageSettings::CreateDefaultsInstance(this, GetClass()));

	FCoreDelegates::ApplicationWillDeactivateDelegate.AddUObject(this, &URageAudioSettings::HandleApplicationDeactivated);
	FCoreDelegates::ApplicationHasReactivatedDelegate.AddUObject(this, &URageAudioSettings::HandleApplicationReactivated);

	LoadSoundAssetsAsync();
}

void URageAudioSettings::ApplySettings()
{
	RageSettings::CopyObjectProperties(this, Pending);

	ApplyVolumes();

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

void URageAudioSettings::LoadSoundAssetsAsync()
{
	if (SETTINGS->MasterSoundClass.IsNull() || SETTINGS->MasterSoundMix.IsNull())
	{
		S_LOG(Warning, "URageAudioSettings requires MasterSoundClass and MasterSoundMix to be set (in RageSettings)!");
		return;
	}

	/* A request is already in flight or done - HandleSoundAssetsLoaded() owns the result either way. */
	if (SoundAssetsLoadHandle.IsValid())
	{
		return;
	}

	TArray<FSoftObjectPath> Assets {SETTINGS->MasterSoundClass.ToSoftObjectPath(), SETTINGS->MasterSoundMix.ToSoftObjectPath()};
	
	for (const TPair<FName, TSoftObjectPtr<USoundClass>>& Entry : SETTINGS->VolumeSoundClasses)
	{
		if (!Entry.Value.IsNull())
		{
			Assets.AddUnique(Entry.Value.ToSoftObjectPath());
		}
	}

	FStreamableManager& StreamableManager = UAssetManager::Get().GetStreamableManager();
	FStreamableDelegate LoadDelegate = FStreamableDelegate::CreateUObject(this, &URageAudioSettings::HandleSoundAssetsLoaded);
	SoundAssetsLoadHandle = StreamableManager.RequestAsyncLoad(Assets, LoadDelegate);
}

void URageAudioSettings::HandleSoundAssetsLoaded()
{
	LoadedMasterSoundClass = SETTINGS->MasterSoundClass.Get();
	LoadedMasterSoundMix = SETTINGS->MasterSoundMix.Get();

	if (!IsValid(LoadedMasterSoundClass) || !IsValid(LoadedMasterSoundMix))
	{
		S_LOG(Error, "URageAudioSettings::HandleSoundAssetsLoaded the master sound mix/class finished loading but could not be resolved - volumes will not be applied.");
		return;
	}

	ResolveVolumeSoundClasses();
	ApplyVolumes();
}

void URageAudioSettings::ResolveVolumeSoundClasses()
{
	LoadedVolumeSoundClasses.Reset();

	for (const TPair<FName, TSoftObjectPtr<USoundClass>>& Entry : SETTINGS->VolumeSoundClasses)
	{
		USoundClass* SoundClass = Entry.Value.LoadSynchronous();

		if (!IsValid(SoundClass))
		{
			S_LOG(Warning, "URageAudioSettings VolumeSoundClasses[{property}] names no sound class, so that volume will not be heard.", *Entry.Key.ToString());
			continue;
		}

		if (!FindVolumeProperty(Entry.Key))
		{
			S_LOG(Warning, "URageAudioSettings VolumeSoundClasses[{property}] is not a float on {settings_class}, so {sound_class} keeps the volume it was authored with.",
				*Entry.Key.ToString(), *GetClass()->GetName(), *SoundClass->GetName());
			continue;
		}

		LoadedVolumeSoundClasses.Add(Entry.Key, SoundClass);
	}
}

bool URageAudioSettings::EnsureSoundAssetsLoaded()
{
	if (IsValid(LoadedMasterSoundClass) && IsValid(LoadedMasterSoundMix))
	{
		return true;
	}

	if (SETTINGS->MasterSoundClass.IsNull() || SETTINGS->MasterSoundMix.IsNull())
	{
		return false;
	}
	
	S_LOG(Warning, "URageAudioSettings::EnsureSoundAssetsLoaded the master sound mix/class was still loading, falling back to a synchronous load.");
	LoadedMasterSoundClass = SETTINGS->MasterSoundClass.LoadSynchronous();
	LoadedMasterSoundMix = SETTINGS->MasterSoundMix.LoadSynchronous();

	if (!IsValid(LoadedMasterSoundClass) || !IsValid(LoadedMasterSoundMix))
	{
		S_LOG(Error, "URageAudioSettings::EnsureSoundAssetsLoaded failed to load the master sound mix/class - volumes will not be applied.");
		return false;
	}

	ResolveVolumeSoundClasses();
	return true;
}

void URageAudioSettings::ApplyVolumes()
{
	if (!EnsureSoundAssetsLoaded())
	{
		return;
	}

	FAudioDevice* AudioDevice = ResolveAudioDevice();
	if (!AudioDevice)
	{
		S_LOG(Verbose, "URageAudioSettings::ApplyVolumes no audio device available yet - volumes will apply on the next apply.");
		return;
	}

	ActivateMasterSoundMix(AudioDevice);

	ApplyMasterVolume(AudioDevice);
	ApplyClassVolumes(AudioDevice);
}

void URageAudioSettings::ApplyMasterVolume(FAudioDevice* AudioDevice)
{
	ApplyVolumeToClass(AudioDevice, LoadedMasterSoundClass, MasterVolume);
}

void URageAudioSettings::ApplyClassVolumes(FAudioDevice* AudioDevice)
{
	for (const TPair<FName, TObjectPtr<USoundClass>>& Entry : LoadedVolumeSoundClasses)
	{
		ApplyVolumeToClass(AudioDevice, Entry.Value, GetVolumeForProperty(Entry.Key));
	}
}

void URageAudioSettings::ApplyVolumeToClass(FAudioDevice* AudioDevice, USoundClass* SoundClass, float Volume) const
{
	if (!IsValid(SoundClass))
	{
		return;
	}
	
	AudioDevice->SetSoundMixClassOverride(LoadedMasterSoundMix, SoundClass, FMath::Clamp(Volume, 0.f, 1.f), 1.f, 0.f, true);
}

void URageAudioSettings::ActivateMasterSoundMix(FAudioDevice* AudioDevice)
{
	if (MasterSoundMixDeviceId.IsSet() && MasterSoundMixDeviceId.GetValue() == AudioDevice->DeviceID)
	{
		return;
	}

	AudioDevice->PushSoundMixModifier(LoadedMasterSoundMix);
	MasterSoundMixDeviceId = AudioDevice->DeviceID;
}

const FProperty* URageAudioSettings::FindVolumeProperty(FName PropertyName) const
{
	const FProperty* Property = GetClass()->FindPropertyByName(PropertyName);
	const bool bIsFloatingPoint = Property && (Property->IsA<FFloatProperty>() || Property->IsA<FDoubleProperty>());

	return bIsFloatingPoint ? Property : nullptr;
}

float URageAudioSettings::GetVolumeForProperty(FName PropertyName) const
{
	const FProperty* Property = FindVolumeProperty(PropertyName);

	if (const FFloatProperty* FloatProperty = CastField<FFloatProperty>(Property))
	{
		return FloatProperty->GetPropertyValue_InContainer(this);
	}

	if (const FDoubleProperty* DoubleProperty = CastField<FDoubleProperty>(Property))
	{
		return static_cast<float>(DoubleProperty->GetPropertyValue_InContainer(this));
	}
	
	return 1.f;
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
