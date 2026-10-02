// Copyright (c) 2026 Abdallah Boutrif
// SPDX-License-Identifier: MIT

#include "RageComboRow.h"

#include "Components/ComboBoxString.h"
#include "Internationalization/Internationalization.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(RageComboRow)

void URageComboRow::SetOptions(const TArray<FString>& NewOptions)
{
	TArray<FText> Texts;
	Texts.Reserve(NewOptions.Num());
	for (const FString& Option : NewOptions)
	{
		Texts.Add(FText::AsCultureInvariant(Option));
	}

	bOptionsAreText = false;
	ApplyOptions(Texts);
}

void URageComboRow::SetOptionTexts(const TArray<FText>& NewOptions)
{
	bOptionsAreText = true;
	ApplyOptions(NewOptions);
}

void URageComboRow::SetSelectedIndex(int32 NewIndex, bool bNotify)
{
	const int32 ValidIndex = Options.IsValidIndex(NewIndex) ? NewIndex : INDEX_NONE;
	if (ValidIndex == SelectedIndex)
	{
		return;
	}

	SelectedIndex = ValidIndex;

	if (IsValid(ComboBox))
	{
		bSuppressNotify = true;
		if (SelectedIndex == INDEX_NONE)
		{
			ComboBox->ClearSelection();
		}
		else
		{
			ComboBox->SetSelectedIndex(SelectedIndex);
		}
		bSuppressNotify = false;
	}

	NativeOnSelectionChanged();
	RefreshModified();

	if (bNotify)
	{
		ValueChangedDelegate.Broadcast(RowId, VAL(int32, SelectedIndex));
	}
}

int32 URageComboRow::GetSelectedIndex() const
{
	return SelectedIndex;
}

int32 URageComboRow::GetOptionCount() const
{
	return Options.Num();
}

FText URageComboRow::GetOptionText(int32 Index) const
{
	return Options.IsValidIndex(Index) ? Options[Index] : FText::GetEmpty();
}

void URageComboRow::NativeConstruct()
{
	Super::NativeConstruct();

	if (IsValid(ComboBox))
	{
		ComboBox->OnSelectionChanged.AddUniqueDynamic(this, &URageComboRow::HandleSelectionChanged);
	}

	CultureChangedHandle = FInternationalization::Get().OnCultureChanged().AddUObject(this, &URageComboRow::HandleCultureChanged);
}

void URageComboRow::NativeDestruct()
{
	FInternationalization::Get().OnCultureChanged().Remove(CultureChangedHandle);
	CultureChangedHandle.Reset();

	Super::NativeDestruct();
}

FString URageComboRow::GetValueKey() const
{
	return FString::FromInt(SelectedIndex);
}

FText URageComboRow::GetValueText() const
{
	return GetOptionText(SelectedIndex);
}

void URageComboRow::NativeOnOptionsChanged()
{
}

void URageComboRow::NativeOnSelectionChanged()
{
}

void URageComboRow::HandleSelectionChanged(FString SelectedItem, ESelectInfo::Type SelectionType)
{
	if (bSuppressNotify)
	{
		return;
	}

	SetSelectedIndex(ComboBox->GetSelectedIndex(), /*bNotify*/ true);
}

void URageComboRow::HandleCultureChanged()
{
	/* Text sitting in a text block retranslates itself, but the strings handed to the box are copies
	 * taken in the language of the moment, so the box is the one thing here that has to be rebuilt.
	 * Rows given plain strings have nothing to rebuild from and are left as they are. */
	if (bOptionsAreText)
	{
		SyncComboBoxOptions();
		NativeOnOptionsChanged();
	}
}

void URageComboRow::ApplyOptions(const TArray<FText>& NewOptions)
{
	Options = NewOptions;

	/* The same choices under new names, or a fresh list the caller is about to select into itself. */
	if (!Options.IsValidIndex(SelectedIndex))
	{
		SelectedIndex = INDEX_NONE;
	}

	SyncComboBoxOptions();
	NativeOnOptionsChanged();
	RefreshModified();
}

void URageComboRow::SyncComboBoxOptions()
{
	if (!IsValid(ComboBox))
	{
		return;
	}

	/* Emptying a live box clears its selection, and a cleared selection reports itself as a change the
	 * player made, at an index that is no longer in any list. Nothing in here is the player choosing anything. */
	bSuppressNotify = true;
	ComboBox->ClearOptions();

	for (const FText& Option : Options)
	{
		ComboBox->AddOption(Option.ToString());
	}

	if (SelectedIndex != INDEX_NONE)
	{
		ComboBox->SetSelectedIndex(SelectedIndex);
	}
	bSuppressNotify = false;

	if (Options.Num() == 1)
	{
		ComboBox->SetIsEnabled(false);
	}
}
