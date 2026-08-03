// Copyright (c) 2026 Abdallah Boutrif
// SPDX-License-Identifier: MIT

#include "RageRowBaseUserWidget.h"

#include "Components/TextBlock.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(RageRowBaseUserWidget)

void URageRowBaseUserWidget::SetLabel(const FText& NewLabel)
{
	Label->SetText(NewLabel);
}

void URageRowBaseUserWidget::SetRowId(FName NewRowId)
{
	RowId = NewRowId;
}

FName URageRowBaseUserWidget::GetRowId() const
{
	return RowId;
}

void URageRowBaseUserWidget::NativePreConstruct()
{
	Super::NativePreConstruct();
	
	if (!LabelText.IsEmpty())
	{
		SetLabel(LabelText);
	}
}
