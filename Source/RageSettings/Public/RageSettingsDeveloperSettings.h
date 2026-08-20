// Copyright (c) 2026 Abdallah Boutrif
// SPDX-License-Identifier: MIT

#pragma once

#include "RageAudioSettings.h"
#include "RageGameSettings.h"
#include "RageInputSettings.h"
#include "Engine/DeveloperSettings.h"
#include "RageSettingsDeveloperSettings.generated.h"

#ifndef SETTINGS
#define SETTINGS URageSettingsDeveloperSettings::Get()
#endif

struct FRageKeybindConfig;
class UInputMappingContext;

UCLASS(Config = Game, DefaultConfig, meta=(DisplayName = "Rage - Settings"))
class RAGESETTINGS_API URageSettingsDeveloperSettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, BlueprintPure, meta=(DisplayName = "Get Rage Settings"))
	static URageSettingsDeveloperSettings* Get();
	
#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif
	
	UPROPERTY(Config, EditAnywhere, NoClear, Category = "Rage|Overrides", meta = (DisplayName = "Game Settings Class"))
	TSubclassOf<URageGameSettings> GameSettingsClass = URageGameSettings::StaticClass();

	UPROPERTY(Config, EditAnywhere, NoClear, Category = "Rage|Overrides", meta = (DisplayName = "Audio Settings Class"))
	TSubclassOf<URageAudioSettings> AudioSettingsClass = URageAudioSettings::StaticClass();

	UPROPERTY(Config, EditAnywhere, NoClear, Category = "Rage|Overrides", meta = (DisplayName = "Input Settings Class"))
	TSubclassOf<URageInputSettings> InputSettingsClass = URageInputSettings::StaticClass();

	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Rage|Audio")
	TSoftObjectPtr<USoundMix> MasterSoundMix = nullptr;

	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Rage|Audio")
	TSoftObjectPtr<USoundClass> MasterSoundClass = nullptr;
	
	UPROPERTY(Config, BlueprintReadOnly, Category = "Rage|Video")
	float FieldOfViewMin = 60.f;
	
	UPROPERTY(Config, BlueprintReadOnly, Category = "Rage|Video")
	float FieldOfViewMax = 120.f;
	
	UPROPERTY(Config, BlueprintReadOnly, Category = "Rage|Video")
	float BrightnessMin = 1.7f;

	UPROPERTY(Config, BlueprintReadOnly, Category = "Rage|Video")
	float BrightnessMax = 2.7f;
	
	UPROPERTY(Config, BlueprintReadOnly, Category = "Rage|Video")
	float FpsMax = 240.f;

	/** Extra seconds to keep the applying state up after the work has actually finished, so a fast apply
	 *  still reads as something having happened rather than a one-frame flash. Added on top of however
	 *  long the apply took, not a minimum duration. Zero fires the finished event as soon as the last
	 *  category settles. */
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Rage|Settings", meta = (ClampMin = "0.0", UIMax = "3.0", ForceUnits = "s"))
	float ApplyFinishedAdditionalHoldSeconds = 0.f;
	
	UPROPERTY(Config, BlueprintReadOnly, Category = "Rage|Input")
	float SensitivityMin = 0.1f;

	UPROPERTY(Config, BlueprintReadOnly, Category = "Rage|Input")
	float SensitivityMax = 5.0f;
	
	UPROPERTY(Config, EditAnywhere, Category = "Rage|Input")
	TArray<FRageKeybindConfig> RemappableActions;
	
	UPROPERTY(Config, EditAnywhere, Category = "Rage|Input")
	TArray<TSoftObjectPtr<UInputMappingContext>> RemappableContexts;

	UPROPERTY(Config, EditAnywhere, Category = "Rage|Input")
	bool bRebindConflictSwapsKey = true;

};
