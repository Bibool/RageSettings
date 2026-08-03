// Copyright (c) 2026 Abdallah Boutrif
// SPDX-License-Identifier: MIT

#include "RageSettingsSubsystem.h"

#include "RageVideoSettings.h"
#include "RageAudioSettings.h"
#include "RageGameSettings.h"
#include "RageInputSettings.h"
#include "RageSettingsDeveloperSettings.h"
#include "Engine/Engine.h"
#include "RageSettingsShared/Public/RageSettingsSharedDebug.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(RageSettingsSubsystem)

void URageSettingsSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	
	if (!IsValid(GEngine))
	{
		S_LOG(Error, "URageSettingsSubsystem::Initialize GEngine is not valid, cannot initialize RageSettingsSubsystem.");
		return;
	}
	
	VideoSettings = Cast<URageVideoSettings>(GEngine->GetGameUserSettings());
	if (!IsValid(VideoSettings))
	{
		S_LOG(Error, "URageSettingsSubsystem::Initialize Failed to cast GEngine->GetGameUserSettings() to URageVideoSettings. Make sure to assign it in DefaultEngine.ini's [/Script/Engine.Engine] section or ProjectSettings");
		return;
	}
	
	const URageSettingsDeveloperSettings* DevSettings = URageSettingsDeveloperSettings::Get();
	
	AudioSettings = NewObject<URageAudioSettings>(this, DevSettings->AudioSettingsClass);
	GameSettings = NewObject<URageGameSettings>(this, DevSettings->GameSettingsClass);
	InputSettings = NewObject<URageInputSettings>(this, DevSettings->InputSettingsClass);
	InputSettings->InitializeWithGameInstance(GetGameInstance());
	
	AllCategories = {
		TScriptInterface<IRageSettingsCategoryInterface>(VideoSettings), 
		TScriptInterface<IRageSettingsCategoryInterface>(AudioSettings),
		TScriptInterface<IRageSettingsCategoryInterface>(GameSettings), 
		TScriptInterface<IRageSettingsCategoryInterface>(InputSettings)};

	for (const TScriptInterface<IRageSettingsCategoryInterface>& Category : AllCategories)
	{
		Category->LoadSettings();
	}
	
	VideoSettings->DirtyStateChangedDelegate.AddUniqueDynamic(this, &URageSettingsSubsystem::HandleCategoryDirtyStateChanged);
	AudioSettings->DirtyStateChangedDelegate.AddUniqueDynamic(this, &URageSettingsSubsystem::HandleCategoryDirtyStateChanged);
	GameSettings->DirtyStateChangedDelegate.AddUniqueDynamic(this, &URageSettingsSubsystem::HandleCategoryDirtyStateChanged);
	InputSettings->DirtyStateChangedDelegate.AddUniqueDynamic(this, &URageSettingsSubsystem::HandleCategoryDirtyStateChanged);
}

void URageSettingsSubsystem::Deinitialize()
{
	VideoSettings->DirtyStateChangedDelegate.RemoveAll(this);
	AudioSettings->DirtyStateChangedDelegate.RemoveAll(this);
	GameSettings->DirtyStateChangedDelegate.RemoveAll(this);
	InputSettings->DirtyStateChangedDelegate.RemoveAll(this);
	
	Super::Deinitialize();
}

URageSettingsSubsystem* URageSettingsSubsystem::Get(const UObject* WorldContextObject)
{
	if (!IsValid(WorldContextObject))
	{
		return nullptr;
	}
	
	const UWorld* World = WorldContextObject->GetWorld();
	if (!IsValid(World))
	{
		return nullptr;
	}

	const UGameInstance* GameInstance = World->GetGameInstance();
	if (!IsValid(GameInstance))
	{
		return nullptr;
	}

	return GameInstance->GetSubsystem<URageSettingsSubsystem>();
}

URageVideoSettings* URageSettingsSubsystem::GetVideoSettings() const
{
	return VideoSettings;
}

