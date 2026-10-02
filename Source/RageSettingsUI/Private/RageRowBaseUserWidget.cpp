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

const FText& URageRowBaseUserWidget::GetRowLabel() const
{
	return LabelText;
}

void URageRowBaseUserWidget::SetHint(const FText& NewHint)
{
	HintText = NewHint;
	NativeOnTextsChanged();
	PresentationChangedDelegate.Broadcast(this);
}

const FText& URageRowBaseUserWidget::GetHintText() const
{
	return HintText;
}

void URageRowBaseUserWidget::SetDescription(const FText& NewDescription)
{
	DescriptionText = NewDescription;
	NativeOnTextsChanged();
	PresentationChangedDelegate.Broadcast(this);
}

const FText& URageRowBaseUserWidget::GetDescriptionText() const
{
	return DescriptionText;
}

void URageRowBaseUserWidget::SetRowEnabled(bool bEnabled, const FText& InDisabledReason)
{
	DisabledReason = bEnabled ? FText::GetEmpty() : InDisabledReason;

	SetIsEnabled(bEnabled);
	RefreshLabel();

	NativeOnRowEnabledChanged(bEnabled);
	PresentationChangedDelegate.Broadcast(this);
}

void URageRowBaseUserWidget::SetRowId(FName NewRowId)
{
	RowId = NewRowId;
}

FName URageRowBaseUserWidget::GetRowId() const
{
	return RowId;
}

void URageRowBaseUserWidget::CaptureBaseline()
{
	if (!IsSettingRow())
	{
		return;
	}

	BaselineKey = GetValueKey();
	BaselineText = GetValueText();
	bHasBaseline = true;

	RefreshModified();
}

bool URageRowBaseUserWidget::IsModified() const
{
	return bIsModified;
}

bool URageRowBaseUserWidget::IsSettingRow() const
{
	return !GetValueKey().IsEmpty();
}

FText URageRowBaseUserWidget::GetValueDisplayText() const
{
	return GetValueText();
}

FText URageRowBaseUserWidget::GetBaselineDisplayText() const
{
	return BaselineText;
}

bool URageRowBaseUserWidget::IsRowHovered() const
{
	return bRowHovered;
}

bool URageRowBaseUserWidget::IsRowFocused() const
{
	return bRowFocused;
}

void URageRowBaseUserWidget::NativePreConstruct()
{
	Super::NativePreConstruct();

	if (!LabelText.IsEmpty())
	{
		SetLabel(LabelText);
	}
	else
	{
		NativeOnTextsChanged();
	}
}

void URageRowBaseUserWidget::NativeOnMouseEnter(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	Super::NativeOnMouseEnter(InGeometry, InMouseEvent);

	bRowHovered = true;
	PresentationChangedDelegate.Broadcast(this);

	if (IsSettingRow())
	{
		HighlightedDelegate.Broadcast(this);
	}
}

void URageRowBaseUserWidget::NativeOnMouseLeave(const FPointerEvent& InMouseEvent)
{
	Super::NativeOnMouseLeave(InMouseEvent);

	bRowHovered = false;
	PresentationChangedDelegate.Broadcast(this);
}

void URageRowBaseUserWidget::NativeOnAddedToFocusPath(const FFocusEvent& InFocusEvent)
{
	Super::NativeOnAddedToFocusPath(InFocusEvent);

	bRowFocused = true;
	PresentationChangedDelegate.Broadcast(this);

	if (IsSettingRow())
	{
		HighlightedDelegate.Broadcast(this);
	}
}

void URageRowBaseUserWidget::NativeOnRemovedFromFocusPath(const FFocusEvent& InFocusEvent)
{
	Super::NativeOnRemovedFromFocusPath(InFocusEvent);

	bRowFocused = false;
	PresentationChangedDelegate.Broadcast(this);
}

void URageRowBaseUserWidget::NativeOnRowEnabledChanged(bool bEnabled)
{
}

void URageRowBaseUserWidget::NativeOnTextsChanged()
{
}

void URageRowBaseUserWidget::NativeOnModifiedChanged(bool bNewModified)
{
}

FString URageRowBaseUserWidget::GetValueKey() const
{
	return FString();
}

FText URageRowBaseUserWidget::GetValueText() const
{
	return FText::GetEmpty();
}

void URageRowBaseUserWidget::RefreshModified()
{
	const bool bNewModified = bHasBaseline && GetValueKey() != BaselineKey;
	if (bNewModified == bIsModified)
	{
		return;
	}

	bIsModified = bNewModified;

	NativeOnModifiedChanged(bIsModified);
	OnModifiedChanged(bIsModified);
	ModifiedChangedDelegate.Broadcast(this);
	PresentationChangedDelegate.Broadcast(this);
}

void URageRowBaseUserWidget::RefreshLabel()
{
	const FText DesiredText = DisabledReason.IsEmpty() ? LabelText : FText::Format(DisabledReasonFormat, LabelText, DisabledReason);

	if (IsValid(Label))
	{
		Label->SetText(DesiredText);
	}

	OnLabelTextSet(DesiredText);
	NativeOnTextsChanged();
	PresentationChangedDelegate.Broadcast(this);
}
