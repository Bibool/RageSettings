// Copyright (c) 2026 Abdallah Boutrif
// SPDX-License-Identifier: MIT

#pragma once

#include "GameplayTagContainer.h"
#include "RageKeybindCategoryConfig.h"
#include "RageRowOverrideData.h"
#include "Engine/DeveloperSettings.h"
#include "RageSettingsUIDeveloperSettings.generated.h"

class URageSelectionRow;
class URageRowBaseUserWidget;
enum class ERageScalabilityCategory : uint8;
class URageToggleRow;
class URageSliderRow;
class URageComboRow;
class UUserWidget;

#ifndef SETTINGS_UI
#define SETTINGS_UI URageSettingsUIDeveloperSettings::Get()
#endif

UCLASS(Config = Game, DefaultConfig, meta = (DisplayName = "Rage - Settings UI"))
class RAGESETTINGSUI_API URageSettingsUIDeveloperSettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	URageSettingsUIDeveloperSettings();
	
	UFUNCTION(BlueprintCallable, BlueprintPure, meta = (DisplayName = "Get Rage Settings UI"))
	static URageSettingsUIDeveloperSettings* Get();
	
	/* Should the enum settings use combo rows instead of selection row. */
	UPROPERTY(Config, EditAnywhere, Category = "Rage|UI|Defaults")
	bool bEnumSettingsUsesComboRow = false;
	
	UPROPERTY(Config, EditAnywhere, NoClear, Category = "Rage|UI|Defaults")
	TSubclassOf<URageToggleRow> DefaultToggleRowClass = nullptr;

	UPROPERTY(Config, EditAnywhere, NoClear, Category = "Rage|UI|Defaults")
	TSubclassOf<URageSliderRow> DefaultSliderRowClass = nullptr;

	UPROPERTY(Config, EditAnywhere, NoClear, Category = "Rage|UI|Defaults")
	TSubclassOf<URageComboRow> DefaultComboRowClass = nullptr;
	
	UPROPERTY(Config, EditAnywhere, NoClear, Category = "Rage|UI|Defaults")
	TSubclassOf<URageSelectionRow> DefaultSelectionRowClass = nullptr;
	
	UPROPERTY(Config, EditAnywhere, NoClear, Category = "Rage|UI|Defaults")
	TSubclassOf<URageRowBaseUserWidget> DefaultCategoryWidgetClass = nullptr;

	UPROPERTY(Config, EditAnywhere, Category = "Rage|UI")
	TMap<FName, FRageRowOverrideData> RowWidgetClassOverrides;
	
	/* Which Scalability settings to generate. */
	UPROPERTY(Config, EditAnywhere, EditFixedSize, Category = "Rage|Video")
	TMap<ERageScalabilityCategory, bool> ScalabilityOverrides;
	
	/** The visual representation of the keybind categories
	 * @see URageSettingsDeveloperSettings::RemappableActions Category inner struct property. */
	UPROPERTY(Config, EditAnywhere, Category = "Rage|Input")
	TMap<FGameplayTag, FRageKeybindCategoryConfig> KeybindCategoriesConfig;
};
