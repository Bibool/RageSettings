// Copyright (c) 2026 Abdallah Boutrif
// SPDX-License-Identifier: MIT

#pragma once

#include "CoreMinimal.h"
#include "RageSliderDisplayFormat.h"

class UEnum;

struct FRageSettingsRowDescriptor
{
	FName PropertyName = NAME_None;
	FText Label = FText::GetEmpty();
	float ClampMin = 0.f;
	float ClampMax = 1.f;
	ERageSliderDisplayFormat SliderFormat = ERageSliderDisplayFormat::Raw;
};

namespace RageSettingsUI
{
	enum class ERowKind : uint8
	{
		Toggle,
		Slider,
		Combo,
		Selection,
		Unsupported
	};
	
	RAGESETTINGSUI_API TArray<FProperty*> CollectRowProperties(UClass* Class);
	RAGESETTINGSUI_API ERowKind ResolveRowKind(const FProperty* Property);
	RAGESETTINGSUI_API FText DeriveDefaultLabel(const FProperty* Property);
	RAGESETTINGSUI_API FText DeriveDefaultEnumValueLabel(const UEnum* Enum, int64 Value);
	RAGESETTINGSUI_API const UEnum* ResolveEnum(const FProperty* Property);
	RAGESETTINGSUI_API double GetNumericPropertyValue(const FProperty* Property, const UObject* Container);
	RAGESETTINGSUI_API void SetNumericPropertyValue(const FProperty* Property, UObject* Container, double NewValue);
	RAGESETTINGSUI_API int32 GetEnumPropertyIndex(const FProperty* Property, const UObject* Container);
	RAGESETTINGSUI_API void SetEnumPropertyByIndex(const FProperty* Property, UObject* Container, int32 Index);
}
