// Copyright (c) 2026 Abdallah Boutrif
// SPDX-License-Identifier: MIT

#include "RageSettingsRowGenerator.h"

#include "RageComboRow.h"
#include "RageSelectionRow.h"
#include "RageSettingsUIDeveloperSettings.h"
#include "Algo/Reverse.h"
#include "Internationalization/StringTableCore.h"
#include "Internationalization/StringTableRegistry.h"
#include "UObject/EnumProperty.h"
#include "UObject/UnrealType.h"

namespace
{
	/* The table the plugin ships, searched before any the project adds so a project table extends this
	 * one instead of having to restate it. */
	const FName PluginStringTableId = TEXT("/RageSettings/ST_Settings.ST_Settings");

	const FRageRowOverrideData* FindRowOverride(const FProperty* Property)
	{
		return Property ? SETTINGS_UI->RowWidgetClassOverrides.Find(Property->GetFName()) : nullptr;
	}

	bool TryFindStringTableText(const FName TableId, const FString& Key, FText& OutText)
	{
		if (TableId.IsNone() || !IStringTableEngineBridge::CanFindOrLoadStringTableAsset())
		{
			return false;
		}

		/* A table nobody has asked for yet is not in the registry, and asking the registry on its own
		 * would report every one of its keys missing. */
		FName ResolvedTableId = TableId;
		IStringTableEngineBridge::FullyLoadStringTableAsset(ResolvedTableId);

		const FStringTableConstPtr StringTable = FStringTableRegistry::Get().FindStringTable(ResolvedTableId);
		if (!StringTable.IsValid() || !StringTable->FindEntry(Key).IsValid())
		{
			return false;
		}

		OutText = FText::FromStringTable(ResolvedTableId, Key);
		return true;
	}

	/** Which of the two enum rows a property wants. An override naming a row Blueprint has already said
	 * so outright, so it decides, and only a property with nothing said about it falls back to the
	 * project wide default. Answering it here rather than where the widget is spawned is what stops an
	 * override pointing at a combo Blueprint from being measured against the selection row base and
	 * quietly dropped. */
	RageSettingsUI::ERowKind ResolveEnumRowKind(const FProperty* Property)
	{
		if (const FRageRowOverrideData* Override = FindRowOverride(Property))
		{
			if (const UClass* OverrideClass = Override->WidgetClass)
			{
				if (OverrideClass->IsChildOf(URageComboRow::StaticClass()))
				{
					return RageSettingsUI::ERowKind::Combo;
				}

				if (OverrideClass->IsChildOf(URageSelectionRow::StaticClass()))
				{
					return RageSettingsUI::ERowKind::Selection;
				}
			}
		}

		return SETTINGS_UI->bEnumSettingsUsesComboRow ? RageSettingsUI::ERowKind::Combo : RageSettingsUI::ERowKind::Selection;
	}

	FText ResolveEnumValueLabel(const UEnum* Enum, const int32 Index, const TMap<FName, FText>* OptionLabels)
	{
		const FString ValueName = Enum->GetAuthoredNameStringByIndex(Index);

		if (OptionLabels)
		{
			if (const FText* Override = OptionLabels->Find(FName(*ValueName)))
			{
				if (!Override->IsEmpty())
				{
					return *Override;
				}
			}
		}

		/* Scoped key first, so two enums that both carry a "None" or a "Custom" can be translated apart,
		 * then the bare value name the hand-authored panels already key on. @see ResolveLocTextForEnum. */
		const FText Scoped = RageSettingsUI::FindStringTableText(FString::Printf(TEXT("%s_%s"), *Enum->GetName(), *ValueName));
		if (!Scoped.IsEmpty())
		{
			return Scoped;
		}

		const FText Bare = RageSettingsUI::FindStringTableText(ValueName);
		if (!Bare.IsEmpty())
		{
			return Bare;
		}

		return RageSettingsUI::DeriveDefaultEnumValueLabel(Enum, Enum->GetValueByIndex(Index));
	}
}

TArray<FProperty*> RageSettingsUI::CollectRowProperties(UClass* Class)
{
	TArray<UClass*> Chain;
	for (UClass* Current = Class; Current; Current = Current->GetSuperClass())
	{
		Chain.Add(Current);
	}

	/* Plugin fields list before project-subclass fields */
	Algo::Reverse(Chain);

	TArray<FProperty*> Result;
	for (const UClass* Current : Chain)
	{
		for (TFieldIterator<FProperty> It(Current, EFieldIteratorFlags::ExcludeSuper); It; ++It)
		{
			FProperty* Property = *It;
			if (Property->HasAllPropertyFlags(CPF_Config | CPF_Edit))
			{
				Result.Add(Property);
			}
		}
	}

	return Result;
}

RageSettingsUI::ERowKind RageSettingsUI::ResolveRowKind(const FProperty* Property)
{
	if (Property->IsA<FBoolProperty>())
	{
		return ERowKind::Toggle;
	}

	if (Property->IsA<FEnumProperty>())
	{
		return ResolveEnumRowKind(Property);
	}

	if (const FByteProperty* ByteProperty = CastField<FByteProperty>(Property))
	{
		return IsValid(ByteProperty->Enum) ? ResolveEnumRowKind(Property) : ERowKind::Slider;
	}

	if (Property->IsA<FFloatProperty>() || Property->IsA<FDoubleProperty>() || Property->IsA<FIntProperty>() || Property->IsA<FInt64Property>())
	{
		return ERowKind::Slider;
	}

	return ERowKind::Unsupported;
}

FText RageSettingsUI::DeriveDefaultLabel(const FProperty* Property)
{
	return FText::FromString(FName::NameToDisplayString(Property->GetName(), Property->IsA<FBoolProperty>()));
}

