// Copyright (c) 2026 Abdallah Boutrif
// SPDX-License-Identifier: MIT

#include "RageToggleRow.h"

#include "Components/CheckBox.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(RageToggleRow)

void URageToggleRow::SetValue(bool bChecked, bool bNotify)
{
	bSuppressNotify = !bNotify;
	CheckBox->SetIsChecked(bChecked);
	bSuppressNotify = false;
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

void URageToggleRow::HandleCheckStateChanged(bool bIsChecked)
{
	if (bSuppressNotify)
	{
		return;
	}
	
	ValueChangedDelegate.Broadcast(RowId, VAL(bool, bIsChecked));
}
