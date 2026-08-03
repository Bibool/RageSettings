// Copyright (c) 2026 Abdallah Boutrif
// SPDX-License-Identifier: MIT

#pragma once

#include "CoreMinimal.h"
#include "RageSettingsCategoryInterface.h"
#include "RageAudioSettings.generated.h"

struct FStreamableHandle;
class FAudioDevice;
class USoundMix;
class USoundClass;

/**
 * Player-profile audio settings. See RageGameSettings.h for the persistence/extension rationale
 * shared by every category - projects add their own fields by subclassing (see
 * RageSettingsDeveloperSettings::AudioSettingsClass).
 */
UCLASS(Config = RageAudioSettings)
class RAGESETTINGS_API URageAudioSettings : public UObject, public IRageSettingsCategoryInterface
{
	GENERATED_BODY()

public:
//~ Begin IRageSettingsCategory
	virtual void LoadSettings() override;
	virtual void ApplySettings() override;
	virtual void SaveSettings() override;
	virtual void ResetToDefault() override;
	virtual void RevertPendingChanges() override;
	virtual bool IsDirty() const override;
	virtual void NotifyPendingChangedExternally(bool bWasDirtyBefore) override { BroadcastDirtyIfChanged(bWasDirtyBefore); }
	virtual ERageSettingsCategory GetCategoryId() const override { return ERageSettingsCategory::Audio; }
//~ End IRageSettingsCategory

	UFUNCTION(BlueprintCallable, Category = "Rage|Audio")
	void SetPendingMasterVolume(float NewVolume);

	UFUNCTION(BlueprintCallable, Category = "Rage|Audio")
	void SetPendingMuteWhenUnfocused(bool bMute);

	UFUNCTION(BlueprintPure, Category = "Rage|Audio")
	URageAudioSettings* GetPendingSettings() const { return Pending; }

	UFUNCTION(BlueprintPure, Category = "Rage|Audio")
	URageAudioSettings* GetDefaultSettings() const { return Defaults; }

	UPROPERTY(BlueprintAssignable, Category = "Rage|Delegates")
	FRageCategoryDirtyStateChanged DirtyStateChangedDelegate;

	UPROPERTY(Config, EditAnywhere, BlueprintReadWrite, Category = "Rage|Audio", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float MasterVolume = 1.f;

	UPROPERTY(Config, EditAnywhere, BlueprintReadWrite, Category = "Rage|Audio")
	bool bMuteWhenUnfocused = true;

protected:
	UPROPERTY()
	TObjectPtr<URageAudioSettings> Pending = nullptr;

	UPROPERTY()
	TObjectPtr<URageAudioSettings> Defaults = nullptr;

private:
	void BroadcastDirtyIfChanged(bool bWasDirtyBefore);
	void HandleApplicationDeactivated();
	void HandleApplicationReactivated();
	void ApplyMuteState(bool bShouldMute);
	void LoadMasterClassesAsync();
	void HandleMasterClassesLoaded();
	void ApplyMasterVolume();

	FAudioDevice* ResolveAudioDevice() const;
	
	UPROPERTY(Transient)
	TObjectPtr<USoundClass> LoadedMasterSoundClass = nullptr;

	UPROPERTY(Transient)
	TObjectPtr<USoundMix> LoadedMasterSoundMix = nullptr;

	TSharedPtr<FStreamableHandle> MasterClassesLoadHandle;
};
