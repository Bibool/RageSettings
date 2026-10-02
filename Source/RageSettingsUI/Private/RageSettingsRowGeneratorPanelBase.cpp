// Copyright (c) 2026 Abdallah Boutrif
// SPDX-License-Identifier: MIT

#include "RageSettingsRowGeneratorPanelBase.h"

#include "RageComboRow.h"
#include "RageRowOverrideObject.h"
#include "RageSelectionRow.h"
#include "Components/PanelWidget.h"
#include "RageSettingsCategoryInterface.h"
#include "RageSettingsSharedDebug.h"
#include "RageSettingsUIDeveloperSettings.h"
#include "RageSliderRow.h"
#include "RageToggleRow.h"
#include "UObject/UnrealType.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(RageSettingsRowGeneratorPanelBase)

namespace
{
	const FRageRowOverrideData* FindProjectOverride(const FProperty* Property)
	{
		return URageSettingsUIDeveloperSettings::Get()->RowWidgetClassOverrides.Find(Property->GetFName());
	}

	bool ShouldCreateWidget(const FProperty* Property)
	{
		const FRageRowOverrideData* Override = FindProjectOverride(Property);
		return !Override || Override->bCreateWidget;
	}

	void TryOverrideObjectWidgetManipulation(const FProperty* Property, URageRowBaseUserWidget* InWidget)
	{
		if (const FRageRowOverrideData* Override = FindProjectOverride(Property))
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

	TArray<FProperty*> Properties;
	for (FProperty* Property : RageSettingsUI::CollectRowProperties(PendingObject->GetClass()))
	{
		if (ShouldCreateWidget(Property) && ResolveKind(Property) != RageSettingsUI::ERowKind::Unsupported)
		{
			Properties.Add(Property);
		}
	}

	auto AddRow = [this, Container, &Descriptors](FProperty* Property)
	{
		if (URageRowBaseUserWidget* Row = CreateRow(Property, Descriptors))
		{
			Container->AddChild(Row);
		}
	};

	if (Sections.IsEmpty())
	{
		for (FProperty* Property : Properties)
		{
			AddRow(Property);
		}
	}
	else
	{
		TSet<FProperty*> Listed;
		for (int32 SectionIndex = 0; SectionIndex < Sections.Num(); ++SectionIndex)
		{
			const FRageSettingsSection& Section = Sections[SectionIndex];

			TArray<FProperty*> SectionProperties;
			for (const FName& Name : Section.Properties)
			{
				FProperty* const* Found = Properties.FindByPredicate([Name](const FProperty* Candidate) { return Candidate->GetFName() == Name; });
				if (Found && !Listed.Contains(*Found))
				{
					SectionProperties.Add(*Found);
					Listed.Add(*Found);
				}
			}

			/* A section whose every field was hidden or removed would be a title over nothing. */
			if (SectionProperties.IsEmpty())
			{
				continue;
			}

			AddSectionHeader(Container, Section.Title, SectionIndex);
			for (FProperty* Property : SectionProperties)
			{
				AddRow(Property);
			}
		}

		bool bUnlistedHeaderAdded = false;
		for (FProperty* Property : Properties)
		{
			if (Listed.Contains(Property))
			{
				continue;
			}

			if (!bUnlistedHeaderAdded)
			{
				AddSectionHeader(Container, UnlistedSectionTitle, Sections.Num());
				bUnlistedHeaderAdded = true;
			}

			AddRow(Property);
		}
	}

	RefreshRowsFromSettings(PendingObject);
}

TSubclassOf<URageRowBaseUserWidget> URageSettingsRowGeneratorPanelBase::GetSectionWidgetClass() const
{
	return SectionWidgetClass ? SectionWidgetClass : URageSettingsUIDeveloperSettings::Get()->DefaultCategoryWidgetClass;
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

URageRowBaseUserWidget* URageSettingsRowGeneratorPanelBase::CreateRow(FProperty* Property, const TArray<FRageSettingsRowDescriptor>& Descriptors)
{
	const URageSettingsUIDeveloperSettings* UISettings = URageSettingsUIDeveloperSettings::Get();

	const FRageSettingsRowDescriptor* Descriptor = Descriptors.FindByPredicate(
		[Property](const FRageSettingsRowDescriptor& Candidate) { return Candidate.PropertyName == Property->GetFName(); });

	URageRowBaseUserWidget* Row = nullptr;

	switch (ResolveKind(Property))
	{
		case RageSettingsUI::ERowKind::Toggle:
		{
			const TSubclassOf<URageToggleRow> RowClass = ResolveRowClass(Property, URageToggleRow::StaticClass(), ToggleRowClass, UISettings->DefaultToggleRowClass);
			if (URageToggleRow* ToggleRow = CreateWidget<URageToggleRow>(this, RowClass))
			{
				ToggleRow->ValueChangedDelegate.AddUObject(this, &URageSettingsRowGeneratorPanelBase::HandleGeneratedToggleChanged);
				Row = ToggleRow;
			}
			break;
		}
		case RageSettingsUI::ERowKind::Slider:
		{
			const TSubclassOf<URageSliderRow> RowClass = ResolveRowClass(Property, URageSliderRow::StaticClass(), SliderRowClass, UISettings->DefaultSliderRowClass);
			if (URageSliderRow* SliderRow = CreateWidget<URageSliderRow>(this, RowClass))
			{
				if (Descriptor)
				{
					SliderRow->SetRange(Descriptor->ClampMin, Descriptor->ClampMax);
					SliderRow->SetDisplayFormat(Descriptor->SliderFormat);
				}
				SliderRow->ValueChangedDelegate.AddUObject(this, &URageSettingsRowGeneratorPanelBase::HandleGeneratedSliderChanged);
				Row = SliderRow;
			}
			break;
		}
		case RageSettingsUI::ERowKind::Combo:
		{
			const TSubclassOf<URageComboRow> RowClass = ResolveRowClass(Property, URageComboRow::StaticClass(), ComboRowClass, UISettings->DefaultComboRowClass);
			if (URageComboRow* ComboRow = CreateWidget<URageComboRow>(this, RowClass))
			{
				ComboRow->SetOptionTexts(RageSettingsUI::BuildEnumOptionLabels(Property));
				ComboRow->ValueChangedDelegate.AddUObject(this, &URageSettingsRowGeneratorPanelBase::HandleGeneratedComboChanged);
				Row = ComboRow;
			}
			break;
		}
		case RageSettingsUI::ERowKind::Selection:
		{
			const TSubclassOf<URageSelectionRow> RowClass = ResolveRowClass(Property, URageSelectionRow::StaticClass(), SelectionRowClass, UISettings->DefaultSelectionRowClass);
			if (URageSelectionRow* SelectionRow = CreateWidget<URageSelectionRow>(this, RowClass))
			{
				SelectionRow->SetOptions(RageSettingsUI::BuildEnumOptionLabels(Property));
				SelectionRow->ValueChangedDelegate.AddUObject(this, &URageSettingsRowGeneratorPanelBase::HandleGeneratedSelectionChanged);
				Row = SelectionRow;
			}
			break;
		}
		default:
			break;
	}

	if (!IsValid(Row))
	{
		return nullptr;
	}

	Row->SetLabel(RageSettingsUI::ResolveRowLabel(Property, Descriptor));
	Row->SetRowId(Property->GetFName());

	const FText Hint = RageSettingsUI::ResolveRowHint(Property, Descriptor);
	if (!Hint.IsEmpty())
	{
		Row->SetHint(Hint);
	}

	const FText Description = RageSettingsUI::ResolveRowDescription(Property, Descriptor);
	if (!Description.IsEmpty())
	{
		Row->SetDescription(Description);
	}

	TryOverrideObjectWidgetManipulation(Property, Row);

	return Row;
}

void URageSettingsRowGeneratorPanelBase::AddSectionHeader(UPanelWidget* Container, const FText& Title, const int32 SectionIndex)
{
	const TSubclassOf<URageRowBaseUserWidget> HeaderClass = GetSectionWidgetClass();
	if (Title.IsEmpty() || !HeaderClass)
	{
		return;
	}

	if (URageRowBaseUserWidget* Header = CreateWidget<URageRowBaseUserWidget>(this, HeaderClass))
	{
		Header->SetRowId(FName(TEXT("Section"), SectionIndex + 1));
		Header->SetLabel(Title);
		Container->AddChild(Header);
	}
}

RageSettingsUI::ERowKind URageSettingsRowGeneratorPanelBase::ResolveKind(const FProperty* Property) const
{
	const RageSettingsUI::ERowKind Kind = RageSettingsUI::ResolveRowKind(Property);
	if (Kind != RageSettingsUI::ERowKind::Combo && Kind != RageSettingsUI::ERowKind::Selection)
	{
		return Kind;
	}

	if (const TSubclassOf<URageRowBaseUserWidget>* Override = RowClassOverrides.Find(Property->GetFName()))
	{
		if (const UClass* OverrideClass = Override->Get())
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

	return Kind;
}

UClass* URageSettingsRowGeneratorPanelBase::ResolveRowClass(const FProperty* Property, const UClass* ExpectedBase, UClass* PanelDefault, UClass* ProjectDefault) const
{
	auto Accept = [Property, ExpectedBase](const UClass* Candidate, const TCHAR* Source)
	{
		if (!Candidate)
		{
			return false;
		}

		if (Candidate->IsChildOf(ExpectedBase))
		{
			return true;
		}

		S_LOG(Warning, "Rage Settings : {source}[{property}] ({class}) doesn't derive from {base} - ignoring it.",
			Source, *Property->GetName(), *Candidate->GetName(), *ExpectedBase->GetName());
		return false;
	};

	if (const TSubclassOf<URageRowBaseUserWidget>* PanelOverride = RowClassOverrides.Find(Property->GetFName()))
	{
		if (Accept(PanelOverride->Get(), TEXT("RowClassOverrides")))
		{
			return PanelOverride->Get();
		}
	}

	if (const FRageRowOverrideData* ProjectOverride = FindProjectOverride(Property))
	{
		if (Accept(ProjectOverride->WidgetClass, TEXT("RowWidgetClassOverrides")))
		{
			return ProjectOverride->WidgetClass;
		}
	}

	return PanelDefault ? PanelDefault : ProjectDefault;
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
