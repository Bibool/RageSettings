// Copyright (c) 2026 Abdallah Boutrif
// SPDX-License-Identifier: MIT

#pragma once

#include "CoreMinimal.h"
#include "Containers/Ticker.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "RageSettingsCategoryInterface.h"
#include "RageSettingsSubsystem.generated.h"

class URageVideoSettings;
class URageAudioSettings;
class URageGameSettings;
class URageInputSettings;

/**
 * Subsystem interfacing between the Rage Game Settings and UI.
 * Use this over direct-query of the Game Settings object.
 */
UCLASS()
class RAGESETTINGS_API URageSettingsSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
	
	static URageSettingsSubsystem* Get(const UObject* WorldContextObject);
	
	template <typename T = URageVideoSettings>
	T* GetTypedVideoSettings() const { return Cast<T>(GetVideoSettings()); }
	
	template <typename T = URageAudioSettings>
	T* GetTypedAudioSettings() const { return Cast<T>(GetAudioSettings()); }

	template <typename T = URageGameSettings>
	T* GetTypedGameSettings() const { return Cast<T>(GetGameSettings()); }

	template <typename T = URageInputSettings>
	T* GetTypedInputSettings() const { return Cast<T>(GetInputSettings()); }

	UFUNCTION(BlueprintPure, Category = "Rage|Settings") 
	URageVideoSettings* GetVideoSettings() const;
	
	UFUNCTION(BlueprintPure, Category = "Rage|Settings") 
	URageAudioSettings* GetAudioSettings() const;
	
	UFUNCTION(BlueprintPure, Category = "Rage|Settings") 
	URageGameSettings* GetGameSettings()  const;
	
	UFUNCTION(BlueprintPure, Category = "Rage|Settings") 
	URageInputSettings* GetInputSettings() const;	

	UFUNCTION(BlueprintCallable, Category = "Rage|Settings")
	void ApplyAllDirtySettings();

	UFUNCTION(BlueprintCallable, Category = "Rage|Settings")
	void SaveAllSettings();

	UFUNCTION(BlueprintCallable, Category = "Rage|Settings")
	void ApplyAndSaveAllDirtySettings();

	UFUNCTION(BlueprintCallable, Category = "Rage|Settings")
	void RevertAllPendingChanges();
	
	UFUNCTION(BlueprintCallable, Category = "Rage|Settings")
	void ResetCategoryToDefault(ERageSettingsCategory Category, bool bApplyImmediately = true);

	UFUNCTION(BlueprintCallable, Category = "Rage|Settings")
	void ResetAllCategoriesToDefault(bool bApplyImmediately = true);

	UFUNCTION(BlueprintPure, Category = "Rage|Settings")
	bool HasAnyDirtySettings() const;

	UFUNCTION(BlueprintPure, Category = "Rage|Settings")
	bool IsCategoryDirty(ERageSettingsCategory Category) const;
	
	UFUNCTION(BlueprintPure, Category = "Rage|Settings")
	bool IsRestartRequired() const;
	
	UFUNCTION(BlueprintCallable, Category = "Rage|Settings")
	bool RestartGame();
	
	UPROPERTY(BlueprintAssignable, Category = "Rage|Delegates")
	FRageCategoryDirtyStateChanged AnyCategoryDirtyStateChangedDelegate;

	UPROPERTY(BlueprintAssignable, Category = "Rage|Delegates")
	FRageCategoryApplied CategoryAppliedDelegate;
	
	UPROPERTY(BlueprintAssignable, Category = "Rage|Delegates")
	FRageRestartRequirementEvaluated RestartRequirementEvaluatedDelegate;

private:
	UFUNCTION()
	void HandleCategoryDirtyStateChanged(ERageSettingsCategory Category, bool bIsDirty);
	
	void DeferredEvaluateRestartRequirement();

	IRageSettingsCategoryInterface* ResolveCategory(ERageSettingsCategory Category) const;

	FTSTicker::FDelegateHandle RestartCheckTickerHandle;
	
	UPROPERTY() 
	TObjectPtr<URageVideoSettings> VideoSettings = nullptr;
	
	UPROPERTY() 
	TObjectPtr<URageAudioSettings> AudioSettings = nullptr;
	
	UPROPERTY() 
	TObjectPtr<URageGameSettings>  GameSettings = nullptr;
	
	UPROPERTY() 
	TObjectPtr<URageInputSettings> InputSettings = nullptr;
	
	TArray<TScriptInterface<IRageSettingsCategoryInterface>> AllCategories;
	
};
