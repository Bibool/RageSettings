// Copyright (c) 2026 Abdallah Boutrif
// SPDX-License-Identifier: MIT

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "RageSettingsUIStatics.h"
#include "RageVideoSettingsPanel.h"
#include "RageModalBase.h"
#include "RageSettingsView.generated.h"

class URageSettingsSubsystem;
class URageGameSettingsPanel;
class URageAudioSettingsPanel;
class URageVideoSettingsPanel;
class URageInputSettingsPanel;
class URageConfirmModal;
class URageUnsavedChangesModal;
class URageRowBaseUserWidget;
class UWidgetSwitcher;
class UButton;
class UWidget;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FRageSettingsViewOpened);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FRageSettingsViewClosed);

/**
 * Top-level settings window: a tab strip (Game/Audio/Video/Input) driving a UWidgetSwitcher,
 * plus a shared footer (Apply / Reset to Defaults / Close).
 *
 * This widget does not create or remove itself - whoever spawns it is expected to call
 * InitializeView() once immediately after construction, and to bind OnViewClosed to know when
 * it is actually safe to tear it down (RemoveFromParent(), play a close animation, pop it off
 * a menu stack, ...). Call RequestClose() instead of removing the widget directly - e.g. from
 * a Close button elsewhere in your UI, or from your own back/Escape handling - so pending,
 * un-applied edits get a chance to be caught by the unsaved-changes prompt first.
 *
 * Apply is global (applies every dirty category, not just the visible tab) since that matches
 * how most settings menus behave - tweak Video and Audio, then hit one Apply. Reset to Defaults
 * is scoped to whichever tab is currently active, since resetting every category at once from
 * one button is a bigger, easier-to-regret action than the UI suggests.
 *
 * The tab, footer and marker widgets are all optional, so a subclass can draw them with its own
 * widgets and drive the view through the public functions and the native hooks instead.
 *
 * The view also keeps every row its panels hold. Whenever a category has nothing pending it takes
 * its rows' values as their baseline, which is how it can name the exact rows a player has changed
 * and list them in the unsaved-changes prompt, and it passes on which row the player is pointing at
 * for a details pane to describe.
 */
UCLASS(Abstract, meta=(DisableNativeTick))
class RAGESETTINGSUI_API URageSettingsView : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Rage|Settings|View")
	void CycleCategory(int32 Direction);

	UFUNCTION(BlueprintCallable, Category = "Rage|Settings|View")
	void ShowCategory(ERageSettingsCategory Category);

	/** Requests the view close. If nothing is pending, fires OnViewClosed immediately; if there
	 *  are un-applied edits, shows the unsaved-changes prompt instead and fires OnViewClosed
	 *  only once that's resolved. */
	UFUNCTION(BlueprintCallable, Category = "Rage|Settings|View")
	void RequestClose();

	/** Pulls every panel and the dirty/Apply affordances back in sync with the subsystem. Called
	 *  automatically on construction and whenever this widget is made visible again. */
	UFUNCTION(BlueprintCallable, Category = "Rage|Settings|View")
	void RefreshAll();

	/* Applies and saves every category with pending changes. */
	UFUNCTION(BlueprintCallable, Category = "Rage|Settings|View")
	void ApplyAll();

	UFUNCTION(BlueprintCallable, Category = "Rage|Settings|View")
	void ResetActiveCategoryToDefaults();

	UFUNCTION(BlueprintPure, Category = "Rage|Settings|View")
	ERageSettingsCategory GetActiveCategory() const;

	UFUNCTION(BlueprintPure, Category = "Rage|Settings|View")
	bool IsCategoryDirty(ERageSettingsCategory Category) const;

	UFUNCTION(BlueprintPure, Category = "Rage|Settings|View")
	bool CanApply() const;

	/* The rows whose value is off their baseline, in tab order. */
	UFUNCTION(BlueprintPure, Category = "Rage|Settings|View")
	TArray<URageRowBaseUserWidget*> GetModifiedRows() const;

	UFUNCTION(BlueprintPure, Category = "Rage|Settings|View")
	int32 GetModifiedRowCount() const;

	/* The modified rows as a modal's change list: each row's label, baseline and current value. */
	UFUNCTION(BlueprintPure, Category = "Rage|Settings|View")
	TArray<FRageModalChange> BuildModifiedChanges() const;

	UFUNCTION(BlueprintPure, Category = "Rage|Settings|View")
	URageRowBaseUserWidget* GetHighlightedRow() const;

	UFUNCTION(BlueprintCallable, Category = "Rage|Settings|View")
	void HighlightRow(URageRowBaseUserWidget* Row);

	UFUNCTION(BlueprintImplementableEvent, Category = "Rage|Settings|View")
	void OnCategorySelected(ERageSettingsCategory LastCategory, ERageSettingsCategory NewCategory);

	UFUNCTION(BlueprintImplementableEvent, Category = "Rage|Settings|View")
	void OnButtonDisabled(UWidget* InButton, bool bNewState);

	UFUNCTION(BlueprintImplementableEvent, Category = "Rage|Settings|View")
	void OnModifiedRowCountChanged(int32 NewCount);

	UFUNCTION(BlueprintImplementableEvent, Category = "Rage|Settings|View")
	void OnRowHighlighted(URageRowBaseUserWidget* Row);

	/** Fires once it is actually safe to remove this widget. This view excepts to have its visibility or parentship managed (collapsed/removed) */
	UPROPERTY(BlueprintCallable, BlueprintAssignable, Category = "Rage|Settings|View")
	FRageSettingsViewClosed ViewClosedDelegate;

	UPROPERTY(BlueprintCallable, BlueprintAssignable, Category = "Rage|Settings|View")
	FRageSettingsViewOpened ViewOpenedDelegate;

protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

	virtual void NativeOnCategoryShown(ERageSettingsCategory LastCategory, ERageSettingsCategory NewCategory);
	virtual void NativeOnCategoryDirtyChanged(ERageSettingsCategory Category, bool bIsDirty);
	virtual void NativeOnApplyAvailabilityChanged(bool bCanApply);
	virtual void NativeOnModifiedRowsChanged();
	virtual void NativeOnRowHighlighted(URageRowBaseUserWidget* Row);

	/* Fills the prompt in just before it opens. The base hands it the change list and leaves its wording alone. */
	virtual void NativePopulateUnsavedChangesModal(URageUnsavedChangesModal* Modal);

	UUserWidget* GetPanelFor(ERageSettingsCategory Category) const;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UWidgetSwitcher> CategorySwitcher = nullptr;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<URageGameSettingsPanel> GamePanel = nullptr;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<URageAudioSettingsPanel> AudioPanel = nullptr;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<URageVideoSettingsPanel> VideoPanel = nullptr;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<URageInputSettingsPanel> InputPanel = nullptr;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UButton> GameTabButton = nullptr;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UButton> AudioTabButton = nullptr;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UButton> VideoTabButton = nullptr;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UButton> InputTabButton = nullptr;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UWidget> GameTabDirtyMarker = nullptr;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UWidget> AudioTabDirtyMarker = nullptr;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UWidget> VideoTabDirtyMarker = nullptr;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UWidget> InputTabDirtyMarker = nullptr;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UButton> ApplyButton = nullptr;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UButton> ResetToDefaultsButton = nullptr;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UButton> CloseButton = nullptr;

	// TODO Improve modals, this is wasteful to have two.
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<URageUnsavedChangesModal> UnsavedChangesModal = nullptr;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<URageConfirmModal> RestartRequiredModal = nullptr;

	/* Handed to the input panel, so a key conflict is asked over the whole view rather than inside the panel's scroll box. */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<URageConfirmModal> KeybindConflictModal = nullptr;

	UPROPERTY(EditDefaultsOnly)
	ERageSettingsCategory DefaultCategory = ERageSettingsCategory::Video;

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UUserWidget> ApplyInProgressViewClass = nullptr;

private:
	void InitializeView();

	UWidget* GetDirtyMarkerFor(ERageSettingsCategory Category) const;
	void RefreshPanel(ERageSettingsCategory Category);
	void RefreshDirtyMarkers();
	void RefreshApplyButtonEnabled();
	void CloseImmediately();

	void CollectRows();
	void CaptureBaselines(ERageSettingsCategory Category);
	void CaptureCleanBaselines();
	URageRowBaseUserWidget* FindFirstSettingRow(ERageSettingsCategory Category) const;

	void HandleRowModifiedChanged(URageRowBaseUserWidget* Row);
	void HandleRowHighlighted(URageRowBaseUserWidget* Row);

	void HandleVisibilityChanged(ESlateVisibility NewVisibility);

	UFUNCTION()
	void HandleGameTabClicked();

	UFUNCTION()
	void HandleAudioTabClicked();

	UFUNCTION()
	void HandleVideoTabClicked();

	UFUNCTION()
	void HandleInputTabClicked();

	UFUNCTION()
	void HandleApplyClicked();

	UFUNCTION()
	void HandleResetToDefaultsClicked();

	UFUNCTION()
	void HandleCloseClicked();

	UFUNCTION()
	void HandleAnyCategoryDirtyStateChanged(ERageSettingsCategory Category, bool bIsDirty);

	UFUNCTION()
	void HandleModalApplyAndClose();

	UFUNCTION()
	void HandleModalDiscardAndClose();

	UFUNCTION()
	void HandleModalCancel();

	UFUNCTION()
	void HandleRestartRequirementEvaluated(bool bRestartRequired);

	UFUNCTION()
	void HandleRestartConfirmed();

	UFUNCTION()
	void HandleRestartDeclined();

	UFUNCTION()
	void HandleApplyStarted();

	UFUNCTION()
	void HandleApplyFinished();

	UPROPERTY()
	TObjectPtr<URageSettingsSubsystem> Subsystem = nullptr;

	UPROPERTY()
	TObjectPtr<UUserWidget> LastTab = nullptr;

	TMap<ERageSettingsCategory, TWeakObjectPtr<UWidget>> DirtyMarkers;

	/* Every row each panel holds, headers included, in the order the panel lists them. */
	TMap<ERageSettingsCategory, TArray<TWeakObjectPtr<URageRowBaseUserWidget>>> RowsByCategory;

	TWeakObjectPtr<URageRowBaseUserWidget> HighlightedRow;

	int32 ModifiedRowCount = 0;

	ERageSettingsCategory ActiveCategory = ERageSettingsCategory::Video;

	bool bCloseViewAfterRestartPrompt = false;

	UPROPERTY(Transient)
	TObjectPtr<UUserWidget> ApplyInProgressView = nullptr;
};