URageAudioSettings* URageSettingsSubsystem::GetAudioSettings() const
{
	return AudioSettings;
}

URageGameSettings* URageSettingsSubsystem::GetGameSettings() const
{
	return GameSettings;
}

URageInputSettings* URageSettingsSubsystem::GetInputSettings() const
{
	return InputSettings;
}

void URageSettingsSubsystem::ApplyAllDirtySettings()
{
	for (const TScriptInterface<IRageSettingsCategoryInterface>& Category : AllCategories)
	{
		if (Category && Category->IsDirty())
		{
			const ERageSettingsCategory Id = Category->GetCategoryId();
			Category->ApplySettings();
			CategoryAppliedDelegate.Broadcast(Id);
		}
	}
}

void URageSettingsSubsystem::SaveAllSettings()
{
	for (const TScriptInterface<IRageSettingsCategoryInterface>& Category : AllCategories)
	{
		if (Category)
		{
			Category->SaveSettings();
		}
	}
}

void URageSettingsSubsystem::ApplyAndSaveAllDirtySettings()
{
	for (const TScriptInterface<IRageSettingsCategoryInterface>& Category : AllCategories)
	{
		if (Category && Category->IsDirty())
		{
			const ERageSettingsCategory Id = Category->GetCategoryId();
			Category->ApplySettings();
			Category->SaveSettings();
			CategoryAppliedDelegate.Broadcast(Id);
		}
	}
}

void URageSettingsSubsystem::RevertAllPendingChanges()
{
	for (const TScriptInterface<IRageSettingsCategoryInterface>& Category : AllCategories)
	{
		if (Category)
		{
			Category->RevertPendingChanges();
		}
	}
}

void URageSettingsSubsystem::ResetCategoryToDefault(ERageSettingsCategory Category, bool bApplyImmediately)
{
	IRageSettingsCategoryInterface* Target = ResolveCategory(Category);
	if (!Target)
	{
		return;
	}

	Target->ResetToDefault();

	if (bApplyImmediately && Target->IsDirty())
	{
		Target->ApplySettings();
		Target->SaveSettings();
		CategoryAppliedDelegate.Broadcast(Category);
	}
}

void URageSettingsSubsystem::ResetAllCategoriesToDefault(bool bApplyImmediately)
{
	for (const TScriptInterface<IRageSettingsCategoryInterface>& Category : AllCategories)
	{
		if (!Category)
		{
			continue;
		}

		Category->ResetToDefault();

		if (bApplyImmediately && Category->IsDirty())
		{
			const ERageSettingsCategory Id = Category->GetCategoryId();
			Category->ApplySettings();
			Category->SaveSettings();
			CategoryAppliedDelegate.Broadcast(Id);
		}
	}
}

bool URageSettingsSubsystem::HasAnyDirtySettings() const
{
	for (const TScriptInterface<IRageSettingsCategoryInterface>& Category : AllCategories)
	{
		if (Category && Category->IsDirty())
		{
			return true;
		}
	}
	return false;
}

bool URageSettingsSubsystem::IsCategoryDirty(ERageSettingsCategory Category) const
{
	const IRageSettingsCategoryInterface* Target = ResolveCategory(Category);
	return Target && Target->IsDirty();
}

void URageSettingsSubsystem::HandleCategoryDirtyStateChanged(ERageSettingsCategory Category, bool bIsDirty)
{
	AnyCategoryDirtyStateChangedDelegate.Broadcast(Category, bIsDirty);
}

IRageSettingsCategoryInterface* URageSettingsSubsystem::ResolveCategory(ERageSettingsCategory Category) const
{
	switch (Category)
	{
		case ERageSettingsCategory::Video: return VideoSettings;
		case ERageSettingsCategory::Audio: return AudioSettings;
		case ERageSettingsCategory::Game:  return GameSettings;
		case ERageSettingsCategory::Input: return InputSettings;
	}
	
	return nullptr;
}
