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
class URageSelectionRow;
class URageRowBaseUserWidget;
class IRageSettingsCategoryInterface;

/* A titled group of generated rows, listed in the order its properties are named. */
USTRUCT(BlueprintType)
struct RAGESETTINGSUI_API FRageSettingsSection
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rage|UI")
	FText Title = FText::GetEmpty();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rage|UI")
	TArray<FName> Properties;
};

/**
 * Reusable base for a settings panel whose rows are generated at runtime from whichever
 * Config+EditAnywhere properties are visible on its category's Pending object - base plugin
 * fields and any project-subclass-added fields alike, with zero UMG/Blueprint work per field.
 *
 * Not mandatory: a panel can stay fully hand-authored (see RageVideoSettingsPanel, whose rows
 * pull runtime hardware-capability data that can't be inferred from a UPROPERTY), or, like this
 * base class itself, call BuildRows()/RefreshRowsFromSettings() for only a subset of its rows
 * alongside hand-authored ones.
 *
 * Row classes resolve per property in this order: the panel's RowClassOverrides, the project's
 * RowWidgetClassOverrides, the panel's class for that kind of row, then the project default. That lets
 * two panels for the same category, an old one and a restyled one, live side by side. */
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
	 * derived declaration order, or the order Sections gives) into Container, and immediately
	 * populates them via RefreshRowsFromSettings(). */
	void BuildRows(UPanelWidget* Container, UObject* PendingObject);

	/** Pushes each generated row's current value from PendingObject, without notifying. */
	void RefreshRowsFromSettings(const UObject* PendingObject) const;

	/* The header class sections and keybind categories are titled with on this panel. */
	TSubclassOf<URageRowBaseUserWidget> GetSectionWidgetClass() const;

	/* The main panel widget that contains all generated rows. */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UPanelWidget> RowsContainer = nullptr;

	/* Unset falls back to the project default in the Settings UI developer settings. */
	UPROPERTY(EditDefaultsOnly, Category = "Rage|UI|Rows")
	TSubclassOf<URageToggleRow> ToggleRowClass = nullptr;

	UPROPERTY(EditDefaultsOnly, Category = "Rage|UI|Rows")
	TSubclassOf<URageSliderRow> SliderRowClass = nullptr;

	UPROPERTY(EditDefaultsOnly, Category = "Rage|UI|Rows")
	TSubclassOf<URageComboRow> ComboRowClass = nullptr;

	UPROPERTY(EditDefaultsOnly, Category = "Rage|UI|Rows")
	TSubclassOf<URageSelectionRow> SelectionRowClass = nullptr;

	UPROPERTY(EditDefaultsOnly, Category = "Rage|UI|Rows")
	TSubclassOf<URageRowBaseUserWidget> SectionWidgetClass = nullptr;

	/** A row class for one property on this panel only, ahead of the project's own overrides. An enum
	 * property given a combo or a selection row here becomes that kind of row whatever the project says. */
	UPROPERTY(EditDefaultsOnly, Category = "Rage|UI|Rows")
	TMap<FName, TSubclassOf<URageRowBaseUserWidget>> RowClassOverrides;

	/* Groups the generated rows under titles. Empty lists them in declaration order with no headers. */
	UPROPERTY(EditDefaultsOnly, Category = "Rage|UI|Rows")
	TArray<FRageSettingsSection> Sections;

	/** The title over rows that no section names, such as fields a project subclass added later. They
	 * list after every section, and under no header of their own while this is empty. */
	UPROPERTY(EditDefaultsOnly, Category = "Rage|UI|Rows")
	FText UnlistedSectionTitle = FText::GetEmpty();

private:
	URageRowBaseUserWidget* CreateRow(FProperty* Property, const TArray<FRageSettingsRowDescriptor>& Descriptors);
	void AddSectionHeader(UPanelWidget* Container, const FText& Title, int32 SectionIndex);

	RageSettingsUI::ERowKind ResolveKind(const FProperty* Property) const;
	UClass* ResolveRowClass(const FProperty* Property, const UClass* ExpectedBase, UClass* PanelDefault, UClass* ProjectDefault) const;

	void HandleGeneratedToggleChanged(FName RowId, FRageVariant bNewValue);
	void HandleGeneratedSliderChanged(FName RowId, FRageVariant NewValue);
	void HandleGeneratedComboChanged(FName RowId, FRageVariant NewIndex);
	void HandleGeneratedSelectionChanged(FName RowId, FRageVariant NewIndex);

	FProperty* FindRowProperty(FName RowId) const;

	IRageSettingsCategoryInterface* GetOwningCategory() const;

	UPROPERTY()
	TObjectPtr<UObject> BoundPendingObject = nullptr;
};
