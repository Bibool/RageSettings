// Copyright (c) 2026 Abdallah Boutrif
// SPDX-License-Identifier: MIT

#pragma once

#include "CoreMinimal.h"
#include "InputCoreTypes.h"
#include "RageSettingsCategoryInterface.h"
#include "RageInputSettings.generated.h"

class UEnhancedInputUserSettings;
class UGameInstance;
class ULocalPlayer;

/**
 * Input settings only support local player (index 0) and thus only single screen games.
 * It also doesn't support multiple player profiles, nor does it support secondary keybinds. */
UCLASS(Config = RageInputSettings)
class RAGESETTINGS_API URageInputSettings : public UObject, public IRageSettingsCategoryInterface
{
	GENERATED_BODY()

public:
	void InitializeWithGameInstance(UGameInstance* InGameInstance);

//~ Begin IRageSettingsCategory
	virtual void LoadSettings() override;
	virtual void ApplySettings() override;
	virtual void SaveSettings() override;
	virtual void ResetToDefault() override;
	virtual void RevertPendingChanges() override;
	virtual bool IsDirty() const override;
	virtual void NotifyPendingChangedExternally(bool bWasDirtyBefore) override { BroadcastDirtyIfChanged(bWasDirtyBefore); }
	virtual ERageSettingsCategory GetCategoryId() const override { return ERageSettingsCategory::Input; }
//~ End IRageSettingsCategory

	UFUNCTION(BlueprintCallable, Category = "Rage|Input")
	void SetPendingMouseSensitivity(float X, float Y);

	UFUNCTION(BlueprintCallable, Category = "Rage|Input")
	void SetPendingInvertMouseY(bool bInvert);
	
	UFUNCTION(BlueprintCallable, Category = "Rage|Input")
	void SetPendingGamepadSensitivity(float X, float Y);

	UFUNCTION(BlueprintCallable, Category = "Rage|Input")
	void SetPendingInvertGamepadY(bool bInvert);

	UFUNCTION(BlueprintPure, Category = "Rage|Input")
	URageInputSettings* GetPendingSettings() const { return Pending; }

	UFUNCTION(BlueprintPure, Category = "Rage|Input")
	URageInputSettings* GetDefaultSettings() const { return Defaults; }

	UFUNCTION(BlueprintPure, Category = "Rage|Input")
	float GetMouseSensitivityX() const;

	UFUNCTION(BlueprintPure, Category = "Rage|Input")
	float GetMouseSensitivityY() const;

	UFUNCTION(BlueprintCallable, Category = "Rage|Input|Keybinds")
	bool RemapPlayerKey(FName MappingName, FKey NewKey);

	UFUNCTION(BlueprintPure, Category = "Rage|Input|Keybinds")
	FKey GetCurrentKeyForMapping(FName MappingName) const;

	UFUNCTION(BlueprintPure, Category = "Rage|Input|Keybinds")
	TArray<FName> FindMappingsUsingKey(FKey Key, FName IgnoreMappingName) const;

	UFUNCTION(BlueprintCallable, Category = "Rage|Input|Keybinds")
	bool ClearKeyMapping(FName MappingName);

	UFUNCTION(BlueprintCallable, Category = "Rage|Input|Keybinds")
	void ResetKeyMappingToDefault(FName MappingName);

	UFUNCTION(BlueprintCallable, Category = "Rage|Input|Keybinds")
	void ResetAllKeyMappingsToDefault();

	UPROPERTY(BlueprintAssignable, Category = "Rage|Delegates")
	FRageCategoryDirtyStateChanged DirtyStateChangedDelegate;

	UPROPERTY(Config, EditAnywhere, BlueprintReadWrite, Category = "Rage|Input")
	float MouseSensitivityX = 1.f;

	UPROPERTY(Config, EditAnywhere, BlueprintReadWrite, Category = "Rage|Input")
	float MouseSensitivityY = 1.f;

	UPROPERTY(Config, EditAnywhere, BlueprintReadWrite, Category = "Rage|Input")
	bool bInvertMouseY = false;
	
	UPROPERTY(Config, EditAnywhere, BlueprintReadWrite, Category = "Rage|Input")
	float GamepadSensitivityX = 1.f;

	UPROPERTY(Config, EditAnywhere, BlueprintReadWrite, Category = "Rage|Input")
	float GamepadSensitivityY = 1.f;

	UPROPERTY(Config, EditAnywhere, BlueprintReadWrite, Category = "Rage|Input")
	bool bInvertGamepadY = false;

protected:
	UPROPERTY()
	TObjectPtr<URageInputSettings> Pending = nullptr;

	UPROPERTY()
	TObjectPtr<URageInputSettings> Defaults = nullptr;

	TWeakObjectPtr<UGameInstance> GameInstance = nullptr;

private:
	void BroadcastDirtyIfChanged(bool bWasDirtyBefore);
	UEnhancedInputUserSettings* GetEnhancedInputUserSettings() const;
};
