// Copyright (c) 2026 Abdallah Boutrif
// SPDX-License-Identifier: MIT

#pragma once

#include "CoreMinimal.h"
#include "RageSliderDisplayFormat.h"

class UEnum;
class URageRowBaseUserWidget;

struct FRageSettingsRowDescriptor
{
	FName PropertyName = NAME_None;
	FText Label = FText::GetEmpty();
	FText Hint = FText::GetEmpty();
	FText Description = FText::GetEmpty();
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

	/** The text a generated row is labelled with, resolved in the order a project can override it:
	 * the panel's own descriptor, then the row override's DesiredLabel, then a string table entry
	 * keyed by the property name, and only then a display string derived from that name.
	 *
	 * The string tables are the only step that survives cooking, since a UPROPERTY's metadata is
	 * editor-only, so a field that has to read in more than one language needs an entry in one. */
	RAGESETTINGSUI_API FText ResolveRowLabel(const FProperty* Property, const FRageSettingsRowDescriptor* Descriptor);

	/** The line under a generated row's label and the longer text a details pane shows for it, looked up
	 * like the label but against the keys <Property>_Hint and <Property>_Description, and never derived:
	 * a row nobody wrote a hint for has none. */
	RAGESETTINGSUI_API FText ResolveRowHint(const FProperty* Property, const FRageSettingsRowDescriptor* Descriptor);
	RAGESETTINGSUI_API FText ResolveRowDescription(const FProperty* Property, const FRageSettingsRowDescriptor* Descriptor);

	/** Gives a hand-built row the id Key, and the hint and description a string table keeps under
	 * Key_Hint and Key_Description. Whatever the row was authored with stays when no table has them. */
	RAGESETTINGSUI_API void ApplyRowKey(URageRowBaseUserWidget* Row, FName Key);

	/** The option labels for an enum-backed row, one per entry and in the order the row shows them,
	 * so an index into this array is an index into the enum. @see ResolveRowLabel for the lookup. */
	RAGESETTINGSUI_API TArray<FText> BuildEnumOptionLabels(const FProperty* Property);

	/** The entry Key resolves to in the plugin's string table, or in one of the project's own
	 * (URageSettingsUIDeveloperSettings::AdditionalStringTables), and empty text when no table
	 * carries it. Loads a table that is not in memory yet, which is why this belongs to row building
	 * rather than to anything that runs per frame. */
	RAGESETTINGSUI_API FText FindStringTableText(const FString& Key);
	
	RAGESETTINGSUI_API double GetNumericPropertyValue(const FProperty* Property, const UObject* Container);
	RAGESETTINGSUI_API void SetNumericPropertyValue(const FProperty* Property, UObject* Container, double NewValue);
	RAGESETTINGSUI_API int32 GetEnumPropertyIndex(const FProperty* Property, const UObject* Container);
	RAGESETTINGSUI_API void SetEnumPropertyByIndex(const FProperty* Property, UObject* Container, int32 Index);
}
