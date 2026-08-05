// Copyright (c) 2026 Abdallah Boutrif
// SPDX-License-Identifier: MIT

#include "RageSettingsRowGeneratorPanelBase.h"

#include "RageComboRow.h"
#include "RageMacros.h"
#include "RageRowOverrideObject.h"
#include "RageSelectionRow.h"
#include "Components/PanelWidget.h"
#include "RageSettingsCategoryInterface.h"
#include "RageSettingsSharedDebug.h"
#include "RageSettingsUIDeveloperSettings.h"
#include "RageSliderRow.h"
#include "RageToggleRow.h"
#include "Internationalization/StringTableCore.h"
#include "Internationalization/StringTableRegistry.h"
#include "UObject/UnrealType.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(RageSettingsRowGeneratorPanelBase)

namespace
{
	UClass* ResolveRowWidgetClass(const FProperty* Property, const UClass* ExpectedBase, UClass* ProjectDefault)
	{
		const URageSettingsUIDeveloperSettings* UISettings = URageSettingsUIDeveloperSettings::Get();
		if (const FRageRowOverrideData* Override = UISettings->RowWidgetClassOverrides.Find(Property->GetFName()))
		{
			if (UClass* OverrideClass = Override->WidgetClass)
			{
				if (OverrideClass->IsChildOf(ExpectedBase))
				{
					return OverrideClass;
				}
				
				S_LOG(Warning, "Rage Settings : RowWidgetClassOverrides[{property}] ({class}) doesn't derive from {base} - ignoring, falling back to the project defaults.",
					*Property->GetName(), *OverrideClass->GetName(), *ExpectedBase->GetName());
			}
		}

		return ProjectDefault;
	}
	
	FText ResolveLabel(const FProperty* Property, const FRageSettingsRowDescriptor* Descriptor)
	{
		if (Descriptor && !Descriptor->Label.IsEmpty())
		{
			return Descriptor->Label;
		}
		
		const URageSettingsUIDeveloperSettings* UISettings = URageSettingsUIDeveloperSettings::Get();
		if (const FRageRowOverrideData* Override = UISettings->RowWidgetClassOverrides.Find(Property->GetFName()))
		{
			if (!Override->DesiredLabel.IsEmpty())
			{
				return Override->DesiredLabel;
			}
		}
		
		if (Property)
		{
			const FString PropertyStr = Property->GetFName().ToString();
			const FStringTableConstPtr StringTable = FStringTableRegistry::Get().FindStringTable(TEXT("/RageSettings/ST_Settings.ST_Settings"));
			if (StringTable.IsValid() && StringTable->FindEntry(PropertyStr))
			{
				return RAGE_LOC_Str(PropertyStr);
			}
			
			return RageSettingsUI::DeriveDefaultLabel(Property);
		}
		
		return FText::GetEmpty();
	}
	
	void TryOverrideObjectWidgetManipulation(const FProperty* Property, URageRowBaseUserWidget* InWidget)
	{
		const URageSettingsUIDeveloperSettings* UISettings = URageSettingsUIDeveloperSettings::Get();
		if (const FRageRowOverrideData* Override = UISettings->RowWidgetClassOverrides.Find(Property->GetFName()))
		{
			if (IsValid(Override->OverrideObject))
			{
				Override->OverrideObject->ManipulateGeneratedWidget(InWidget);
			}
		}
	}
}

