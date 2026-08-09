// Copyright (c) 2026 Abdallah Boutrif
// SPDX-License-Identifier: MIT

#include "RageInputSettings.h"

#include "Engine/GameInstance.h"
#include "Engine/LocalPlayer.h"
#include "EnhancedInputSubsystems.h"
#include "InputMappingContext.h"
#include "UserSettings/EnhancedInputUserSettings.h"
#include "GameplayTagContainer.h"
#include "RageSettingsDeveloperSettings.h"
#include "RageSettingsReflectionUtils.h"
#include "RageSettingsShared/Public/RageSettingsSharedDebug.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(RageInputSettings)

void URageInputSettings::InitializeWithGameInstance(UGameInstance* InGameInstance)
{
	GameInstance = InGameInstance;
}

void URageInputSettings::LoadSettings()
{
	Pending = CastChecked<URageInputSettings>(RageSettings::CreateShadowInstance(this, this));
	Defaults = CastChecked<URageInputSettings>(RageSettings::CreateShadowInstance(this, GetClass()->GetDefaultObject()));
}

void URageInputSettings::ApplySettings()
{
	RageSettings::CopyObjectProperties(this, Pending);

	DirtyStateChangedDelegate.Broadcast(ERageSettingsCategory::Input, false);
}

void URageInputSettings::SaveSettings()
{
	SaveConfig();
}

void URageInputSettings::ResetToDefault()
{
	const bool bWasDirty = IsDirty();
	RageSettings::CopyObjectProperties(Pending, Defaults);
	BroadcastDirtyIfChanged(bWasDirty);
}

void URageInputSettings::RevertPendingChanges()
{
	const bool bWasDirty = IsDirty();
	RageSettings::CopyObjectProperties(Pending, this);
	BroadcastDirtyIfChanged(bWasDirty);
}

bool URageInputSettings::IsDirty() const
{
	return !RageSettings::AreObjectsEqual(this, Pending);
}

void URageInputSettings::BroadcastDirtyIfChanged(bool bWasDirtyBefore)
{
	const bool bIsDirtyNow = IsDirty();
	if (bWasDirtyBefore != bIsDirtyNow)
	{
		DirtyStateChangedDelegate.Broadcast(ERageSettingsCategory::Input, bIsDirtyNow);
	}
}

void URageInputSettings::SetPendingMouseSensitivity(float X, float Y)
{
	const bool bWasDirty = IsDirty();
	Pending->MouseSensitivityX = FMath::Clamp(X, URageSettingsDeveloperSettings::Get()->SensitivityMin, URageSettingsDeveloperSettings::Get()->SensitivityMax);
	Pending->MouseSensitivityY = FMath::Clamp(Y, URageSettingsDeveloperSettings::Get()->SensitivityMin, URageSettingsDeveloperSettings::Get()->SensitivityMax);
	BroadcastDirtyIfChanged(bWasDirty);
}

void URageInputSettings::SetPendingInvertMouseY(bool bInvert)
{
	const bool bWasDirty = IsDirty();
	Pending->bInvertMouseY = bInvert;
	BroadcastDirtyIfChanged(bWasDirty);
}

void URageInputSettings::SetPendingGamepadSensitivity(float X, float Y)
{
	const bool bWasDirty = IsDirty();
	Pending->GamepadSensitivityX = FMath::Clamp(X, URageSettingsDeveloperSettings::Get()->SensitivityMin, URageSettingsDeveloperSettings::Get()->SensitivityMax);
	Pending->GamepadSensitivityY = FMath::Clamp(Y, URageSettingsDeveloperSettings::Get()->SensitivityMin, URageSettingsDeveloperSettings::Get()->SensitivityMax);
	BroadcastDirtyIfChanged(bWasDirty);
}

void URageInputSettings::SetPendingInvertGamepadY(bool bInvert)
{
	const bool bWasDirty = IsDirty();
	Pending->bInvertGamepadY = bInvert;
	BroadcastDirtyIfChanged(bWasDirty);
}

float URageInputSettings::GetMouseSensitivityX() const
{
	return MouseSensitivityX;
}

float URageInputSettings::GetMouseSensitivityY() const
{
	return MouseSensitivityY * (bInvertMouseY ? -1.f : 1.f);
}

UEnhancedInputUserSettings* URageInputSettings::GetEnhancedInputUserSettings() const
{
	if (!GameInstance.IsValid())
	{
		return nullptr;
	}

	const ULocalPlayer* LocalPlayer = GameInstance->GetLocalPlayerByIndex(0);
	if (!LocalPlayer)
	{
		return nullptr;
	}
	
	return LocalPlayer->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>()->GetUserSettings();
}

