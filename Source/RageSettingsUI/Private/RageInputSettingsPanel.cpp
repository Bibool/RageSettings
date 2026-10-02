// Copyright (c) 2026 Abdallah Boutrif
// SPDX-License-Identifier: MIT

#include "RageInputSettingsPanel.h"

#include "RageConfirmModal.h"
#include "RageInputSettings.h"
#include "RageKeybindConfig.h"
#include "RageKeybindRow.h"
#include "RageRowBaseUserWidget.h"
#include "RageMacros.h"
#include "RageSettingsDeveloperSettings.h"
#include "RageSettingsSharedDebug.h"
#include "RageSettingsSubsystem.h"
#include "RageSettingsUIDeveloperSettings.h"
#include "RageSliderDisplayFormat.h"
#include "Components/PanelWidget.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(RageInputSettingsPanel)

namespace
{
	/** Sorts categories that have no KeybindCategoriesConfig entry - and uncategorised actions - below every configured one. */
	constexpr int32 UnconfiguredCategoryPriority = MIN_int32;

	int32 GetCategoryPriority(const FGameplayTag& Category)
	{
		const FRageKeybindCategoryConfig* CategoryConfig = SETTINGS_UI->KeybindCategoriesConfig.Find(Category);
		return CategoryConfig ? CategoryConfig->Priority : UnconfiguredCategoryPriority;
	}

	TSubclassOf<URageRowBaseUserWidget> ResolveCategoryWidgetClass(const FRageKeybindCategoryConfig& CategoryConfig, const TSubclassOf<URageRowBaseUserWidget> PanelDefault)
	{
		if (CategoryConfig.OptionalCategoryWidget.IsSet() && CategoryConfig.OptionalCategoryWidget.GetValue().Get())
		{
			return CategoryConfig.OptionalCategoryWidget.GetValue();
		}

		return PanelDefault;
	}
}

void URageInputSettingsPanel::SetKeybindConflictModal(URageConfirmModal* NewModal)
{
	if (NewModal == KeybindConflictModal)
	{
		return;
	}

	if (IsValid(KeybindConflictModal))
	{
		KeybindConflictModal->ConfirmChosenDelegate.RemoveAll(this);
		KeybindConflictModal->AlternateChosenDelegate.RemoveAll(this);
		KeybindConflictModal->CancelChosenDelegate.RemoveAll(this);
	}

	KeybindConflictModal = NewModal;
	BindConflictModal();
}

void URageInputSettingsPanel::NativeConstruct()
{
	Super::NativeConstruct();

	BindConflictModal();
}

void URageInputSettingsPanel::BindConflictModal()
{
	if (IsValid(KeybindConflictModal))
	{
		KeybindConflictModal->ConfirmChosenDelegate.AddUniqueDynamic(this, &URageInputSettingsPanel::HandleConflictConfirmed);
		KeybindConflictModal->AlternateChosenDelegate.AddUniqueDynamic(this, &URageInputSettingsPanel::HandleConflictAlternateChosen);
		KeybindConflictModal->CancelChosenDelegate.AddUniqueDynamic(this, &URageInputSettingsPanel::HandleConflictCancelled);
	}
}

void URageInputSettingsPanel::InitializePanel(URageSettingsSubsystem* InSubsystem)
{
	InputSettings = InSubsystem->GetInputSettings();

	InputSettings->RegisterRemappableContexts();

	BuildRows(RowsContainer, InputSettings->GetPendingSettings());
	BuildKeybindRows();
}

void URageInputSettingsPanel::RefreshFromSettings()
{
	Super::RefreshFromSettings();

	for (const TPair<FName, URageKeybindRow*>& Entry : KeybindRowsByMapping)
	{
		if (IsValid(Entry.Value))
		{
			Entry.Value->SetCurrentKey(InputSettings->GetCurrentKeyForMapping(Entry.Key));
		}
	}
}