void URageSettingsRowGeneratorPanelBase::BuildRows(UPanelWidget* Container, UObject* PendingObject)
{
	BoundPendingObject = PendingObject;

	if (!IsValid(Container) || !IsValid(PendingObject))
	{
		return;
	}

	Container->ClearChildren();

	const TArray<FRageSettingsRowDescriptor> Descriptors = GetRowDescriptors();
	const URageSettingsUIDeveloperSettings* UISettings = URageSettingsUIDeveloperSettings::Get();

	for (FProperty* Property : RageSettingsUI::CollectRowProperties(PendingObject->GetClass()))
	{
		const RageSettingsUI::ERowKind Kind = RageSettingsUI::ResolveRowKind(Property);
		if (Kind == RageSettingsUI::ERowKind::Unsupported)
		{
			continue;
		}

		const FRageSettingsRowDescriptor* Descriptor = Descriptors.FindByPredicate(
			[Property](const FRageSettingsRowDescriptor& Candidate) { return Candidate.PropertyName == Property->GetFName(); });

		const FText Label = ResolveLabel(Property, Descriptor);

		switch (Kind)
		{
			case RageSettingsUI::ERowKind::Toggle:
			{
				TSubclassOf<URageToggleRow> RowClass = ResolveRowWidgetClass(Property, URageToggleRow::StaticClass(), UISettings->DefaultToggleRowClass.Get());
				if (URageToggleRow* Row = CreateWidget<URageToggleRow>(this, RowClass))
				{
					Row->SetLabel(Label);
					Row->SetRowId(Property->GetFName());
					Row->ValueChangedDelegate.AddUObject(this, &URageSettingsRowGeneratorPanelBase::HandleGeneratedToggleChanged);
					TryOverrideObjectWidgetManipulation(Property, Row);
					Container->AddChild(Row);
				}
				break;
			}
			case RageSettingsUI::ERowKind::Slider:
			{
				TSubclassOf<URageSliderRow> RowClass = ResolveRowWidgetClass(Property, URageSliderRow::StaticClass(), UISettings->DefaultSliderRowClass.Get());
				if (URageSliderRow* Row = CreateWidget<URageSliderRow>(this, RowClass))
				{
					Row->SetLabel(Label);
					Row->SetRowId(Property->GetFName());
					if (Descriptor)
					{
						Row->SetRange(Descriptor->ClampMin, Descriptor->ClampMax);
						Row->SetDisplayFormat(Descriptor->SliderFormat);
					}
					Row->ValueChangedDelegate.AddUObject(this, &URageSettingsRowGeneratorPanelBase::HandleGeneratedSliderChanged);
					TryOverrideObjectWidgetManipulation(Property, Row);
					Container->AddChild(Row);
				}
				break;
			}
			case RageSettingsUI::ERowKind::Combo:
			{
				TSubclassOf<URageComboRow> RowClass = ResolveRowWidgetClass(Property, URageComboRow::StaticClass(), UISettings->DefaultComboRowClass.Get());
				if (URageComboRow* Row = CreateWidget<URageComboRow>(this, RowClass))
				{
					Row->SetLabel(Label);
					Row->SetRowId(Property->GetFName());

					TArray<FString> Options;
					if (const UEnum* Enum = RageSettingsUI::ResolveEnum(Property))
					{
						const int32 Count = Enum->NumEnums() - (Enum->ContainsExistingMax() ? 1 : 0);
						Options.Reserve(Count);
						for (int32 Index = 0; Index < Count; ++Index)
						{
							Options.Add(RageSettingsUI::DeriveDefaultEnumValueLabel(Enum, Enum->GetValueByIndex(Index)).ToString());
						}
					}
					Row->SetOptions(Options);
					Row->ValueChangedDelegate.AddUObject(this, &URageSettingsRowGeneratorPanelBase::HandleGeneratedComboChanged);
					TryOverrideObjectWidgetManipulation(Property, Row);
					Container->AddChild(Row);
				}
				break;
			}
			case RageSettingsUI::ERowKind::Selection:
			{
				TSubclassOf<URageSelectionRow> RowClass = ResolveRowWidgetClass(Property, URageSelectionRow::StaticClass(), UISettings->DefaultSelectionRowClass.Get());
				if (URageSelectionRow* Row = CreateWidget<URageSelectionRow>(this, RowClass))
				{
					Row->SetLabel(Label);
					Row->SetRowId(Property->GetFName());

					TArray<FText> Options;
					if (const UEnum* Enum = RageSettingsUI::ResolveEnum(Property))
					{
						const int32 Count = Enum->NumEnums() - (Enum->ContainsExistingMax() ? 1 : 0);
						Options.Reserve(Count);
						for (int32 Index = 0; Index < Count; ++Index)
						{
							Options.Add(RageSettingsUI::DeriveDefaultEnumValueLabel(Enum, Enum->GetValueByIndex(Index)));
						}
					}
					Row->SetOptions(Options);
					Row->ValueChangedDelegate.AddUObject(this, &URageSettingsRowGeneratorPanelBase::HandleGeneratedSelectionChanged);
					TryOverrideObjectWidgetManipulation(Property, Row);
					Container->AddChild(Row);
				}
			}
			break;
			default:
				break;
		}
	}

	RefreshRowsFromSettings(PendingObject);
}

void URageSettingsRowGeneratorPanelBase::RefreshFromSettings()
{
	RefreshRowsFromSettings(BoundPendingObject);
}

