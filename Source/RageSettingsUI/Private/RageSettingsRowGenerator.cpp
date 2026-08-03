// Copyright (c) 2026 Abdallah Boutrif
// SPDX-License-Identifier: MIT

#include "RageSettingsRowGenerator.h"

#include "Algo/Reverse.h"
#include "UObject/EnumProperty.h"
#include "UObject/UnrealType.h"

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
		return ERowKind::Selection;
	}
	
	if (const FByteProperty* ByteProperty = CastField<FByteProperty>(Property))
	{
		return IsValid(ByteProperty->Enum) ? ERowKind::Selection : ERowKind::Slider;
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
	if (!IsValid(Enum) || !Enum->IsValidEnumValue(Enum->GetValueByIndex(Index)))
	{
		return;
	}

	const int64 Value = Enum->GetValueByIndex(Index);

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
