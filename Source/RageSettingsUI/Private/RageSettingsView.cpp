// Copyright (c) 2026 Abdallah Boutrif
// SPDX-License-Identifier: MIT

#include "RageSettingsView.h"

#include "RageSettingsSubsystem.h"
#include "RageGameSettingsPanel.h"
#include "RageAudioSettingsPanel.h"
#include "RageVideoSettingsPanel.h"
#include "RageInputSettingsPanel.h"
#include "RageConfirmModal.h"
#include "RageMacros.h"
#include "RageUnsavedChangesModal.h"
#include "RageSettingsPanelInterface.h"
#include "Components/WidgetSwitcher.h"
#include "Components/Button.h"
#include "Components/Widget.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(RageSettingsView)

void URageSettingsView::ShowCategory(ERageSettingsCategory Category)
{
	ActiveCategory = Category;

	if (UUserWidget* TargetPanel = GetPanelFor(Category))
	{
		CategorySwitcher->SetActiveWidget(TargetPanel);
	}

	/* Refresh panel to force showing the current settings. */
	RefreshPanel(Category);
}

void URageSettingsView::RequestClose()
{
	if (Subsystem->HasAnyDirtySettings() && IsValid(UnsavedChangesModal))
	{
		UnsavedChangesModal->Open();
		return;
	}

	CloseImmediately();
}

void URageSettingsView::RefreshAll()
{
	RefreshPanel(ERageSettingsCategory::Game);
	RefreshPanel(ERageSettingsCategory::Audio);
	RefreshPanel(ERageSettingsCategory::Video);
	RefreshPanel(ERageSettingsCategory::Input);

	RefreshDirtyMarkers();
	RefreshApplyButtonEnabled();
}

void URageSettingsView::NativeConstruct()
{
	Super::NativeConstruct();
	
	Subsystem = URageSettingsSubsystem::Get(this);
	check(Subsystem);
	
	DirtyMarkers = {
		{ERageSettingsCategory::Game, GameTabDirtyMarker},
		{ERageSettingsCategory::Audio, AudioTabDirtyMarker},
		{ERageSettingsCategory::Video, VideoTabDirtyMarker},
		{ERageSettingsCategory::Input, InputTabDirtyMarker}};

	GameTabButton->OnClicked.AddUniqueDynamic(this, &URageSettingsView::HandleGameTabClicked);
	AudioTabButton->OnClicked.AddUniqueDynamic(this, &URageSettingsView::HandleAudioTabClicked);
	VideoTabButton->OnClicked.AddUniqueDynamic(this, &URageSettingsView::HandleVideoTabClicked);
	InputTabButton->OnClicked.AddUniqueDynamic(this, &URageSettingsView::HandleInputTabClicked);

	ApplyButton->OnClicked.AddUniqueDynamic(this, &URageSettingsView::HandleApplyClicked);
	ResetToDefaultsButton->OnClicked.AddUniqueDynamic(this, &URageSettingsView::HandleResetToDefaultsClicked);
	CloseButton->OnClicked.AddUniqueDynamic(this, &URageSettingsView::HandleCloseClicked);

	if (IsValid(UnsavedChangesModal))
	{
		UnsavedChangesModal->ApplyAndCloseChosenDelegate.AddUniqueDynamic(this, &URageSettingsView::HandleModalApplyAndClose);
		UnsavedChangesModal->DiscardAndCloseChosenDelegate.AddUniqueDynamic(this, &URageSettingsView::HandleModalDiscardAndClose);
		UnsavedChangesModal->CancelChosenDelegate.AddUniqueDynamic(this, &URageSettingsView::HandleModalCancel);
	}

	if (IsValid(RestartRequiredModal))
	{
		RestartRequiredModal->SetMessage(RAGE_LOC("RestartRequired"));
		RestartRequiredModal->ConfirmChosenDelegate.AddUniqueDynamic(this, &URageSettingsView::HandleRestartConfirmed);
		RestartRequiredModal->CancelChosenDelegate.AddUniqueDynamic(this, &URageSettingsView::HandleRestartDeclined);
	}

	/* Necessary as if the view is not destroyed but rather hidden, it will not refresh itself. */
	OnNativeVisibilityChanged.RemoveAll(this);
	OnNativeVisibilityChanged.AddUObject(this, &URageSettingsView::HandleVisibilityChanged);

	InitializeView();
}

void URageSettingsView::NativeDestruct()
{
	OnNativeVisibilityChanged.RemoveAll(this);

	Super::NativeDestruct();
}

void URageSettingsView::InitializeView()
{
	GamePanel->InitializePanel(Subsystem);
	AudioPanel->InitializePanel(Subsystem);
	VideoPanel->InitializePanel(Subsystem);
	InputPanel->InitializePanel(Subsystem);

	Subsystem->AnyCategoryDirtyStateChangedDelegate.AddUniqueDynamic(this, &URageSettingsView::HandleAnyCategoryDirtyStateChanged);
	Subsystem->RestartRequirementEvaluatedDelegate.AddUniqueDynamic(this, &URageSettingsView::HandleRestartRequirementEvaluated);

	ShowCategory(DefaultCategory);
	RefreshAll();
}