void URageSettingsRowGeneratorPanelBase::RefreshRowsFromSettings(const UObject* PendingObject) const
{
	if (!IsValid(RowsContainer) || !IsValid(PendingObject))
	{
		return;
	}

	for (UWidget* Child : RowsContainer->GetAllChildren())
	{
		if (URageToggleRow* ToggleRow = Cast<URageToggleRow>(Child))
		{
			if (const FBoolProperty* BoolProperty = CastField<FBoolProperty>(FindRowProperty(ToggleRow->GetRowId())))
			{
				ToggleRow->SetValue(BoolProperty->GetPropertyValue_InContainer(PendingObject));
			}
		}
		else if (URageSliderRow* SliderRow = Cast<URageSliderRow>(Child))
		{
			if (const FProperty* Property = FindRowProperty(SliderRow->GetRowId()))
			{
				SliderRow->SetValue(static_cast<float>(RageSettingsUI::GetNumericPropertyValue(Property, PendingObject)));
			}
		}
		else if (URageComboRow* ComboRow = Cast<URageComboRow>(Child))
		{
			if (const FProperty* Property = FindRowProperty(ComboRow->GetRowId()))
			{
				ComboRow->SetSelectedIndex(RageSettingsUI::GetEnumPropertyIndex(Property, PendingObject));
			}
		}
		else if (URageSelectionRow* SelectionRow = Cast<URageSelectionRow>(Child))
		{
			if (const FProperty* Property = FindRowProperty(SelectionRow->GetRowId()))
			{
				SelectionRow->SetSelectedIndex(RageSettingsUI::GetEnumPropertyIndex(Property, PendingObject));
			}
		}
	}
}

void URageSettingsRowGeneratorPanelBase::HandleGeneratedToggleChanged(FName RowId, FRageVariant bNewValue)
{
	FBoolProperty* BoolProperty = CastField<FBoolProperty>(FindRowProperty(RowId));
	IRageSettingsCategoryInterface* Category = GetOwningCategory();
	if (!BoolProperty || !Category || !IsValid(BoundPendingObject))
	{
		return;
	}

	const bool bWasDirty = Category->IsDirty();
	BoolProperty->SetPropertyValue_InContainer(BoundPendingObject, bNewValue.Get<bool>());
	Category->NotifyPendingChangedExternally(bWasDirty);
}

void URageSettingsRowGeneratorPanelBase::HandleGeneratedSliderChanged(FName RowId, FRageVariant NewValue)
{
	FProperty* Property = FindRowProperty(RowId);
	IRageSettingsCategoryInterface* Category = GetOwningCategory();
	if (!Property || !Category || !IsValid(BoundPendingObject))
	{
		return;
	}

	const bool bWasDirty = Category->IsDirty();
	RageSettingsUI::SetNumericPropertyValue(Property, BoundPendingObject, NewValue.Get<float>());
	Category->NotifyPendingChangedExternally(bWasDirty);
}

void URageSettingsRowGeneratorPanelBase::HandleGeneratedComboChanged(FName RowId, FRageVariant NewIndex)
{
	FProperty* Property = FindRowProperty(RowId);
	IRageSettingsCategoryInterface* Category = GetOwningCategory();
	if (!Property || !Category || !IsValid(BoundPendingObject))
	{
		return;
	}

	const bool bWasDirty = Category->IsDirty();
	RageSettingsUI::SetEnumPropertyByIndex(Property, BoundPendingObject, NewIndex.Get<int32>());
	Category->NotifyPendingChangedExternally(bWasDirty);
}

void URageSettingsRowGeneratorPanelBase::HandleGeneratedSelectionChanged(FName RowId, FRageVariant NewIndex)
{
	FProperty* Property = FindRowProperty(RowId);
	IRageSettingsCategoryInterface* Category = GetOwningCategory();
	if (!Property || !Category || !IsValid(BoundPendingObject))
	{
		return;
	}

	const bool bWasDirty = Category->IsDirty();
	RageSettingsUI::SetEnumPropertyByIndex(Property, BoundPendingObject, NewIndex.Get<int32>());
	Category->NotifyPendingChangedExternally(bWasDirty);
}

FProperty* URageSettingsRowGeneratorPanelBase::FindRowProperty(FName RowId) const
{
	return IsValid(BoundPendingObject) ? BoundPendingObject->GetClass()->FindPropertyByName(RowId) : nullptr;
}

IRageSettingsCategoryInterface* URageSettingsRowGeneratorPanelBase::GetOwningCategory() const
{
	return IsValid(BoundPendingObject) ? Cast<IRageSettingsCategoryInterface>(BoundPendingObject->GetOuter()) : nullptr;
}
