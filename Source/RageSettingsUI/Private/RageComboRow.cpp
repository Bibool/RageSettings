// Copyright (c) 2026 Abdallah Boutrif
// SPDX-License-Identifier: MIT

#include "RageComboRow.h"

#include "Components/ComboBoxString.h"
#include "Internationalization/Internationalization.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(RageComboRow)

void URageComboRow::SetOptions(const TArray<FString>& NewOptions)
{
	OptionTexts.Reset();

	ApplyOptionStrings(NewOptions);
}

void URageComboRow::SetOptionTexts(const TArray<FText>& NewOptions)
{
	OptionTexts = NewOptions;

	RefreshOptionStrings();
}

void URageComboRow::SetSelectedIndex(int32 NewIndex, bool bNotify)
{
	bSuppressNotify = !bNotify;
	ComboBox->SetSelectedIndex(NewIndex);
	bSuppressNotify = false;
}

int32 URageComboRow::GetSelectedIndex() const
{
	return ComboBox->GetSelectedIndex();
}

void URageComboRow::NativeConstruct()
{
	Super::NativeConstruct();

	ComboBox->OnSelectionChanged.AddUniqueDynamic(this, &URageComboRow::HandleSelectionChanged);

	CultureChangedHandle = FInternationalization::Get().OnCultureChanged().AddUObject(this, &URageComboRow::HandleCultureChanged);
}

void URageComboRow::NativeDestruct()
{
	FInternationalization::Get().OnCultureChanged().Remove(CultureChangedHandle);
	CultureChangedHandle.Reset();

	Super::NativeDestruct();
}

void URageComboRow::HandleSelectionChanged(FString SelectedItem, ESelectInfo::Type SelectionType)
{
	if (bSuppressNotify)
	{
		return;
	}

	ValueChangedDelegate.Broadcast(RowId, VAL(int32, ComboBox->GetSelectedIndex()));
}

void URageComboRow::HandleCultureChanged()
{
	/* Text sitting in a text block retranslates itself, but the strings handed to the box are copies
	 * taken in the language of the moment, so the box is the one thing here that has to be rebuilt.
	 * Rows given plain strings have nothing to rebuild from and are left as they are. */
	if (!OptionTexts.IsEmpty())
	{
		RefreshOptionStrings();
	}
}

void URageComboRow::RefreshOptionStrings()
{
	TArray<FString> Strings;
	Strings.Reserve(OptionTexts.Num());
	for (const FText& Option : OptionTexts)
	{
		Strings.Add(Option.ToString());
	}

	ApplyOptionStrings(Strings);
}

void URageComboRow::ApplyOptionStrings(const TArray<FString>& NewOptions)
{
	const int32 PreviousIndex = ComboBox->GetSelectedIndex();

	/* Emptying a live box clears its selection, and a cleared selection reports itself as a change the
	 * player made, at an index that is no longer in any list. Nothing between here and the reselect
	 * below is the player choosing anything. */
	bSuppressNotify = true;
	ComboBox->ClearOptions();

	for (const FString& Option : NewOptions)
	{
		ComboBox->AddOption(Option);
	}
	bSuppressNotify = false;

	if (NewOptions.Num() == 1)
	{
		ComboBox->SetIsEnabled(false);
	}

	/* The same choices under new names, or a fresh list the caller is about to select into itself. */
	if (PreviousIndex != INDEX_NONE && NewOptions.IsValidIndex(PreviousIndex))
	{
		SetSelectedIndex(PreviousIndex);
	}
}
