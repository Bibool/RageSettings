// Copyright (c) 2026 Abdallah Boutrif
// SPDX-License-Identifier: MIT

#pragma once

#include "CoreMinimal.h"
#include "RageSettingsCategoryInterface.h"
#include "RageGameSettings.generated.h"

UCLASS(Config = RageGameSettings)
class RAGESETTINGS_API URageGameSettings : public UObject, public IRageSettingsCategoryInterface
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
	virtual ERageSettingsCategory GetCategoryId() const override { return ERageSettingsCategory::Game; }
//~ End IRageSettingsCategory

	UFUNCTION(BlueprintPure, Category = "Rage|Game")
	URageGameSettings* GetPendingSettings() const { return Pending; }

	UFUNCTION(BlueprintPure, Category = "Rage|Game")
	URageGameSettings* GetDefaultSettings() const { return Defaults; }

	UPROPERTY(BlueprintAssignable, Category = "Rage|Delegates")
	FRageCategoryDirtyStateChanged DirtyStateChangedDelegate;

protected:
	UPROPERTY()
	TObjectPtr<URageGameSettings> Pending = nullptr;

	UPROPERTY()
	TObjectPtr<URageGameSettings> Defaults = nullptr;

private:
	void BroadcastDirtyIfChanged(bool bWasDirtyBefore);
};
