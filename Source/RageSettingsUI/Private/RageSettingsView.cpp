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
#include "RageRowBaseUserWidget.h"
#include "RageUnsavedChangesModal.h"
#include "RageSettingsPanelInterface.h"
#include "Blueprint/WidgetTree.h"
#include "Components/WidgetSwitcher.h"
#include "Components/Button.h"
#include "Components/Widget.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(RageSettingsView)

namespace
{
	constexpr ERageSettingsCategory AllCategories[] = { ERageSettingsCategory::Game, ERageSettingsCategory::Audio,
		ERageSettingsCategory::Video, ERageSettingsCategory::Input };
}

void URageSettingsView::CycleCategory(int32 Direction)
{
	int32 NextIndex = (StaticCast<int32>(ActiveCategory) + Direction + RageSettingsCategory::Count) % RageSettingsCategory::Count;
	ShowCategory(StaticCast<ERageSettingsCategory>(NextIndex));
}

void URageSettingsView::ShowCategory(ERageSettingsCategory Category)
{
	ERageSettingsCategory LastCategory = ActiveCategory;

	ActiveCategory = Category;

	if (UUserWidget* TargetPanel = GetPanelFor(Category))
	{
		CategorySwitcher->SetActiveWidget(TargetPanel);
	}

	/* Refresh panel to force showing the current settings. */
	RefreshPanel(Category);

	/* A details pane left describing a row on the tab just left would be describing something off screen. */
	HighlightRow(FindFirstSettingRow(Category));

	NativeOnCategoryShown(LastCategory, Category);
	OnCategorySelected(LastCategory, Category);
}

void URageSettingsView::RequestClose()
{
	if (Subsystem->HasAnyDirtySettings() && IsValid(UnsavedChangesModal))
	{
		if (UnsavedChangesModal->IsOpen())
		{
			HandleModalCancel();
			return;
		}

		NativePopulateUnsavedChangesModal(UnsavedChangesModal);
		UnsavedChangesModal->Open();
		return;
	}

	CloseImmediately();
}

void URageSettingsView::RefreshAll()
{
	for (const ERageSettingsCategory Category : AllCategories)
	{
		RefreshPanel(Category);
	}

	CaptureCleanBaselines();

	RefreshDirtyMarkers();
	RefreshApplyButtonEnabled();
}

void URageSettingsView::ApplyAll()
{
	bCloseViewAfterRestartPrompt = false;

	Subsystem->ApplyAndSaveAllDirtySettings();
}

void URageSettingsView::ResetActiveCategoryToDefaults()
{
	bCloseViewAfterRestartPrompt = false;

	Subsystem->ResetCategoryToDefault(ActiveCategory);

	RefreshPanel(ActiveCategory);
}

ERageSettingsCategory URageSettingsView::GetActiveCategory() const
{
	return ActiveCategory;
}

bool URageSettingsView::IsCategoryDirty(const ERageSettingsCategory Category) const
{
	return IsValid(Subsystem) && Subsystem->IsCategoryDirty(Category);
}

bool URageSettingsView::CanApply() const
{
	return IsValid(Subsystem) && Subsystem->HasAnyDirtySettings();
}

TArray<URageRowBaseUserWidget*> URageSettingsView::GetModifiedRows() const
{
	TArray<URageRowBaseUserWidget*> Rows;
	for (const ERageSettingsCategory Category : AllCategories)
	{
		if (const TArray<TWeakObjectPtr<URageRowBaseUserWidget>>* CategoryRows = RowsByCategory.Find(Category))
		{
			for (const TWeakObjectPtr<URageRowBaseUserWidget>& Row : *CategoryRows)
			{
				if (Row.IsValid() && Row->IsModified())
				{
					Rows.Add(Row.Get());
				}
			}
		}
	}

	return Rows;
}

int32 URageSettingsView::GetModifiedRowCount() const
{
	return ModifiedRowCount;
}

TArray<FRageModalChange> URageSettingsView::BuildModifiedChanges() const
{
	TArray<FRageModalChange> Changes;
	for (const URageRowBaseUserWidget* Row : GetModifiedRows())
	{
		FRageModalChange& Change = Changes.AddDefaulted_GetRef();
		Change.Label = Row->GetRowLabel();
		Change.From = Row->GetBaselineDisplayText();
		Change.To = Row->GetValueDisplayText();
	}

	return Changes;
}

