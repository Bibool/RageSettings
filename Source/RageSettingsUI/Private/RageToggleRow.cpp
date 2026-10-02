// Copyright (c) 2026 Abdallah Boutrif
// SPDX-License-Identifier: MIT

#include "RageToggleRow.h"

#include "RageMacros.h"
#include "Components/CheckBox.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(RageToggleRow)

void URageToggleRow::SetValue(bool bChecked, bool bNotify)
{
	bSuppressNotify = !bNotify;
	CheckBox->SetIsChecked(bChecked);
	bSuppressNotify = false;

	NativeOnToggleChanged(bChecked);
}

bool URageToggleRow::GetValue() const
{
	return CheckBox->IsChecked();
}

void URageToggleRow::NativeConstruct()
{
	Super::NativeConstruct();
	
	CheckBox->OnCheckStateChanged.AddUniqueDynamic(this, &URageToggleRow::HandleCheckStateChanged);
}

void URageToggleRow::NativeOnToggleChanged(bool bChecked)
{
	RefreshModified();
}

FString URageToggleRow::GetValueKey() const
{
	return IsValid(CheckBox) && CheckBox->IsChecked() ? TEXT("1") : TEXT("0");
}

FText URageToggleRow::GetValueText() const
{
	return IsValid(CheckBox) && CheckBox->IsChecked() ? RAGE_LOC("On") : RAGE_LOC("Off");
}

void URageToggleRow::HandleCheckStateChanged(bool bIsChecked)
{
	NativeOnToggleChanged(bIsChecked);

	if (bSuppressNotify)
	{
		return;
	}
	
	ValueChangedDelegate.Broadcast(RowId, VAL(bool, bIsChecked));
}
