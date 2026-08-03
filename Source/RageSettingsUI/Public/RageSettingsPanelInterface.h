// Copyright (c) 2026 Abdallah Boutrif
// SPDX-License-Identifier: MIT

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "RageSettingsCategory.h"
#include "RageSettingsPanelInterface.generated.h"

class URageSettingsSubsystem;

UINTERFACE(MinimalAPI, Blueprintable)
class URageSettingsPanelInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * Implemented by every category panel (Game/Audio/Video/Input) so URageSettingsView can drive
 * all of them the same way - the UI-layer mirror of the backend's IRageSettingsCategory.
 */
class RAGESETTINGSUI_API IRageSettingsPanelInterface
{
	GENERATED_BODY()

public:
	/** Called once, right after the subsystem becomes available, before any RefreshFromSettings()
	 *  call - lets the panel cache whichever concrete settings object it needs and wire up its
	 *  row widgets' delegates. */
	virtual void InitializePanel(URageSettingsSubsystem* InSubsystem) = 0;

	/** Pulls every value from the category's Pending state into this panel's widgets, without
	 *  firing their OnValueChanged delegates. Cheap - safe to call every time this panel's tab
	 *  becomes active, which is what guarantees the UI is correct even after a Reset/Revert
	 *  that happened while the tab wasn't visible. */
	virtual void RefreshFromSettings() = 0;

	virtual ERageSettingsCategory GetCategoryId() const = 0;
};
