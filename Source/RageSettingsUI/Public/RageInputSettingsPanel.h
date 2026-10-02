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
struct FRageModalChange;

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

	/** Uses NewModal for keybind conflicts in place of the one this panel was built with. A modal belongs
	 * over the whole screen, which a panel sitting inside a scroll box cannot give it, so the view that
	 * owns the screen hands its own down. */
	UFUNCTION(BlueprintCallable, Category = "Rage|Input")
	void SetKeybindConflictModal(URageConfirmModal* NewModal);

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

	/** Lets the player pick between swapping keys and unbinding the other action, through the conflict
	 * modal's alternate choice. Off leaves the pick to bRebindConflictSwapsKey and the modal only confirms. */
	UPROPERTY(EditDefaultsOnly, Category = "Rage|Input")
	bool bOfferUnbindOnConflict = false;

private:
	void BuildKeybindRows();

	URageKeybindRow* CreateKeybindRow(const FRageKeybindConfig& Config);

	void RefreshKeyForMapping(FName MappingName) const;

	/** Applies the stashed remap and, if it took, rehomes whoever was holding the key. */
	void CommitPendingRemap(bool bSwap);

	bool ApplyRemap(FName MappingName, FKey NewKey);

	/** Hands the first displaced action the freed key when bSwap, and unbinds the rest. */
	void ResolveConflictingMappings(bool bSwap);

	/** Whether the pending remap will hand its old key over rather than leaving the other unbound. */
	bool WillSwapConflictingKey() const;

	/* Whether the modal asks swap or unbind, rather than confirming the one bRebindConflictSwapsKey picked. */
	bool ShouldOfferConflictChoice() const;

	FText GetMappingDisplayName(FName MappingName) const;

	FText BuildConflictMessage() const;

	TArray<FRageModalChange> BuildConflictChanges(bool bSwap) const;

	void BindConflictModal();

	void ClearPendingRemap();

	UFUNCTION()
	void HandleKeyRemapped(FName MappingName, FKey NewKey);

	UFUNCTION()
	void HandleResetKeyToDefaultRequested(FName MappingName);

	UFUNCTION()
	void HandleConflictConfirmed();

	UFUNCTION()
	void HandleConflictAlternateChosen();

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
