// Copyright (c) 2026 Abdallah Boutrif
// SPDX-License-Identifier: MIT

#include "RageGameSettingsPanel.h"

#include "RageGameSettings.h"
#include "RageSettingsSubsystem.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(RageGameSettingsPanel)

void URageGameSettingsPanel::InitializePanel(URageSettingsSubsystem* InSubsystem)
{
	GameSettings = InSubsystem->GetGameSettings();
	BuildRows(RowsContainer, GameSettings->GetPendingSettings());
}
