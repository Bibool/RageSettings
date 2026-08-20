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
}

void URageSliderRow::SetDisplayFormat(ERageSliderDisplayFormat NewFormat)
{
	DisplayFormat = NewFormat;
	
	const bool bStepped = NewFormat == ERageSliderDisplayFormat::Integer;
	Slider->SetStepSize(bStepped ? 1.f : 0.01f);
	Slider->MouseUsesStep = bStepped;
	Slider->SynchronizeProperties();
	RefreshValueText(Slider->GetValue());	
}

void URageSliderRow::SetValue(float NewValue, bool bNotify)
{
	bSuppressNotify = !bNotify;
	Slider->SetValue(NewValue);
	RefreshValueText(NewValue);
	bSuppressNotify = false;
}

float URageSliderRow::GetValue() const
{
	return Slider->GetValue();
}

void URageSliderRow::NativeConstruct()
{
	Super::NativeConstruct();
	
	Slider->OnValueChanged.AddUniqueDynamic(this, &URageSliderRow::HandleSliderValueChanged);
}

void URageSliderRow::HandleSliderValueChanged(float NewValue)
{
	RefreshValueText(NewValue);
	
	if (bSuppressNotify)
	{
		return;
	}
	
	ValueChangedDelegate.Broadcast(RowId, VAL(float, NewValue));
}

void URageSliderRow::RefreshValueText_Implementation(float Value)
{
	FText DesiredText;
	
	switch (DisplayFormat)
	{
		case ERageSliderDisplayFormat::Percent:
			DesiredText = FText::AsPercent(Value);
			break;
		case ERageSliderDisplayFormat::Multiplier:
			DesiredText = FText::FromString(FString::Printf(TEXT("%.2fx"), Value));
			break;
		case ERageSliderDisplayFormat::Integer:
			DesiredText = FText::AsNumber(FMath::RoundToInt(Value));
			break;
		case ERageSliderDisplayFormat::Raw:
		default:
			DesiredText = FText::FromString(FString::Printf(TEXT("%.2f"), Value));
			break;
	}
	
	if (IsValid(ValueText))
	{
		ValueText->SetText(DesiredText);
	}

	OnValueTextSet(DesiredText);
}
