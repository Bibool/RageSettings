// Copyright (c) 2026 Abdallah Boutrif
// SPDX-License-Identifier: MIT

#include "RageGameSettings.h"

#include "RageSettingsReflectionUtils.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(RageGameSettings)

void URageGameSettings::LoadSettings()
{
	// Current was already populated from GameUserSettings.ini by the ordinary UObject config
	// system, since its fields are declared UPROPERTY(config) - no manual LoadConfig() call needed here.
	Pending = CastChecked<URageGameSettings>(RageSettings::CreateShadowInstance(this, this));
	Defaults = CastChecked<URageGameSettings>(RageSettings::CreateShadowInstance(this, GetClass()->GetDefaultObject()));
}

void URageGameSettings::ApplySettings()
{
	RageSettings::CopyObjectProperties(this, Pending);

	/* Apply stuff here. */

	DirtyStateChangedDelegate.Broadcast(ERageSettingsCategory::Game, false);
}

void URageGameSettings::SaveSettings()
{
	SaveConfig();
}

void URageGameSettings::ResetToDefault()
{
	const bool bWasDirty = IsDirty();
	RageSettings::CopyObjectProperties(Pending, Defaults);
	BroadcastDirtyIfChanged(bWasDirty);
}

void URageGameSettings::RevertPendingChanges()
{
	const bool bWasDirty = IsDirty();
	RageSettings::CopyObjectProperties(Pending, this);
	BroadcastDirtyIfChanged(bWasDirty);
}

bool URageGameSettings::IsDirty() const
{
	return !RageSettings::AreObjectsEqual(this, Pending);
}

void URageGameSettings::BroadcastDirtyIfChanged(bool bWasDirtyBefore)
{
	const bool bIsDirtyNow = IsDirty();
	if (bWasDirtyBefore != bIsDirtyNow)
	{
		DirtyStateChangedDelegate.Broadcast(ERageSettingsCategory::Game, bIsDirtyNow);
	}
}
