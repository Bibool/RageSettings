// Copyright (c) 2026 Abdallah Boutrif
// SPDX-License-Identifier: MIT

#pragma once

#include "RageSettingChangeRegistry.h"
#include "RageSettingsCategory.h"
#include "UObject/Interface.h"
#include "RageSettingsCategoryInterface.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FRageCategoryDirtyStateChanged, ERageSettingsCategory, Category, bool, bDirty);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FRageCategoryApplied, ERageSettingsCategory, Category);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FRageSettingChanged, ERageSettingsCategory, Category, FName, PropertyName);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FRageSettingsApplyStarted);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FRageSettingsApplyFinished);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FRageRestartRequirementEvaluated, bool, bRestartRequired);

UINTERFACE(MinimalAPI, Blueprintable)
class URageSettingsCategoryInterface : public UInterface
{
	GENERATED_BODY()
};

/** Pure Abstract Interface which functionality MUST be implemented by each derived settings class.
 * The lifecycle:
 *
 *   LoadSettings()          Populate Current (from disk) and Pending (= Current). Called once
 *                           per category, at startup.
 *   ...UI edits Pending via the category's own Set*() functions...
 *   IsDirty()               True whenever Pending differs from Current.
 *   ApplySettings()         Push Pending -> Current and perform the real engine-side effect
 *                           (resize the window, push a sound-mix volume, flip a cvar, ...).
 *   SaveSettings()          Persist Current to disk.
 *   ResetToDefault()        Stage Pending = Defaults. Does NOT implicitly apply or save - the
 *                           caller decides whether a "Reset" button takes effect immediately.
 *   RevertPendingChanges()  Discard edits: Pending = Current (a menu's "Cancel" / "Back").
 *
 * Dirty state is always *derived* (Pending vs Current) rather than tracked with a separate
 * bool, so it can never drift out of sync. */
class RAGESETTINGS_API IRageSettingsCategoryInterface
{
	GENERATED_BODY()

public:
	virtual void LoadSettings() = 0;
	virtual void ApplySettings() = 0;
	virtual void SaveSettings() = 0;
	virtual void ResetToDefault() = 0;
	virtual void RevertPendingChanges() = 0;
	virtual bool IsDirty() const = 0;
	virtual ERageSettingsCategory GetCategoryId() const = 0;

	/** Called by a caller (e.g. the generic UI row generator) that just wrote a field on
	 * GetPendingSettings() directly via reflection, bypassing any typed SetPendingX() call, so
	 * the category can still re-derive and broadcast its dirty state. Pass IsDirty() captured
	 * *before* that external write. */
	virtual void NotifyPendingChangedExternally(bool bWasDirtyBefore) = 0;

	/** True while ApplySettings has work still outstanding on a later tick. Categories that apply
	 *  everything synchronously need not override this. */
	virtual bool IsApplyInProgress() const { return false; }

	/** The shadow the category stages edits into, which the subsystem diffs against the live object
	 * to work out what an apply is about to change. */
	virtual const UObject* GetPendingObject() const = 0;

	/** Where this category's per-property listeners live. Prefer OnSettingChanged below, which is
	 * the same thing without the reader having to know a registry exists. */
	virtual FRageSettingChangeRegistry& GetChangeRegistry() = 0;

	/** Listeners for one property of this category, fired once its apply has settled:
	 *
	 *   Settings->OnSettingChanged(GET_MEMBER_NAME_CHECKED(URageInputSettings, bInvertMouseY))
	 *       .AddUObject(this, &ARagePlayerController::HandleInvertYChanged);
	 *
	 * This is the live value changing, not the player staging an edit they may still cancel, so a
	 * handler can read the setting through its ordinary getter and act on it. */
	FRageSettingChangedNative& OnSettingChanged(FName PropertyName) { return GetChangeRegistry().OnPropertyChanged(PropertyName); }

	/** As above, called once for each property of this category that changed. */
	FRageSettingChangedNative& OnAnySettingChanged() { return GetChangeRegistry().OnAnyPropertyChanged(); }
};
