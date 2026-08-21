// Copyright (c) 2026 Abdallah Boutrif
// SPDX-License-Identifier: MIT

#pragma once

#include "CoreMinimal.h"
#include "RageSettingsCategory.h"

/** Carries the category alongside the property name so one handler can serve several categories
 * without a lambda capturing which of them it was bound to. */
DECLARE_MULTICAST_DELEGATE_TwoParams(FRageSettingChangedNative, ERageSettingsCategory, FName);

/**
 * Where a category's per-property listeners live.
 * 
 * C++ only. Blueprint listens through URageSettingsSubsystem::SettingChangedDelegate and filters by name.
 *
 * Listeners fire once the category's apply has actually settled, which for video can be a later
 * tick than the one the player pressed Apply on. What they will not wait for is
 * ApplyFinishedAdditionalHoldSeconds, or a slower category applying alongside this one.
 */
struct RAGESETTINGS_API FRageSettingChangeRegistry
{
	/** Listeners for one property, by its C++ name. Reach for GET_MEMBER_NAME_CHECKED over a string
	 * literal: a literal goes on binding to nothing the day the field is renamed. */
	FRageSettingChangedNative& OnPropertyChanged(FName PropertyName) { return PerProperty.FindOrAdd(PropertyName); }

	/** Listeners for every property of this category, called once per property that changed. */
	FRageSettingChangedNative& OnAnyPropertyChanged() { return AnyProperty; }

	void Broadcast(ERageSettingsCategory Category, const TArray<FName>& ChangedProperties);

private:
	TMap<FName, FRageSettingChangedNative> PerProperty;

	FRageSettingChangedNative AnyProperty;
};
