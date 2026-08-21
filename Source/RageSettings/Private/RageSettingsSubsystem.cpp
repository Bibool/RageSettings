// Copyright (c) 2026 Abdallah Boutrif
// SPDX-License-Identifier: MIT

#include "RageSettingsSubsystem.h"

#include "RageVideoSettings.h"
#include "RageAudioSettings.h"
#include "RageGameSettings.h"
#include "RageInputSettings.h"
#include "RageSettingsDeveloperSettings.h"
#include "RageSettingsReflectionUtils.h"
#include "Engine/Engine.h"
#include "HAL/PlatformProcess.h"
#include "Misc/CommandLine.h"
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
	FTSTicker::RemoveTicker(RestartCheckTickerHandle);
	FTSTicker::RemoveTicker(ApplySettledTickerHandle);
	FTSTicker::RemoveTicker(ChangeBroadcastTickerHandle);

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
	BeginApply();

	for (const TScriptInterface<IRageSettingsCategoryInterface>& Category : AllCategories)
	{
		if (Category && Category->IsDirty())
		{
			ApplyCategoryTrackingChanges(Category.GetInterface(), false);
		}
	}

	DeferredEvaluateRestartRequirement();
	EndApplyWhenSettled();
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
	BeginApply();

	for (const TScriptInterface<IRageSettingsCategoryInterface>& Category : AllCategories)
	{
		if (Category && Category->IsDirty())
		{
			ApplyCategoryTrackingChanges(Category.GetInterface(), true);
		}
	}

	DeferredEvaluateRestartRequirement();
	EndApplyWhenSettled();
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
		BeginApply();

		ApplyCategoryTrackingChanges(Target, true);

		DeferredEvaluateRestartRequirement();
		EndApplyWhenSettled();
	}
}