TArray<FRageSettingsRowDescriptor> URageInputSettingsPanel::GetRowDescriptors() const
{
	const URageSettingsDeveloperSettings* DevSettings = URageSettingsDeveloperSettings::Get();

	FRageSettingsRowDescriptor MouseSensitivityX;
	MouseSensitivityX.PropertyName = GET_MEMBER_NAME_CHECKED(URageInputSettings, MouseSensitivityX);
	MouseSensitivityX.Label = RAGE_LOC("MouseSensitivityX");
	MouseSensitivityX.ClampMin = DevSettings->SensitivityMin;
	MouseSensitivityX.ClampMax = DevSettings->SensitivityMax;
	MouseSensitivityX.SliderFormat = ERageSliderDisplayFormat::Multiplier;

	FRageSettingsRowDescriptor MouseSensitivityY;
	MouseSensitivityY.PropertyName = GET_MEMBER_NAME_CHECKED(URageInputSettings, MouseSensitivityY);
	MouseSensitivityY.Label = RAGE_LOC("MouseSensitivityY");
	MouseSensitivityY.ClampMin = DevSettings->SensitivityMin;
	MouseSensitivityY.ClampMax = DevSettings->SensitivityMax;
	MouseSensitivityY.SliderFormat = ERageSliderDisplayFormat::Multiplier;

	FRageSettingsRowDescriptor GamepadSensitivityX;
	GamepadSensitivityX.PropertyName = GET_MEMBER_NAME_CHECKED(URageInputSettings, GamepadSensitivityX);
	GamepadSensitivityX.Label = RAGE_LOC("GamepadSensitivityX");
	GamepadSensitivityX.ClampMin = DevSettings->SensitivityMin;
	GamepadSensitivityX.ClampMax = DevSettings->SensitivityMax;
	GamepadSensitivityX.SliderFormat = ERageSliderDisplayFormat::Multiplier;

	FRageSettingsRowDescriptor GamepadSensitivityY;
	GamepadSensitivityY.PropertyName = GET_MEMBER_NAME_CHECKED(URageInputSettings, GamepadSensitivityY);
	GamepadSensitivityY.Label = RAGE_LOC("GamepadSensitivityY");
	GamepadSensitivityY.ClampMin = DevSettings->SensitivityMin;
	GamepadSensitivityY.ClampMax = DevSettings->SensitivityMax;
	GamepadSensitivityY.SliderFormat = ERageSliderDisplayFormat::Multiplier;

	return { MouseSensitivityX, MouseSensitivityY, GamepadSensitivityX, GamepadSensitivityY };
}

void URageInputSettingsPanel::BuildKeybindRows()
{
	KeybindListContainer->ClearChildren();
	KeybindRows.Reset();
	KeybindRowsByMapping.Reset();
	
	TArray<FGameplayTag> OrderedCategories;
	TMap<FGameplayTag, TArray<const FRageKeybindConfig*>> ActionsByCategory;
	for (const FRageKeybindConfig& Config : SETTINGS->RemappableActions)
	{
		TArray<const FRageKeybindConfig*>& CategoryActions = ActionsByCategory.FindOrAdd(Config.Category);
		if (CategoryActions.IsEmpty())
		{
			OrderedCategories.Add(Config.Category);
		}

		CategoryActions.Add(&Config);
	}

	OrderedCategories.StableSort([](const FGameplayTag& A, const FGameplayTag& B)
	{
		return GetCategoryPriority(A) > GetCategoryPriority(B);
	});

	for (const FGameplayTag& Category : OrderedCategories)
	{
		if (const FRageKeybindCategoryConfig* CategoryConfig = SETTINGS_UI->KeybindCategoriesConfig.Find(Category))
		{
			if (URageRowBaseUserWidget* CategoryWidget = CreateWidget<URageRowBaseUserWidget>(this, ResolveCategoryWidgetClass(*CategoryConfig, GetSectionWidgetClass())))
			{
				CategoryWidget->SetRowId(Category.GetTagName());
				CategoryWidget->SetLabel(CategoryConfig->CategoryDisplayName);
				KeybindListContainer->AddChild(CategoryWidget);
			}
		}
		else if (Category.IsValid())
		{
			S_LOG(Warning, "Rage Settings : keybind category {category} has no KeybindCategoriesConfig entry - listing its actions without a header.",
				*Category.ToString());
		}

		TArray<const FRageKeybindConfig*>& CategoryActions = ActionsByCategory[Category];
		CategoryActions.StableSort([](const FRageKeybindConfig& A, const FRageKeybindConfig& B) { return A.Priority > B.Priority; });

		for (const FRageKeybindConfig* Config : CategoryActions)
		{
			if (URageKeybindRow* Row = CreateKeybindRow(*Config))
			{
				KeybindListContainer->AddChild(Row);
				KeybindRows.Add(Row);
				KeybindRowsByMapping.Add(Config->MappingName, Row);
			}
		}
	}
}