UUserWidget* URageSettingsView::GetPanelFor(const ERageSettingsCategory Category) const
{
	switch (Category)
	{
		case ERageSettingsCategory::Game:  return GamePanel;
		case ERageSettingsCategory::Audio: return AudioPanel;
		case ERageSettingsCategory::Video: return VideoPanel;
		case ERageSettingsCategory::Input: return InputPanel;
	}

	return nullptr;
}

UWidget* URageSettingsView::GetDirtyMarkerFor(const ERageSettingsCategory Category) const
{
	return DirtyMarkers[Category].Get();
}

void URageSettingsView::RefreshPanel(const ERageSettingsCategory Category)
{
	if (IRageSettingsPanelInterface* Panel = Cast<IRageSettingsPanelInterface>(GetPanelFor(Category)))
	{
		Panel->RefreshFromSettings();
	}
}

void URageSettingsView::RefreshDirtyMarkers()
{
	for (const ERageSettingsCategory Category : { ERageSettingsCategory::Game, ERageSettingsCategory::Audio,
		ERageSettingsCategory::Video, ERageSettingsCategory::Input })
	{
		if (UWidget* Marker = GetDirtyMarkerFor(Category))
		{
			Marker->SetVisibility(Subsystem->IsCategoryDirty(Category) ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Collapsed);
		}
	}
}

void URageSettingsView::RefreshApplyButtonEnabled()
{
	ApplyButton->SetIsEnabled(Subsystem->HasAnyDirtySettings());
}

void URageSettingsView::HandleVisibilityChanged(const ESlateVisibility NewVisibility)
{
	if (NewVisibility != ESlateVisibility::Collapsed && NewVisibility != ESlateVisibility::Hidden)
	{
		RefreshAll();
	}
}

void URageSettingsView::CloseImmediately()
{
	ViewClosedDelegate.Broadcast();
}

void URageSettingsView::HandleGameTabClicked()
{
	ShowCategory(ERageSettingsCategory::Game);
}

void URageSettingsView::HandleAudioTabClicked()
{
	ShowCategory(ERageSettingsCategory::Audio);
}

void URageSettingsView::HandleVideoTabClicked()
{
	ShowCategory(ERageSettingsCategory::Video);
}

void URageSettingsView::HandleInputTabClicked()
{
	ShowCategory(ERageSettingsCategory::Input);
}

void URageSettingsView::HandleApplyClicked()
{
	bCloseViewAfterRestartPrompt = false;
	
	Subsystem->ApplyAndSaveAllDirtySettings();
}

void URageSettingsView::HandleResetToDefaultsClicked()
{
	bCloseViewAfterRestartPrompt = false;

	Subsystem->ResetCategoryToDefault(ActiveCategory);

	RefreshPanel(ActiveCategory);
}

void URageSettingsView::HandleCloseClicked()
{
	RequestClose();
}

void URageSettingsView::HandleAnyCategoryDirtyStateChanged(ERageSettingsCategory Category, bool bIsDirty)
{
	if (UWidget* Marker = GetDirtyMarkerFor(Category))
	{
		Marker->SetVisibility(bIsDirty ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Collapsed);
	}

	RefreshApplyButtonEnabled();
}

void URageSettingsView::HandleModalApplyAndClose()
{
	UnsavedChangesModal->Close();
	
	bCloseViewAfterRestartPrompt = true;

	Subsystem->ApplyAndSaveAllDirtySettings();
}

void URageSettingsView::HandleModalDiscardAndClose()
{
	UnsavedChangesModal->Close();

	Subsystem->RevertAllPendingChanges();

	/* This should be handled via visibility changed now, but it's a safe operation to do. */
	RefreshAll();

	CloseImmediately();
}

void URageSettingsView::HandleModalCancel()
{
	UnsavedChangesModal->Close();
}

void URageSettingsView::HandleRestartRequirementEvaluated(bool bRestartRequired)
{
	if (bRestartRequired && IsValid(RestartRequiredModal))
	{
		RestartRequiredModal->Open();
		return;
	}
	
	if (bCloseViewAfterRestartPrompt)
	{
		bCloseViewAfterRestartPrompt = false;
		CloseImmediately();
	}
}

void URageSettingsView::HandleRestartConfirmed()
{
	RestartRequiredModal->Close();

	if (!Subsystem->RestartGame())
	{
		HandleRestartDeclined();
	}
}

void URageSettingsView::HandleRestartDeclined()
{
	RestartRequiredModal->Close();

	if (bCloseViewAfterRestartPrompt)
	{
		bCloseViewAfterRestartPrompt = false;
		CloseImmediately();
	}
}
