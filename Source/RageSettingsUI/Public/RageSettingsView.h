// Copyright (c) 2026 Abdallah Boutrif
// SPDX-License-Identifier: MIT

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "RageSettingsUIStatics.h"
#include "RageVideoSettingsPanel.h"
#include "RageSettingsView.generated.h"

class URageSettingsSubsystem;
class URageGameSettingsPanel;
class URageAudioSettingsPanel;
class URageVideoSettingsPanel;
class URageInputSettingsPanel;
class URageConfirmModal;
class URageUnsavedChangesModal;
class UWidgetSwitcher;
class UButton;
class UWidget;

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
 */
UCLASS(Abstract, meta=(DisableNativeTick))
class RAGESETTINGSUI_API URageSettingsView : public UUserWidget
{
	GENERATED_BODY()

public:
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

	/** Fires once it is actually safe to remove this widget. This view excepts to have its visibility or parentship managed (collapsed/removed) */
	UPROPERTY(BlueprintAssignable, Category = "Rage|Settings|View")
	FRageSettingsViewClosed ViewClosedDelegate;

protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

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

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget)) 
	TObjectPtr<UButton> GameTabButton = nullptr;
	
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget)) 
	TObjectPtr<UButton> AudioTabButton = nullptr;
	
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget)) 
	TObjectPtr<UButton> VideoTabButton = nullptr;
	
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget)) 
	TObjectPtr<UButton> InputTabButton = nullptr;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget)) 
	TObjectPtr<UWidget> GameTabDirtyMarker = nullptr;
	
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget)) 
	TObjectPtr<UWidget> AudioTabDirtyMarker = nullptr;
	
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget)) 
	TObjectPtr<UWidget> VideoTabDirtyMarker = nullptr;
	
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget)) 
	TObjectPtr<UWidget> InputTabDirtyMarker = nullptr;
	
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget)) 
	TObjectPtr<UButton> ApplyButton = nullptr;
	
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget)) 
	TObjectPtr<UButton> ResetToDefaultsButton = nullptr;
	
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget)) 
	TObjectPtr<UButton> CloseButton = nullptr;

	// TODO Improve modals, this is wasteful to have two.
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<URageUnsavedChangesModal> UnsavedChangesModal = nullptr;
	
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<URageConfirmModal> RestartRequiredModal = nullptr;
	
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional)) 
	TObjectPtr<UUserWidget> ApplyInProgressView = nullptr;
	
	UPROPERTY(EditDefaultsOnly)
	ERageSettingsCategory DefaultCategory = ERageSettingsCategory::Video;

private:
	void InitializeView();

	UUserWidget* GetPanelFor(ERageSettingsCategory Category) const;
	UWidget* GetDirtyMarkerFor(ERageSettingsCategory Category) const;
	void RefreshPanel(ERageSettingsCategory Category);
	void RefreshDirtyMarkers();
	void RefreshApplyButtonEnabled();
	void CloseImmediately();
	
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

	TMap<ERageSettingsCategory, TWeakObjectPtr<UWidget>> DirtyMarkers;

	ERageSettingsCategory ActiveCategory = ERageSettingsCategory::Video;

	bool bCloseViewAfterRestartPrompt = false;
};