void URageInputSettings::RegisterRemappableContexts()
{
	UEnhancedInputUserSettings* UserSettings = GetEnhancedInputUserSettings();
	if (!IsValid(UserSettings))
	{
		return;
	}

	for (const TSoftObjectPtr<UInputMappingContext>& ContextRef : URageSettingsDeveloperSettings::Get()->RemappableContexts)
	{
		if (const UInputMappingContext* Context = ContextRef.LoadSynchronous())
		{
			UserSettings->RegisterInputMappingContext(Context);
		}
	}
}

bool URageInputSettings::RemapPlayerKey(FName MappingName, FKey NewKey)
{
	if (UEnhancedInputUserSettings* UserSettings = GetEnhancedInputUserSettings())
	{
		FMapPlayerKeyArgs Args;
		Args.MappingName = MappingName;
		Args.NewKey = NewKey;
		Args.Slot = EPlayerMappableKeySlot::First;
		FGameplayTagContainer FailureReason;
		UserSettings->MapPlayerKey(Args, FailureReason);
		if (FailureReason.IsEmpty())
		{
			UserSettings->ApplySettings();
			UserSettings->SaveSettings();
			return true;
		}

		S_LOG(Warning, "URageInputSettings::RemapPlayerKey Failed to map key {key} to {new_key} with reason {reason}", MappingName.ToString(), NewKey.ToString(), FailureReason.ToString());
	}

	return false;
}

FKey URageInputSettings::GetCurrentKeyForMapping(FName MappingName) const
{
	const UEnhancedInputUserSettings* UserSettings = GetEnhancedInputUserSettings();
	if (!IsValid(UserSettings))
	{
		return EKeys::Invalid;
	}
	
	const UEnhancedPlayerMappableKeyProfile* Profile = UserSettings->GetActiveKeyProfile();
	if (!IsValid(Profile))
	{
		return EKeys::Invalid;
	}
	
	const FKeyMappingRow* Row = Profile->FindKeyMappingRow(MappingName);
	if (!Row)
	{
		return EKeys::Invalid;
	}

	for (const FPlayerKeyMapping& Mapping : Row->Mappings)
	{
		if (Mapping.GetSlot() == EPlayerMappableKeySlot::First)
		{
			return Mapping.GetCurrentKey();
		}
	}
	
	return EKeys::Invalid;
}

TArray<FName> URageInputSettings::FindMappingsUsingKey(FKey Key, FName IgnoreMappingName) const
{
	TArray<FName> MappingNames;

	if (!Key.IsValid())
	{
		return MappingNames;
	}

	const UEnhancedInputUserSettings* UserSettings = GetEnhancedInputUserSettings();
	if (!IsValid(UserSettings))
	{
		return MappingNames;
	}

	const UEnhancedPlayerMappableKeyProfile* Profile = UserSettings->GetActiveKeyProfile();
	if (!IsValid(Profile))
	{
		return MappingNames;
	}

	// The profile holds a row per mappable action from the moment its context is registered, so this
	// sees actions still on their default key - not just ones the player has already remapped.
	Profile->GetMappingNamesForKey(Key, MappingNames);
	MappingNames.Remove(IgnoreMappingName);

	return MappingNames;
}

bool URageInputSettings::ClearKeyMapping(FName MappingName)
{
	// Not UnMapPlayerKey - that resets the mapping to its default key, which is the opposite of what
	// a caller clearing a conflict wants. Mapping an invalid key is what leaves an action unbound.
	return RemapPlayerKey(MappingName, EKeys::Invalid);
}

void URageInputSettings::ResetKeyMappingToDefault(FName MappingName)
{
	UEnhancedInputUserSettings* UserSettings = GetEnhancedInputUserSettings();
	if (!IsValid(UserSettings))
	{
		return;
	}
	
	UEnhancedPlayerMappableKeyProfile* Profile = UserSettings->GetActiveKeyProfile();
	if (!IsValid(Profile))
	{
		return;
	}
	
	Profile->ResetMappingToDefault(MappingName);

	UserSettings->ApplySettings();
	UserSettings->SaveSettings();
}

void URageInputSettings::ResetAllKeyMappingsToDefault()
{
	UEnhancedInputUserSettings* UserSettings = GetEnhancedInputUserSettings();
	if (!IsValid(UserSettings))
	{
		return;
	}
	
	UEnhancedPlayerMappableKeyProfile* Profile = UserSettings->GetActiveKeyProfile();
	if (!IsValid(Profile))
	{
		return;
	}
	
	Profile->ResetToDefault();
	UserSettings->ApplySettings();
	UserSettings->SaveSettings();
}