URageRowBaseUserWidget* URageSettingsView::GetHighlightedRow() const
{
	return HighlightedRow.Get();
}

void URageSettingsView::HighlightRow(URageRowBaseUserWidget* Row)
{
	if (HighlightedRow.Get() == Row)
	{
		return;
	}

	HighlightedRow = Row;

	NativeOnRowHighlighted(Row);
	OnRowHighlighted(Row);
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

	Subsystem->ApplyStartedDelegate.AddUniqueDynamic(this, &URageSettingsView::HandleApplyStarted);
	Subsystem->ApplyFinishedDelegate.AddUniqueDynamic(this, &URageSettingsView::HandleApplyFinished);

	if (IsValid(GameTabButton))
	{
		GameTabButton->OnClicked.AddUniqueDynamic(this, &URageSettingsView::HandleGameTabClicked);
	}

	if (IsValid(AudioTabButton))
	{
		AudioTabButton->OnClicked.AddUniqueDynamic(this, &URageSettingsView::HandleAudioTabClicked);
	}

	if (IsValid(VideoTabButton))
	{
		VideoTabButton->OnClicked.AddUniqueDynamic(this, &URageSettingsView::HandleVideoTabClicked);
	}

	if (IsValid(InputTabButton))
	{
		InputTabButton->OnClicked.AddUniqueDynamic(this, &URageSettingsView::HandleInputTabClicked);
	}

	if (IsValid(ApplyButton))
	{
		ApplyButton->OnClicked.AddUniqueDynamic(this, &URageSettingsView::HandleApplyClicked);
	}

	if (IsValid(ResetToDefaultsButton))
	{
		ResetToDefaultsButton->OnClicked.AddUniqueDynamic(this, &URageSettingsView::HandleResetToDefaultsClicked);
	}

	if (IsValid(CloseButton))
	{
		CloseButton->OnClicked.AddUniqueDynamic(this, &URageSettingsView::HandleCloseClicked);
	}

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

	if (IsValid(KeybindConflictModal))
	{
		InputPanel->SetKeybindConflictModal(KeybindConflictModal);
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

void URageSettingsView::NativeOnCategoryShown(ERageSettingsCategory LastCategory, ERageSettingsCategory NewCategory)
{
}

void URageSettingsView::NativeOnCategoryDirtyChanged(ERageSettingsCategory Category, bool bIsDirty)
{
}

void URageSettingsView::NativeOnApplyAvailabilityChanged(bool bCanApply)
{
}

void URageSettingsView::NativeOnModifiedRowsChanged()
{
}

void URageSettingsView::NativeOnRowHighlighted(URageRowBaseUserWidget* Row)
{
}

void URageSettingsView::NativePopulateUnsavedChangesModal(URageUnsavedChangesModal* Modal)
{
	Modal->SetChanges(BuildModifiedChanges());
}

void URageSettingsView::InitializeView()
{
	GamePanel->InitializePanel(Subsystem);
	AudioPanel->InitializePanel(Subsystem);
	VideoPanel->InitializePanel(Subsystem);
	InputPanel->InitializePanel(Subsystem);

	/* The panels build their generated rows while initializing, so the rows only all exist from here. */
	CollectRows();

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
	const TWeakObjectPtr<UWidget>* Marker = DirtyMarkers.Find(Category);
	return Marker ? Marker->Get() : nullptr;
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
	for (const ERageSettingsCategory Category : AllCategories)
	{
		const bool bIsDirty = Subsystem->IsCategoryDirty(Category);
		if (UWidget* Marker = GetDirtyMarkerFor(Category))
		{
			Marker->SetVisibility(bIsDirty ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Collapsed);
		}

		NativeOnCategoryDirtyChanged(Category, bIsDirty);
	}
}

void URageSettingsView::RefreshApplyButtonEnabled()
{
	const bool bNewState = Subsystem->HasAnyDirtySettings();
	if (IsValid(ApplyButton))
	{
		ApplyButton->SetIsEnabled(bNewState);
	}

	OnButtonDisabled(ApplyButton, bNewState);
	NativeOnApplyAvailabilityChanged(bNewState);
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

void URageSettingsView::CollectRows()
{
	for (TPair<ERageSettingsCategory, TArray<TWeakObjectPtr<URageRowBaseUserWidget>>>& Entry : RowsByCategory)
	{
		for (const TWeakObjectPtr<URageRowBaseUserWidget>& Row : Entry.Value)
		{
			if (Row.IsValid())
			{
				Row->ModifiedChangedDelegate.RemoveAll(this);
				Row->HighlightedDelegate.RemoveAll(this);
			}
		}
	}

	RowsByCategory.Reset();

	for (const ERageSettingsCategory Category : AllCategories)
	{
		const UUserWidget* Panel = GetPanelFor(Category);
		if (!IsValid(Panel) || !IsValid(Panel->WidgetTree))
		{
			continue;
		}

		TArray<TWeakObjectPtr<URageRowBaseUserWidget>>& Rows = RowsByCategory.Add(Category);

		/* Rows live in the panel's tree, generated ones included once they are added to a container in it.
		 * A row is a widget of its own, so the walk sees the row and never the label and buttons inside it. */
		Panel->WidgetTree->ForEachWidget([this, &Rows](UWidget* Widget)
		{
			if (URageRowBaseUserWidget* Row = Cast<URageRowBaseUserWidget>(Widget))
			{
				Row->ModifiedChangedDelegate.AddUObject(this, &URageSettingsView::HandleRowModifiedChanged);
				Row->HighlightedDelegate.AddUObject(this, &URageSettingsView::HandleRowHighlighted);
				Rows.Add(Row);
			}
		});
	}
}

void URageSettingsView::CaptureBaselines(const ERageSettingsCategory Category)
{
	if (const TArray<TWeakObjectPtr<URageRowBaseUserWidget>>* Rows = RowsByCategory.Find(Category))
	{
		for (const TWeakObjectPtr<URageRowBaseUserWidget>& Row : *Rows)
		{
			if (Row.IsValid())
			{
				Row->CaptureBaseline();
			}
		}
	}
}

void URageSettingsView::CaptureCleanBaselines()
{
	/* A category with nothing pending shows exactly what is applied, which is what every row should be measured from.
	 * A dirty one keeps the baseline it had, or has none yet and marks nothing. */
	for (const ERageSettingsCategory Category : AllCategories)
	{
		if (!Subsystem->IsCategoryDirty(Category))
		{
			CaptureBaselines(Category);
		}
	}
}

URageRowBaseUserWidget* URageSettingsView::FindFirstSettingRow(const ERageSettingsCategory Category) const
{
	if (const TArray<TWeakObjectPtr<URageRowBaseUserWidget>>* Rows = RowsByCategory.Find(Category))
	{
		for (const TWeakObjectPtr<URageRowBaseUserWidget>& Row : *Rows)
		{
			if (Row.IsValid() && Row->IsSettingRow() && Row->IsVisible())
			{
				return Row.Get();
			}
		}
	}

	return nullptr;
}

void URageSettingsView::HandleRowModifiedChanged(URageRowBaseUserWidget* Row)
{
	const int32 NewCount = GetModifiedRows().Num();
	if (NewCount == ModifiedRowCount)
	{
		return;
	}

	ModifiedRowCount = NewCount;

	NativeOnModifiedRowsChanged();
	OnModifiedRowCountChanged(ModifiedRowCount);
}

void URageSettingsView::HandleRowHighlighted(URageRowBaseUserWidget* Row)
{
	HighlightRow(Row);
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
	ApplyAll();
}

void URageSettingsView::HandleResetToDefaultsClicked()
{
	ResetActiveCategoryToDefaults();
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

	/* Clean again, by an apply, a revert or a value set back by hand: the rows now show what is applied. */
	if (!bIsDirty)
	{
		RefreshPanel(Category);
		CaptureBaselines(Category);
	}

	NativeOnCategoryDirtyChanged(Category, bIsDirty);
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

void URageSettingsView::HandleApplyStarted()
{
	if (!IsValid(ApplyInProgressView))
	{
		ApplyInProgressView = CreateWidget<UUserWidget>(GetWorld(), ApplyInProgressViewClass);
		ApplyInProgressView->AddToViewport();
	}

	if (IsValid(ApplyInProgressView))
	{
		ApplyInProgressView->SetVisibility(ESlateVisibility::HitTestInvisible);
	}
}

void URageSettingsView::HandleApplyFinished()
{
	if (IsValid(ApplyInProgressView))
	{
		ApplyInProgressView->SetVisibility(ESlateVisibility::Collapsed);
	}
}
