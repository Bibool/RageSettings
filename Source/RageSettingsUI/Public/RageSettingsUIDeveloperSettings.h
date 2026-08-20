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

	/** String tables a generated row searches for its label and its enum option names, after the one
	 * this plugin ships. A project points this at its own table so the fields and enums it adds by
	 * subclassing can be translated without anyone editing an asset inside the plugin.
	 *
	 * Labels are keyed by the property name ("Culture"), options by either the enum entry name
	 * ("French") or that name scoped to its enum ("ERageCultures_French"), which is what to reach for
	 * when two enums share an entry name. */
	UPROPERTY(Config, EditAnywhere, Category = "Rage|UI")
	TArray<FName> AdditionalStringTables;
	
	/* Which Scalability settings to generate. */
	UPROPERTY(Config, EditAnywhere, EditFixedSize, Category = "Rage|Video")
	TMap<ERageScalabilityCategory, bool> ScalabilityOverrides;
	
	/** The visual representation of the keybind categories
	 * @see URageSettingsDeveloperSettings::RemappableActions Category inner struct property. */
	UPROPERTY(Config, EditAnywhere, Category = "Rage|Input")
	TMap<FGameplayTag, FRageKeybindCategoryConfig> KeybindCategoriesConfig;
};
