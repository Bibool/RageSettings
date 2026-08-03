// Copyright (c) 2026 Abdallah Boutrif
// SPDX-License-Identifier: MIT

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "RageSettingsPanelInterface.h"
#include "RageSettingsRowGenerator.h"
#include "RageVariant.h"
#include "RageSettingsRowGeneratorPanelBase.generated.h"

class UPanelWidget;
class URageToggleRow;
class URageSliderRow;
class URageComboRow;
class IRageSettingsCategoryInterface;

/**
 * Reusable base for a settings panel whose rows are generated at runtime from whichever
 * Config+EditAnywhere properties are visible on its category's Pending object - base plugin
 * fields and any project-subclass-added fields alike, with zero UMG/Blueprint work per field.
 *
 * Not mandatory: a panel can stay fully hand-authored (see RageVideoSettingsPanel, whose rows
 * pull runtime hardware-capability data that can't be inferred from a UPROPERTY), or, like this
 * base class itself, call BuildRows()/RefreshRowsFromSettings() for only a subset of its rows
 * alongside hand-authored ones. */
UCLASS(Abstract, meta=(DisableNativeTick))
class RAGESETTINGSUI_API URageSettingsRowGeneratorPanelBase : public UUserWidget, public IRageSettingsPanelInterface
{
	GENERATED_BODY()

public:
//~ Begin IRageSettingsPanel
	virtual void InitializePanel(URageSettingsSubsystem* InSubsystem) override { checkNoEntry(); }
	virtual ERageSettingsCategory GetCategoryId() const override { checkNoEntry(); return ERageSettingsCategory::Game; }
	virtual void RefreshFromSettings() override;
//~ End IRageSettingsPanel

protected:
	/** Override to supply clamp range / display format / label overrides for this panel's numeric
	 * rows. Only really useful for for sliders - toggles/combos get a usable row with zero entries here. */
	virtual TArray<FRageSettingsRowDescriptor> GetRowDescriptors() const { return {}; }

	/** Spawns one row per Config+EditAnywhere property found on PendingObject's class (base-to-
	 * derived declaration order) into Container, using ToggleRowClass/SliderRowClass/ComboRowClass,
	 * and immediately populates them via RefreshRowsFromSettings(). */
	void BuildRows(UPanelWidget* Container, UObject* PendingObject);

	/** Pushes each generated row's current value from PendingObject, without notifying. */
	void RefreshRowsFromSettings(const UObject* PendingObject) const;
	
	/* The main panel widget that contains all generated rows. */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UPanelWidget> RowsContainer = nullptr;

private:
	void HandleGeneratedToggleChanged(FName RowId, FRageVariant bNewValue);
	void HandleGeneratedSliderChanged(FName RowId, FRageVariant NewValue);
	void HandleGeneratedComboChanged(FName RowId, FRageVariant NewIndex);
	void HandleGeneratedSelectionChanged(FName RowId, FRageVariant NewIndex);

	FProperty* FindRowProperty(FName RowId) const;
	
	IRageSettingsCategoryInterface* GetOwningCategory() const;

	UPROPERTY()
	TObjectPtr<UObject> BoundPendingObject = nullptr;
};