URageKeybindRow* URageInputSettingsPanel::CreateKeybindRow(const FRageKeybindConfig& Config)
{
	URageKeybindRow* Row = CreateWidget<URageKeybindRow>(this, KeybindRowClass);
	if (!IsValid(Row))
	{
		return nullptr;
	}

	Row->Setup(Config.MappingName, Config.DisplayName);
	Row->SetToolTipText(Config.Description);
	Row->SetDescription(Config.Description);
	Row->SetCurrentKey(InputSettings->GetCurrentKeyForMapping(Config.MappingName));
	Row->KeyRemappedDelegate.AddDynamic(this, &URageInputSettingsPanel::HandleKeyRemapped);
	Row->ResetToDefaultRequestedDelegate.AddDynamic(this, &URageInputSettingsPanel::HandleResetKeyToDefaultRequested);

	return Row;
}

void URageInputSettingsPanel::RefreshKeyForMapping(FName MappingName) const
{
	if (URageKeybindRow* const* Row = KeybindRowsByMapping.Find(MappingName))
	{
		(*Row)->SetCurrentKey(InputSettings->GetCurrentKeyForMapping(MappingName));
	}
}

void URageInputSettingsPanel::CommitPendingRemap(const bool bSwap)
{
	if (ApplyRemap(PendingRemapMappingName, PendingRemapKey))
	{
		ResolveConflictingMappings(bSwap);
	}

	ClearPendingRemap();
}

bool URageInputSettingsPanel::ApplyRemap(FName MappingName, FKey NewKey)
{
	if (InputSettings->RemapPlayerKey(MappingName, NewKey))
	{
		return true;
	}

	RefreshKeyForMapping(MappingName);
	return false;
}

void URageInputSettingsPanel::ResolveConflictingMappings(const bool bSwap)
{
	const bool bCanSwap = bSwap && PendingRemapPreviousKey.IsValid();

	for (int32 Index = 0; Index < PendingRemapConflicts.Num(); ++Index)
	{
		const FName Conflicting = PendingRemapConflicts[Index];

		if (bCanSwap && Index == 0)
		{
			InputSettings->RemapPlayerKey(Conflicting, PendingRemapPreviousKey);
		}
		else
		{
			InputSettings->ClearKeyMapping(Conflicting);
		}

		RefreshKeyForMapping(Conflicting);
	}
}

bool URageInputSettingsPanel::WillSwapConflictingKey() const
{
	return SETTINGS->bRebindConflictSwapsKey && PendingRemapPreviousKey.IsValid();
}

bool URageInputSettingsPanel::ShouldOfferConflictChoice() const
{
	return bOfferUnbindOnConflict && PendingRemapPreviousKey.IsValid();
}

FText URageInputSettingsPanel::GetMappingDisplayName(const FName MappingName) const
{
	const FRageKeybindConfig* Config = SETTINGS->RemappableActions.FindByPredicate(
		[MappingName](const FRageKeybindConfig& Candidate) { return Candidate.MappingName == MappingName; });

	return Config && !Config->DisplayName.IsEmpty() ? Config->DisplayName : FText::FromName(MappingName);
}

FText URageInputSettingsPanel::BuildConflictMessage() const
{
	TArray<FText> ConflictingNames;
	ConflictingNames.Reserve(PendingRemapConflicts.Num());
	for (const FName Conflicting : PendingRemapConflicts)
	{
		ConflictingNames.Add(GetMappingDisplayName(Conflicting));
	}

	FFormatNamedArguments Args;
	Args.Add(TEXT("Key"), PendingRemapKey.GetDisplayName());
	Args.Add(TEXT("PreviousKey"), PendingRemapPreviousKey.GetDisplayName());
	Args.Add(TEXT("Actions"), FText::Join(RAGE_LOC("KeybindConflictSeparator"), ConflictingNames));
	
	if (ShouldOfferConflictChoice())
	{
		return FText::Format(RAGE_LOC("KeybindConflictChoice"), Args);
	}

	return FText::Format(WillSwapConflictingKey() ? RAGE_LOC("KeybindConflictSwap") : RAGE_LOC("KeybindConflict"), Args);
}

