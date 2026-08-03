// Copyright (c) 2026 Abdallah Boutrif
// SPDX-License-Identifier: MIT

#pragma once

#include "CoreMinimal.h"
#include "RageSettingsRowGeneratorPanelBase.h"
#include "RageSettingsUIStatics.h"
#include "RageInputSettingsPanel.generated.h"

class URageInputSettings;
class URageKeybindRow;
class URageConfirmModal;
class UPanelWidget;
struct FRageKeybindConfig;

/**
 * Input settings panel. Analog/toggle rows (sensitivity, inversion, and any project-subclass-
 * added fields) are generated at runtime by the inherited row generator - see
 * URageSettingsRowGeneratorPanelBase and RageSettingsDeveloperSettings::InputSettingsClass.
 * Keybind rows remain hand-built (BuildKeybindRows) since they come from a designer-configured
 * list of mapping names, not from reflected properties.
 */
UCLASS(Abstract, meta=(DisableNativeTick))
class RAGESETTINGSUI_API URageInputSettingsPanel : public URageSettingsRowGeneratorPanelBase
{
	GENERATED_BODY()

public:
	//~ Begin IRageSettingsPanel
	virtual void InitializePanel(URageSettingsSubsystem* InSubsystem) override;
	virtual void RefreshFromSettings() override;
	virtual ERageSettingsCategory GetCategoryId() const override { return ERageSettingsCategory::Input; }
	//~ End IRageSettingsPanel

protected:
	virtual void NativeConstruct() override;

	virtual TArray<FRageSettingsRowDescriptor> GetRowDescriptors() const override;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UPanelWidget> KeybindListContainer = nullptr;

	/* Asks before taking a key off another action. Without one the rebind still resolves the
	 * conflict, it just doesn't ask first. */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<URageConfirmModal> KeybindConflictModal = nullptr;

	UPROPERTY(EditDefaultsOnly, Category = "Rage|Input")
	TSubclassOf<URageKeybindRow> KeybindRowClass = nullptr;

private:
	void BuildKeybindRows();

	URageKeybindRow* CreateKeybindRow(const FRageKeybindConfig& Config);

	void RefreshKeyForMapping(FName MappingName) const;

	/** Applies the stashed remap and, if it took, rehomes whoever was holding the key. */
	void CommitPendingRemap();

	bool ApplyRemap(FName MappingName, FKey NewKey);

	/** Hands the displaced action the freed key, or unbinds it - see bRebindConflictSwapsKey. */
	void ResolveConflictingMappings();

	/** Whether the pending remap will hand its old key over rather than leaving the other unbound. */
	bool WillSwapConflictingKey() const;

	FText BuildConflictMessage() const;

	void ClearPendingRemap();

	UFUNCTION()
	void HandleKeyRemapped(FName MappingName, FKey NewKey);

	UFUNCTION()
	void HandleResetKeyToDefaultRequested(FName MappingName);

	UFUNCTION()
	void HandleConflictConfirmed();

	UFUNCTION()
	void HandleConflictCancelled();

	UPROPERTY()
	TArray<TObjectPtr<URageKeybindRow>> KeybindRows;

	UPROPERTY()
	TObjectPtr<URageInputSettings> InputSettings;

	UPROPERTY()
	TMap<FName, URageKeybindRow*> KeybindRowsByMapping;

	/* The remap in flight - stashed because the conflict modal answers on a later frame. */
	FName PendingRemapMappingName = NAME_None;
	FKey PendingRemapKey;
	/* What the mapping held before the remap, captured up front - this is what a swap hands over. */
	FKey PendingRemapPreviousKey;
	TArray<FName> PendingRemapConflicts;
};
