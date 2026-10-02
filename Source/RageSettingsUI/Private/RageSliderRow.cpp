// Copyright (c) 2026 Abdallah Boutrif
// SPDX-License-Identifier: MIT

#include "RageSliderRow.h"

#include "Components/Slider.h"
#include "Components/TextBlock.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(RageSliderRow)

void URageSliderRow::SetRange(float NewMin, float NewMax)
{
	Slider->SetMinValue(NewMin);
	Slider->SetMaxValue(NewMax);

	NativeOnRangeChanged();
}

void URageSliderRow::SetDisplayFormat(ERageSliderDisplayFormat NewFormat)
{
	DisplayFormat = NewFormat;

	const bool bStepped = NewFormat == ERageSliderDisplayFormat::Integer;
	Slider->SetStepSize(bStepped ? 1.f : 0.01f);
	Slider->MouseUsesStep = bStepped;
	Slider->SynchronizeProperties();
	RefreshValueText(Slider->GetValue());

	NativeOnRangeChanged();
}

void URageSliderRow::SetValue(float NewValue, bool bNotify)
{
	bSuppressNotify = !bNotify;
	Slider->SetValue(NewValue);
	RefreshValueText(NewValue);
	bSuppressNotify = false;

	RefreshModified();
}

float URageSliderRow::GetValue() const
{
	return Slider->GetValue();
}

float URageSliderRow::GetMinValue() const
{
	return Slider->GetMinValue();
}

float URageSliderRow::GetMaxValue() const
{
	return Slider->GetMaxValue();
}

FText URageSliderRow::FormatValue(const float Value) const
{
	return NativeFormatValue(Value);
}

void URageSliderRow::NativeConstruct()
{
	Super::NativeConstruct();

	Slider->OnValueChanged.AddUniqueDynamic(this, &URageSliderRow::HandleSliderValueChanged);
}

FString URageSliderRow::GetValueKey() const
{
	/* Compared through the readout's own rounding, so a drag that lands back on the shown number counts as unchanged. */
	return IsValid(Slider) ? NativeFormatValue(Slider->GetValue()).ToString() : FString();
}

FText URageSliderRow::GetValueText() const
{
	return IsValid(Slider) ? NativeFormatValue(Slider->GetValue()) : FText::GetEmpty();
}

void URageSliderRow::NativeOnRangeChanged()
{
}

FText URageSliderRow::NativeFormatValue(const float Value) const
{
	switch (DisplayFormat)
	{
		case ERageSliderDisplayFormat::Percent:
			return FText::AsPercent(Value);
		case ERageSliderDisplayFormat::Multiplier:
			return FText::FromString(FString::Printf(TEXT("%.2fx"), Value));
		case ERageSliderDisplayFormat::Integer:
			return FText::AsNumber(FMath::RoundToInt(Value));
		case ERageSliderDisplayFormat::Raw:
		default:
			return FText::FromString(FString::Printf(TEXT("%.2f"), Value));
	}
}

void URageSliderRow::HandleSliderValueChanged(float NewValue)
{
	RefreshValueText(NewValue);
	RefreshModified();

	if (bSuppressNotify)
	{
		return;
	}

	ValueChangedDelegate.Broadcast(RowId, VAL(float, NewValue));
}

void URageSliderRow::RefreshValueText_Implementation(float Value)
{
	const FText DesiredText = NativeFormatValue(Value);

	if (IsValid(ValueText))
	{
		ValueText->SetText(DesiredText);
	}

	OnValueTextSet(DesiredText);
}