FText RageSettingsUI::DeriveDefaultEnumValueLabel(const UEnum* Enum, int64 Value)
{
	if (!IsValid(Enum))
	{
		return FText::GetEmpty();
	}

	return FText::FromString(FName::NameToDisplayString(Enum->GetNameStringByValue(Value), false));
}

FText RageSettingsUI::FindStringTableText(const FString& Key)
{
	FText Found;
	if (TryFindStringTableText(PluginStringTableId, Key, Found))
	{
		return Found;
	}

	for (const FName& TableId : SETTINGS_UI->AdditionalStringTables)
	{
		if (TryFindStringTableText(TableId, Key, Found))
		{
			return Found;
		}
	}

	return FText::GetEmpty();
}

FText RageSettingsUI::ResolveRowLabel(const FProperty* Property, const FRageSettingsRowDescriptor* Descriptor)
{
	if (Descriptor && !Descriptor->Label.IsEmpty())
	{
		return Descriptor->Label;
	}

	if (const FRageRowOverrideData* Override = FindRowOverride(Property))
	{
		if (!Override->DesiredLabel.IsEmpty())
		{
			return Override->DesiredLabel;
		}
	}

	if (!Property)
	{
		return FText::GetEmpty();
	}

	const FText FromTable = FindStringTableText(Property->GetName());
	return FromTable.IsEmpty() ? DeriveDefaultLabel(Property) : FromTable;
}

TArray<FText> RageSettingsUI::BuildEnumOptionLabels(const FProperty* Property)
{
	TArray<FText> Options;

	const UEnum* Enum = ResolveEnum(Property);
	if (!IsValid(Enum))
	{
		return Options;
	}

	const FRageRowOverrideData* Override = FindRowOverride(Property);
	const TMap<FName, FText>* OptionLabels = Override ? &Override->OptionLabels : nullptr;

	const int32 Count = Enum->NumEnums() - (Enum->ContainsExistingMax() ? 1 : 0);
	Options.Reserve(Count);
	for (int32 Index = 0; Index < Count; ++Index)
	{
		Options.Add(ResolveEnumValueLabel(Enum, Index, OptionLabels));
	}

	return Options;
}

const UEnum* RageSettingsUI::ResolveEnum(const FProperty* Property)
{
	if (const FEnumProperty* EnumProperty = CastField<FEnumProperty>(Property))
	{
		return EnumProperty->GetEnum();
	}
	if (const FByteProperty* ByteProperty = CastField<FByteProperty>(Property))
	{
		return ByteProperty->Enum;
	}
	return nullptr;
}

double RageSettingsUI::GetNumericPropertyValue(const FProperty* Property, const UObject* Container)
{
	const FNumericProperty* NumericProperty = CastField<FNumericProperty>(Property);
	if (!NumericProperty)
	{
		return 0.0;
	}

	const void* ValuePtr = NumericProperty->ContainerPtrToValuePtr<void>(Container);
	return NumericProperty->IsFloatingPoint() ? NumericProperty->GetFloatingPointPropertyValue(ValuePtr) : StaticCast<double>(NumericProperty->GetSignedIntPropertyValue(ValuePtr));
}

void RageSettingsUI::SetNumericPropertyValue(const FProperty* Property, UObject* Container, double NewValue)
{
	const FNumericProperty* NumericProperty = CastField<FNumericProperty>(Property);
	if (!NumericProperty)
	{
		return;
	}

	void* ValuePtr = NumericProperty->ContainerPtrToValuePtr<void>(Container);
	if (NumericProperty->IsFloatingPoint())
	{
		NumericProperty->SetFloatingPointPropertyValue(ValuePtr, NewValue);
	}
	else
	{
		NumericProperty->SetIntPropertyValue(ValuePtr, StaticCast<int64>(NewValue));
	}
}

int32 RageSettingsUI::GetEnumPropertyIndex(const FProperty* Property, const UObject* Container)
{
	const UEnum* Enum = ResolveEnum(Property);
	if (!IsValid(Enum))
	{
		return INDEX_NONE;
	}

	int64 Value = 0;
	if (const FEnumProperty* EnumProperty = CastField<FEnumProperty>(Property))
	{
		const void* ValuePtr = EnumProperty->ContainerPtrToValuePtr<void>(Container);
		Value = EnumProperty->GetUnderlyingProperty()->GetSignedIntPropertyValue(ValuePtr);
	}
	else if (const FByteProperty* ByteProperty = CastField<FByteProperty>(Property))
	{
		Value = ByteProperty->GetPropertyValue_InContainer(Container);
	}

	return Enum->GetIndexByValue(Value);
}

void RageSettingsUI::SetEnumPropertyByIndex(const FProperty* Property, UObject* Container, int32 Index)
{
	const UEnum* Enum = ResolveEnum(Property);

	/* Bounds before value: asking an enum for the value at an index it does not have asserts, and a row
	 * whose list was emptied under it reports its selection as index none. */
	if (!IsValid(Enum) || Index < 0 || Index >= Enum->NumEnums())
	{
		return;
	}

	const int64 Value = Enum->GetValueByIndex(Index);
	if (!Enum->IsValidEnumValue(Value))
	{
		return;
	}

	if (const FEnumProperty* EnumProperty = CastField<FEnumProperty>(Property))
	{
		void* ValuePtr = EnumProperty->ContainerPtrToValuePtr<void>(Container);
		EnumProperty->GetUnderlyingProperty()->SetIntPropertyValue(ValuePtr, Value);
	}
	else if (const FByteProperty* ByteProperty = CastField<FByteProperty>(Property))
	{
		ByteProperty->SetPropertyValue_InContainer(Container, StaticCast<uint8>(Value));
	}
}
