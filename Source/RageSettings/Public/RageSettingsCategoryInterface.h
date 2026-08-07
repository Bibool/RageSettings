// Copyright (c) 2026 Abdallah Boutrif
// SPDX-License-Identifier: MIT

#pragma once

#include "RageSettingsCategory.h"
#include "UObject/Interface.h"
#include "RageSettingsCategoryInterface.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FRageCategoryDirtyStateChanged, ERageSettingsCategory, Category, bool, bDirty);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FRageCategoryApplied, ERageSettingsCategory, Category);
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
};
