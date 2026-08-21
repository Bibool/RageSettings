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

	/** True between ApplyStartedDelegate and ApplyFinishedDelegate, for UI created mid-apply. */
	UFUNCTION(BlueprintPure, Category = "Rage|Settings")
	bool IsApplyInProgress() const { return bApplyInProgress; }
	
	UFUNCTION(BlueprintCallable, Category = "Rage|Settings")
	bool RestartGame();
	
	UPROPERTY(BlueprintAssignable, Category = "Rage|Delegates")
	FRageCategoryDirtyStateChanged AnyCategoryDirtyStateChangedDelegate;

	UPROPERTY(BlueprintAssignable, Category = "Rage|Delegates")
	FRageCategoryApplied CategoryAppliedDelegate;
	
	UPROPERTY(BlueprintAssignable, Category = "Rage|Delegates")
	FRageRestartRequirementEvaluated RestartRequirementEvaluatedDelegate;

	/** Fires once when an apply begins, before any category is touched. */
	UPROPERTY(BlueprintAssignable, Category = "Rage|Delegates")
	FRageSettingsApplyStarted ApplyStartedDelegate;

	/** Fires once the last category has finished, including work deferred to a later tick. Always
	 *  follows ApplyStartedDelegate, in the same frame when nothing needed deferring. */
	UPROPERTY(BlueprintAssignable, Category = "Rage|Delegates")
	FRageSettingsApplyFinished ApplyFinishedDelegate;

	/** Fires once per property an apply actually changed, once that property's category has settled.
	 *  Blueprint's way in: C++ listeners that care about a single setting should bind through the
	 *  category instead, with IRageSettingsCategoryInterface::OnSettingChanged, and be spared waking
	 *  for every other property in the game. */
	UPROPERTY(BlueprintAssignable, Category = "Rage|Delegates")
	FRageSettingChanged SettingChangedDelegate;

private:
	UFUNCTION()
	void HandleCategoryDirtyStateChanged(ERageSettingsCategory Category, bool bIsDirty);

	void DeferredEvaluateRestartRequirement();

	void BeginApply();
	void EndApplyWhenSettled();
	void FinishApply();
	bool IsAnyCategoryApplying() const;

	/** Notes what the apply is about to change, applies it, and hands the list to
	 *  QueueChangeBroadcast. Every path that applies a category goes through here, which is what
	 *  keeps the per-property listeners from depending on which button the player pressed. */
	void ApplyCategoryTrackingChanges(IRageSettingsCategoryInterface* Category, bool bSaveAfterApply);

	void QueueChangeBroadcast(ERageSettingsCategory Category, TArray<FName>&& ChangedProperties);
	void FlushSettledChangeBroadcasts();
	void BroadcastCategoryChanges(ERageSettingsCategory Category, const TArray<FName>& ChangedProperties);

	IRageSettingsCategoryInterface* ResolveCategory(ERageSettingsCategory Category) const;

	/** A category's changed properties, waiting on that category alone. Held by id rather than by
	 *  pointer because the wait can span ticks. */
	struct FPendingChangeBroadcast
	{
		ERageSettingsCategory Category;
		TArray<FName> ChangedProperties;
	};

	TArray<FPendingChangeBroadcast> PendingChangeBroadcasts;

	bool bFlushingChangeBroadcasts = false;

	FTSTicker::FDelegateHandle RestartCheckTickerHandle;
	FTSTicker::FDelegateHandle ApplySettledTickerHandle;
	FTSTicker::FDelegateHandle ChangeBroadcastTickerHandle;

	bool bApplyInProgress = false;

	float ApplyHoldRemaining = 0.f;
	
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
