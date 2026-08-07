// Copyright (c) 2026 Abdallah Boutrif
// SPDX-License-Identifier: MIT

#include "RageRowBaseUserWidget.h"

#include "Components/TextBlock.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(RageRowBaseUserWidget)

void URageRowBaseUserWidget::SetLabel(const FText& NewLabel)
{
	LabelText = NewLabel;
	RefreshLabel();
}

void URageRowBaseUserWidget::SetRowEnabled(bool bEnabled, const FText& InDisabledReason)
{
	DisabledReason = bEnabled ? FText::GetEmpty() : InDisabledReason;

	SetIsEnabled(bEnabled);
	RefreshLabel();
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

void URageRowBaseUserWidget::RefreshLabel()
{
	if (!IsValid(Label))
	{
		return;
	}

	Label->SetText(DisabledReason.IsEmpty() ? LabelText : FText::Format(DisabledReasonFormat, LabelText, DisabledReason));
}
