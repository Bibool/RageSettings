// Copyright (c) 2026 Abdallah Boutrif
// SPDX-License-Identifier: MIT

#include "RageComboRow.h"

#include "Components/ComboBoxString.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(RageComboRow)

void URageComboRow::SetOptions(const TArray<FString>& NewOptions)
{
	ComboBox->ClearOptions();
	
	for (const FString& Option : NewOptions)
	{
		ComboBox->AddOption(Option);
	}
	
	if (NewOptions.Num() == 1)
	{
		ComboBox->SetIsEnabled(false);
	}
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
}

void URageComboRow::HandleSelectionChanged(FString SelectedItem, ESelectInfo::Type SelectionType)
{
	if (bSuppressNotify)
	{
		return;
	}
	
	ValueChangedDelegate.Broadcast(RowId, VAL(int32, ComboBox->GetSelectedIndex()));
}
