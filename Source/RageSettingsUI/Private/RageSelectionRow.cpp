// Copyright (c) 2026 Abdallah Boutrif
// SPDX-License-Identifier: MIT

#include "RageSelectionRow.h"

#include "RagePipView.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(RageSelectionRow)

void URageSelectionRow::SetOptions(const TArray<FText>& NewOptions)
{
	Options = NewOptions;
	SelectedIndex = Options.IsEmpty() ? INDEX_NONE : FMath::Clamp(SelectedIndex, 0, Options.Num() - 1);

	PipView->SetPipCount(Options.Num());
	RefreshSelection();
}

void URageSelectionRow::SetSelectedIndex(int32 NewIndex, bool bNotify)
{
	const int32 ClampedIndex = Options.IsEmpty() ? INDEX_NONE : FMath::Clamp(NewIndex, 0, Options.Num() - 1);
	if (ClampedIndex == SelectedIndex)
	{
		return;
	}

	SelectedIndex = ClampedIndex;
	RefreshSelection();
	
	if (bNotify)
	{
		ValueChangedDelegate.Broadcast(RowId, VAL(int32, SelectedIndex));
	}
}

int32 URageSelectionRow::GetSelectedIndex() const
{
	return SelectedIndex;
}

int32 URageSelectionRow::GetOptionCount() const
{
	return Options.Num();
}

void URageSelectionRow::NativeOnInitialized()
{
	Super::NativeOnInitialized();
	
	if (PipStyleOverride.IsSet())
	{
		PipView->SetPipStyle(PipStyleOverride.GetValue());
	}
}

void URageSelectionRow::NativeConstruct()
{
	Super::NativeConstruct();

	Left_Button->OnClicked.AddUniqueDynamic(this, &URageSelectionRow::HandleLeftClicked);
	Right_Button->OnClicked.AddUniqueDynamic(this, &URageSelectionRow::HandleRightClicked);
	PipView->PipClickedDelegate.AddUniqueDynamic(this, &URageSelectionRow::HandlePipClicked);

	RefreshSelection();
}

void URageSelectionRow::RefreshSelection_Implementation()
{
	const FText Text = Options.IsValidIndex(SelectedIndex) ? Options[SelectedIndex] : FText::GetEmpty();
	if (IsValid(ValueText))
	{
		ValueText->SetText(Text);
	}
	
	OnValueTextSet(Text);
	
	const bool bCanCycle = Options.Num() > 1;
	Left_Button->SetIsEnabled(bCanCycle && (bWrapAround || SelectedIndex > 0));
	Right_Button->SetIsEnabled(bCanCycle && (bWrapAround || SelectedIndex < Options.Num() - 1));

	PipView->SetSelectedIndex(SelectedIndex);
}

void URageSelectionRow::HandleLeftClicked()
{
	CycleSelection(-1);
}

void URageSelectionRow::HandleRightClicked()
{
	CycleSelection(1);
}

void URageSelectionRow::HandlePipClicked(int32 PipIndex)
{
	SetSelectedIndex(PipIndex, /*bNotify*/ true);
}

void URageSelectionRow::CycleSelection(int8 Direction)
{
	if (Options.IsEmpty())
	{
		return;
	}

	const int32 Count = Options.Num();
	const int32 NextIndex = bWrapAround
		? (SelectedIndex + Direction + Count) % Count
		: FMath::Clamp(SelectedIndex + Direction, 0, Count - 1);

	SetSelectedIndex(NextIndex, /*bNotify*/ true);
}