TArray<FRageModalChange> URageInputSettingsPanel::BuildConflictChanges(const bool bSwap) const
{
	const FText Unbound = RAGE_LOC("KeyUnbound");
	const bool bCanSwap = bSwap && PendingRemapPreviousKey.IsValid();

	TArray<FRageModalChange> Changes;

	FRageModalChange& Remapped = Changes.AddDefaulted_GetRef();
	Remapped.Label = GetMappingDisplayName(PendingRemapMappingName);
	Remapped.From = PendingRemapPreviousKey.IsValid() ? PendingRemapPreviousKey.GetDisplayName() : Unbound;
	Remapped.To = PendingRemapKey.GetDisplayName();

	for (int32 Index = 0; Index < PendingRemapConflicts.Num(); ++Index)
	{
		FRageModalChange& Displaced = Changes.AddDefaulted_GetRef();
		Displaced.Label = GetMappingDisplayName(PendingRemapConflicts[Index]);
		Displaced.From = PendingRemapKey.GetDisplayName();
		Displaced.To = bCanSwap && Index == 0 ? PendingRemapPreviousKey.GetDisplayName() : Unbound;
		Displaced.bWarning = true;
	}

	return Changes;
}

void URageInputSettingsPanel::ClearPendingRemap()
{
	PendingRemapMappingName = NAME_None;
	PendingRemapKey = FKey();
	PendingRemapPreviousKey = FKey();
	PendingRemapConflicts.Reset();
}

void URageInputSettingsPanel::HandleKeyRemapped(FName MappingName, FKey NewKey)
{
	PendingRemapMappingName = MappingName;
	PendingRemapKey = NewKey;
	PendingRemapPreviousKey = InputSettings->GetCurrentKeyForMapping(MappingName);
	PendingRemapConflicts = InputSettings->FindMappingsUsingKey(NewKey, MappingName);

	if (PendingRemapConflicts.IsEmpty() || !IsValid(KeybindConflictModal))
	{
		CommitPendingRemap(WillSwapConflictingKey());
		return;
	}

	FFormatNamedArguments TitleArgs;
	TitleArgs.Add(TEXT("Key"), PendingRemapKey.GetDisplayName());
	KeybindConflictModal->SetTitle(FText::Format(RAGE_LOC("KeybindConflictTitle"), TitleArgs));
	KeybindConflictModal->SetMessage(BuildConflictMessage());

	if (bOfferUnbindOnConflict)
	{
		/* With no old key to hand over there is nothing to swap, so confirming is the unbind and the alternate goes away. */
		const bool bOfferChoice = ShouldOfferConflictChoice();
		KeybindConflictModal->SetAlternateChoiceOffered(bOfferChoice);
		KeybindConflictModal->SetChoiceLabels(bOfferChoice ? RAGE_LOC("KeybindSwap") : RAGE_LOC("KeybindUnbindOther"), RAGE_LOC("KeybindUnbindOther"));
		KeybindConflictModal->SetChanges(BuildConflictChanges(bOfferChoice));
	}
	else
	{
		KeybindConflictModal->SetChanges(BuildConflictChanges(WillSwapConflictingKey()));
	}

	KeybindConflictModal->Open();
}

void URageInputSettingsPanel::HandleResetKeyToDefaultRequested(FName MappingName)
{
	InputSettings->ResetKeyMappingToDefault(MappingName);
	RefreshKeyForMapping(MappingName);
}

void URageInputSettingsPanel::HandleConflictConfirmed()
{
	KeybindConflictModal->Close();

	CommitPendingRemap(bOfferUnbindOnConflict ? ShouldOfferConflictChoice() : WillSwapConflictingKey());
}

void URageInputSettingsPanel::HandleConflictAlternateChosen()
{
	KeybindConflictModal->Close();

	CommitPendingRemap(/*bSwap*/ false);
}

void URageInputSettingsPanel::HandleConflictCancelled()
{
	KeybindConflictModal->Close();

	RefreshKeyForMapping(PendingRemapMappingName);
	ClearPendingRemap();
}
