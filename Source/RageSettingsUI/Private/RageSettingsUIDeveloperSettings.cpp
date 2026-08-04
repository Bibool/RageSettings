// Copyright (c) 2026 Abdallah Boutrif
// SPDX-License-Identifier: MIT

#include "RageSettingsUIDeveloperSettings.h"

#include "RageScalabilityCategory.h"
#include "RageToggleRow.h"
#include "RageSliderRow.h"
#include "RageComboRow.h"
#include "RageSelectionRow.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(RageSettingsUIDeveloperSettings)

URageSettingsUIDeveloperSettings::URageSettingsUIDeveloperSettings()
{
	static ConstructorHelpers::FClassFinder<URageToggleRow> ToggleFinder(TEXT("/RageSettings/W_ToggleRow_View.W_ToggleRow_View_C"));
	if (ToggleFinder.Succeeded())
	{
		DefaultToggleRowClass = ToggleFinder.Class;
	}

	static ConstructorHelpers::FClassFinder<URageSliderRow> SliderFinder(TEXT("/RageSettings/W_SliderRow_View.W_SliderRow_View_C"));
	if (SliderFinder.Succeeded())
	{
		DefaultSliderRowClass = SliderFinder.Class;
	}

	static ConstructorHelpers::FClassFinder<URageComboRow> ComboFinder(TEXT("/RageSettings/W_ComboRow_View.W_ComboRow_View_C"));
	if (ComboFinder.Succeeded())
	{
		DefaultComboRowClass = ComboFinder.Class;
	}
	
	static ConstructorHelpers::FClassFinder<URageRowBaseUserWidget> CategoryFinder(TEXT("/RageSettings/W_Category_View.W_Category_View_C"));
	if (CategoryFinder.Succeeded())
	{
		DefaultCategoryWidgetClass = CategoryFinder.Class;
	}
	
	static ConstructorHelpers::FClassFinder<URageSelectionRow> SelectionFinder(TEXT("/RageSettings/W_SelectionRow_View.W_SelectionRow_View_C"));
	if (SelectionFinder.Succeeded())
	{
		DefaultSelectionRowClass = SelectionFinder.Class;
	}
	
	ScalabilityOverrides.Reserve(RageScalabilityCategory::Max);

	ScalabilityOverrides.Add(ERageScalabilityCategory::ViewDistance, true);
	ScalabilityOverrides.Add(ERageScalabilityCategory::AntiAliasing, true);
	ScalabilityOverrides.Add(ERageScalabilityCategory::Shadow, true);
	ScalabilityOverrides.Add(ERageScalabilityCategory::GlobalIllumination, true);
	ScalabilityOverrides.Add(ERageScalabilityCategory::Reflection, true);
	ScalabilityOverrides.Add(ERageScalabilityCategory::PostProcess, true);
	ScalabilityOverrides.Add(ERageScalabilityCategory::Texture, true);
	ScalabilityOverrides.Add(ERageScalabilityCategory::Effects, true);
	ScalabilityOverrides.Add(ERageScalabilityCategory::Foliage, true);
	ScalabilityOverrides.Add(ERageScalabilityCategory::Shading, true);
}

URageSettingsUIDeveloperSettings* URageSettingsUIDeveloperSettings::Get()
{
	return GetMutableDefault<URageSettingsUIDeveloperSettings>();
}
