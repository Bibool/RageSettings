// Copyright (c) 2026 Abdallah Boutrif
// SPDX-License-Identifier: MIT

#pragma once

#include "CoreMinimal.h"
#include "RageSettingsRowGeneratorPanelBase.h"
#include "RageAudioSettingsPanel.generated.h"

class URageAudioSettings;

UCLASS(Abstract, meta=(DisableNativeTick))
class RAGESETTINGSUI_API URageAudioSettingsPanel : public URageSettingsRowGeneratorPanelBase
{
	GENERATED_BODY()

public:
//~ Begin IRageSettingsPanel
	virtual void InitializePanel(URageSettingsSubsystem* InSubsystem) override;
	virtual ERageSettingsCategory GetCategoryId() const override { return ERageSettingsCategory::Audio; }
//~ End IRageSettingsPanel

protected:
	virtual TArray<FRageSettingsRowDescriptor> GetRowDescriptors() const override;

private:
	UPROPERTY()
	TObjectPtr<URageAudioSettings> AudioSettings = nullptr;
};