void URageSettingsSubsystem::ResetAllCategoriesToDefault(bool bApplyImmediately)
{
	if (bApplyImmediately)
	{
		BeginApply();
	}

	for (const TScriptInterface<IRageSettingsCategoryInterface>& Category : AllCategories)
	{
		if (!Category)
		{
			continue;
		}

		Category->ResetToDefault();

		if (bApplyImmediately && Category->IsDirty())
		{
			ApplyCategoryTrackingChanges(Category.GetInterface(), true);
		}
	}

	if (bApplyImmediately)
	{
		DeferredEvaluateRestartRequirement();
		EndApplyWhenSettled();
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

bool URageSettingsSubsystem::IsRestartRequired() const
{
	return IsValid(VideoSettings) && VideoSettings->IsRestartRequired();
}

void URageSettingsSubsystem::DeferredEvaluateRestartRequirement()
{
	/* Aggregate latent calls*/
	FTSTicker::RemoveTicker(RestartCheckTickerHandle);
	RestartCheckTickerHandle = FTSTicker::GetCoreTicker().AddTicker(FTickerDelegate::CreateWeakLambda(this,
		[this](float) -> bool
		{
			RestartRequirementEvaluatedDelegate.Broadcast(IsRestartRequired());
			return false;
		}));
}

void URageSettingsSubsystem::BeginApply()
{
	if (bApplyInProgress)
	{
		return;
	}

	bApplyInProgress = true;
	ApplyStartedDelegate.Broadcast();
}

void URageSettingsSubsystem::EndApplyWhenSettled()
{
	if (!bApplyInProgress)
	{
		return;
	}

	ApplyHoldRemaining = FMath::Max(0.f, SETTINGS->ApplyFinishedAdditionalHoldSeconds);

	if (ApplyHoldRemaining <= 0.f && !IsAnyCategoryApplying())
	{
		FinishApply();
		return;
	}

	FTSTicker::RemoveTicker(ApplySettledTickerHandle);
	ApplySettledTickerHandle = FTSTicker::GetCoreTicker().AddTicker(FTickerDelegate::CreateWeakLambda(this,
		[this](float DeltaTime) -> bool
		{
			if (IsAnyCategoryApplying())
			{
				return true;
			}

			ApplyHoldRemaining -= DeltaTime;
			if (ApplyHoldRemaining > 0.f)
			{
				return true;
			}

			FinishApply();
			return false;
		}));
}

void URageSettingsSubsystem::FinishApply()
{
	ApplyHoldRemaining = 0.f;
	bApplyInProgress = false;
	ApplyFinishedDelegate.Broadcast();
}

bool URageSettingsSubsystem::IsAnyCategoryApplying() const
{
	for (const TScriptInterface<IRageSettingsCategoryInterface>& Category : AllCategories)
	{
		if (Category && Category->IsApplyInProgress())
		{
			return true;
		}
	}

	return false;
}

void URageSettingsSubsystem::ApplyCategoryTrackingChanges(IRageSettingsCategoryInterface* Category, bool bSaveAfterApply)
{
	/* Read before the apply. Afterwards Pending has been copied over the live object and the two
	 * agree, so there is no longer anything to tell apart. */
	TArray<FName> Changed = RageSettings::CollectChangedProperties(Category->_getUObject(), Category->GetPendingObject());

	const ERageSettingsCategory Id = Category->GetCategoryId();

	Category->ApplySettings();

	if (bSaveAfterApply)
	{
		Category->SaveSettings();
	}

	CategoryAppliedDelegate.Broadcast(Id);

	QueueChangeBroadcast(Id, MoveTemp(Changed));
}

void URageSettingsSubsystem::QueueChangeBroadcast(ERageSettingsCategory Category, TArray<FName>&& ChangedProperties)
{
	if (ChangedProperties.IsEmpty())
	{
		return;
	}

	PendingChangeBroadcasts.Add({Category, MoveTemp(ChangedProperties)});

	/* A category that had nothing to defer is told about in this frame. Holding it until the
	 * subsystem's own apply finishes would put every listener behind whichever category is slowest,
	 * and behind ApplyFinishedAdditionalHoldSeconds, which is there to pace the UI and nothing more. */
	FlushSettledChangeBroadcasts();

	if (PendingChangeBroadcasts.IsEmpty() || ChangeBroadcastTickerHandle.IsValid())
	{
		return;
	}

	ChangeBroadcastTickerHandle = FTSTicker::GetCoreTicker().AddTicker(FTickerDelegate::CreateWeakLambda(this,
		[this](float) -> bool
		{
			FlushSettledChangeBroadcasts();

			if (!PendingChangeBroadcasts.IsEmpty())
			{
				return true;
			}

			ChangeBroadcastTickerHandle.Reset();
			return false;
		}));
}

void URageSettingsSubsystem::FlushSettledChangeBroadcasts()
{
	/* A listener is free to apply something of its own, which lands back here partway through the
	 * walk below. Letting that inner call run would have it removing entries this one has not
	 * reached yet; whatever it queued is picked up before this loop ends anyway. */
	if (bFlushingChangeBroadcasts)
	{
		return;
	}

	TGuardValue<bool> FlushGuard(bFlushingChangeBroadcasts, true);

	for (int32 Index = 0; Index < PendingChangeBroadcasts.Num(); )
	{
		const IRageSettingsCategoryInterface* Target = ResolveCategory(PendingChangeBroadcasts[Index].Category);
		if (Target && Target->IsApplyInProgress())
		{
			++Index;
			continue;
		}

		/* Off the list before anyone is told, so a listener that applies something of its own does
		 * not find this entry still waiting and broadcast it a second time. */
		const FPendingChangeBroadcast Settled = MoveTemp(PendingChangeBroadcasts[Index]);
		PendingChangeBroadcasts.RemoveAt(Index);

		BroadcastCategoryChanges(Settled.Category, Settled.ChangedProperties);
	}
}

void URageSettingsSubsystem::BroadcastCategoryChanges(ERageSettingsCategory Category, const TArray<FName>& ChangedProperties)
{
	IRageSettingsCategoryInterface* Target = ResolveCategory(Category);
	if (!Target)
	{
		return;
	}

	Target->GetChangeRegistry().Broadcast(Category, ChangedProperties);

	for (const FName& PropertyName : ChangedProperties)
	{
		SettingChangedDelegate.Broadcast(Category, PropertyName);
	}
}

bool URageSettingsSubsystem::RestartGame()
{
	if (GIsEditor)
	{
		S_LOG(Warning, "Rage Settings: RestartGame() does nothing in the editor. The setting is written - launch a standalone or packaged build to see it take effect.");
		return false;
	}

	const FString ExecutablePath = FPlatformProcess::ExecutablePath();
	
	const FString Parameters = FCommandLine::GetOriginal();

	FProcHandle Handle = FPlatformProcess::CreateProc(*ExecutablePath, *Parameters, true, false, false, nullptr, 0, nullptr, nullptr);
	if (!Handle.IsValid())
	{
		S_LOG(Error, "Rage Settings: could not relaunch {path}, staying in the current process.", ExecutablePath);
		return false;
	}

	FPlatformProcess::CloseProc(Handle);

	S_LOG(Log, "Rage Settings: relaunched {path}, shutting this process down.", ExecutablePath);

	/* Might require more work with steam, not sure steam SDK would be happy with a direct exe restart over Url lunch path, tbd.*/
	FPlatformMisc::RequestExit(false, TEXT("URageSettingsSubsystem::RestartGame"));
	return true;
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
