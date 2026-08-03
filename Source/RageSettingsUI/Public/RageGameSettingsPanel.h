// Copyright (c) 2026 Abdallah Boutrif
// SPDX-License-Identifier: MIT

#pragma once

#include "CoreMinimal.h"
#include "RageSettingsRowGeneratorPanelBase.h"
#include "RageGameSettingsPanel.generated.h"

class URageGameSettings;

/**
 * Game settings panel. All rows (base plugin fields and any project-subclass-added fields, e.g.
 * bAutoThruster) are generated at runtime by the inherited row generator - see
 * URageSettingsRowGeneratorPanelBase and RageSettingsDeveloperSettings::GameSettingsClass. This
 * panel needs no changes when a project adds its own Game settings fields.
 */
UCLASS(Abstract, meta=(DisableNativeTick))
class RAGESETTINGSUI_API URageGameSettingsPanel : public URageSettingsRowGeneratorPanelBase
{
	GENERATED_BODY()

public:
//~ Begin IRageSettingsPanel
	virtual void InitializePanel(URageSettingsSubsystem* InSubsystem) override;
	virtual ERageSettingsCategory GetCategoryId() const override { return ERageSettingsCategory::Game; }
//~ End IRageSettingsPanel

private:
	UPROPERTY()
	TObjectPtr<URageGameSettings> GameSettings = nullptr;
};
