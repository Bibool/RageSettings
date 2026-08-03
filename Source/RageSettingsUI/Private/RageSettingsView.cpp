// Copyright (c) 2026 Abdallah Boutrif
// SPDX-License-Identifier: MIT

#include "RageSettingsView.h"

#include "RageSettingsSubsystem.h"
#include "RageGameSettingsPanel.h"
#include "RageAudioSettingsPanel.h"
#include "RageVideoSettingsPanel.h"
#include "RageInputSettingsPanel.h"
#include "RageUnsavedChangesModal.h"
#include "Components/WidgetSwitcher.h"
#include "Components/Button.h"
#include "Components/Widget.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(RageSettingsView)

void URageSettingsView::ShowCategory(ERageSettingsCategory Category)
{
	if (ActiveCategory == Category)
	{
		// Avoid unnecessary refresh
		return;
	}
		
	ActiveCategory = Category;

	UUserWidget* TargetPanel = nullptr;
	IRageSettingsPanelInterface* PanelInterface = nullptr;

	switch (Category)
	{
		case ERageSettingsCategory::Game:  TargetPanel = GamePanel;  PanelInterface = GamePanel;  break;
		case ERageSettingsCategory::Audio: TargetPanel = AudioPanel; PanelInterface = AudioPanel; break;
		case ERageSettingsCategory::Video: TargetPanel = VideoPanel; PanelInterface = VideoPanel; break;
		case ERageSettingsCategory::Input: TargetPanel = InputPanel; PanelInterface = InputPanel; break;
	}
	
	if (IsValid(TargetPanel))
	{
		CategorySwitcher->SetActiveWidget(TargetPanel);
	}

	if (PanelInterface)
	{
		PanelInterface->RefreshFromSettings();
	}
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
	
	InitializeView();
}

void URageSettingsView::InitializeView()
{
	GamePanel->InitializePanel(Subsystem);
	GamePanel->RefreshFromSettings();
	
	AudioPanel->InitializePanel(Subsystem); 
	AudioPanel->RefreshFromSettings();
	
	VideoPanel->InitializePanel(Subsystem); 
	VideoPanel->RefreshFromSettings();
	
	InputPanel->InitializePanel(Subsystem); 
	InputPanel->RefreshFromSettings();

	Subsystem->AnyCategoryDirtyStateChangedDelegate.AddUniqueDynamic(this, &URageSettingsView::HandleAnyCategoryDirtyStateChanged);

	ShowCategory(DefaultCategory);
	RefreshApplyButtonEnabled();
}

UWidget* URageSettingsView::GetDirtyMarkerFor(const ERageSettingsCategory Category) const
{
	return DirtyMarkers[Category].Get();
}

void URageSettingsView::RefreshApplyButtonEnabled()
{
	ApplyButton->SetIsEnabled(Subsystem->HasAnyDirtySettings());
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
	Subsystem->ApplyAndSaveAllDirtySettings();
}

void URageSettingsView::HandleResetToDefaultsClicked()
{
	Subsystem->ResetCategoryToDefault(ActiveCategory);
	
	ShowCategory(ActiveCategory);
}

void URageSettingsView::HandleCloseClicked()
{
	RequestClose();
}

void URageSettingsView::HandleAnyCategoryDirtyStateChanged(ERageSettingsCategory Category, bool bIsDirty)
{
	GetDirtyMarkerFor(Category)->SetVisibility(bIsDirty ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Collapsed);
	
	RefreshApplyButtonEnabled();
}

void URageSettingsView::HandleModalApplyAndClose()
{
	UnsavedChangesModal->Close();

	Subsystem->ApplyAndSaveAllDirtySettings();

	CloseImmediately();
}

void URageSettingsView::HandleModalDiscardAndClose()
{
	UnsavedChangesModal->Close();

	Subsystem->RevertAllPendingChanges();

	CloseImmediately();
}

void URageSettingsView::HandleModalCancel()
{
	UnsavedChangesModal->Close();
}
