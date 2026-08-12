// Copyright (c) 2026 Abdallah Boutrif
// SPDX-License-Identifier: MIT

#pragma once

#include "CoreMinimal.h"
#include "RageAntiAliasingMethod.h"
#include "RageFeatureSupport.h"
#include "RageHDRDisplayNits.h"
#include "RageMonitorInfo.h"
#include "RageMSAASampleCount.h"
#include "RageQualityPreset.h"
#include "RageRayTracingSettings.h"
#include "RageRHIType.h"
#include "GameFramework/GameUserSettings.h"
#include "RageSettingsCategoryInterface.h"
#include "RageUpscalerSettings.h"
#include "RageVideoSettings.generated.h"

enum class ERageScalabilityCategory : uint8;

/* Engine treats 0 as uncapped frames*/
#define UNCAPPED_FPS 0.f

/**
 * This requires the GameUserSettings class to be set to this one, either via Project Settings or directly in the DefaultEngine.ini.
 *
 *     [/Script/Engine.Engine]
 *     GameUserSettingsClassName=/Script/RageSettings.RageVideoSettings
 *
 * Projects extend this class by subclassing and pointing GameUserSettingsClassName at their own
 * subclass instead - the same mechanism, just one level deeper. Any Config-flagged field added on
 * the subclass participates in dirty-tracking/apply/save/reset automatically, though it will not
 * appear in the video settings UI without the project also adding a row for it in their own
 * video panel (this panel's rows are hand-authored, since several pull runtime hardware
 * capability data - supported resolutions, DLSS/FSR/XeSS support - that can't be inferred from a
 * UPROPERTY alone).
 */
UCLASS(config = GameUserSettings, configdonotcheckdefaults)
class RAGESETTINGS_API URageVideoSettings : public UGameUserSettings, public IRageSettingsCategoryInterface
{
	GENERATED_BODY()

public:
//~ Begin IRageSettingsCategory
	virtual void LoadSettings() override;
	virtual void ApplySettings() override;
	virtual void SaveSettings() override;
	virtual void ResetToDefault() override;
	virtual void RevertPendingChanges() override;
	virtual bool IsDirty() const override;
	virtual void NotifyPendingChangedExternally(bool bWasDirtyBefore) override { BroadcastDirtyIfChanged(bWasDirtyBefore); }
	virtual ERageSettingsCategory GetCategoryId() const override { return ERageSettingsCategory::Video; }
	virtual bool IsApplyInProgress() const override;
//~ End IRageSettingsCategory

#pragma region SETTERS
	UFUNCTION(BlueprintCallable, Category = "Rage|Video")
	void SetPendingResolution(FIntPoint NewResolution);

	UFUNCTION(BlueprintCallable, Category = "Rage|Video")
	void SetPendingMonitorId(const FString& NewMonitorId);

	UFUNCTION(BlueprintCallable, Category = "Rage|Video")
	void SetPendingWindowMode(EWindowMode::Type NewMode);

	UFUNCTION(BlueprintCallable, Category = "Rage|Video")
	void SetPendingVSyncEnabled(bool bEnabled);

	UFUNCTION(BlueprintCallable, Category = "Rage|Video")
	void SetPendingFrameRateLimit(float NewLimit);

	UFUNCTION(BlueprintCallable, Category = "Rage|Video")
	void SetPendingResolutionScale(float NormalizedScale);

	UFUNCTION(BlueprintCallable, Category = "Rage|Video")
	void SetPendingBrightness(float NewBrightness);

	UFUNCTION(BlueprintCallable, Category = "Rage|Video")
	void SetPendingFieldOfView(float NewFOV);

	UFUNCTION(BlueprintCallable, Category = "Rage|Video")
	void SetPendingHDREnabled(bool bEnabled);

	UFUNCTION(BlueprintCallable, Category = "Rage|Video")
	void SetPendingHDRDisplayNits(ERageHDRDisplayNits NewNits);

	UFUNCTION(BlueprintCallable, Category = "Rage|Video")
	void SetPendingMotionBlurEnabled(bool bEnabled);

	UFUNCTION(BlueprintCallable, Category = "Rage|Video")
	void SetPendingDepthOfFieldEnabled(bool bEnabled);

	UFUNCTION(BlueprintCallable, Category = "Rage|Video")
	void SetPendingFilmGrainEnabled(bool bEnabled);

	UFUNCTION(BlueprintCallable, Category = "Rage|Video")
	void SetPendingChromaticAberrationEnabled(bool bEnabled);

	UFUNCTION(BlueprintCallable, Category = "Rage|Video")
	void SetPendingPreferredRHI(ERageRHIType NewRHI);

	UFUNCTION(BlueprintCallable, Category = "Rage|Video")
	void SetPendingAntiAliasingMethod(ERageAntiAliasingMethod NewMethod);

	UFUNCTION(BlueprintCallable, Category = "Rage|Video")
	void SetPendingMSAASampleCount(ERageMSAASampleCount NewCount);
#pragma endregion

#pragma region GETTERS
	UFUNCTION(BlueprintPure, Category = "Rage|Video")
	bool IsHDRSupported() const;

	UFUNCTION(BlueprintPure, Category = "Rage|Video")
	TArray<FIntPoint> GetSupportedResolutions() const;
	
	UFUNCTION(BlueprintPure, Category = "Rage|Video")
	FIntPoint GetLargestSupportedResolution() const;

	UFUNCTION(BlueprintPure, Category = "Rage|Video")
	TArray<FRageMonitorInfo> GetAvailableMonitors() const;
	
	UFUNCTION(BlueprintPure, Category = "Rage|Video")
	TArray<TEnumAsByte<EWindowMode::Type>> GetAvailableWindowModes() const;

	UFUNCTION(BlueprintPure, Category = "Rage|Video")
	bool IsMonitorSelectionSupported() const;

	UFUNCTION(BlueprintCallable, Category = "Rage|Video")
	void SetPendingQualityPreset(ERageQualityPreset NewPreset);

	UFUNCTION(BlueprintCallable, Category = "Rage|Video")
	void SetPendingScalabilityLevel(ERageScalabilityCategory ScalabilityCategory, int32 Level);

	UFUNCTION(BlueprintPure, Category = "Rage|Video")
	int32 GetPendingScalabilityLevel(ERageScalabilityCategory ScalabilityCategory) const;

	UFUNCTION(BlueprintCallable, Category = "Rage|Video")
	void SetPendingRayTracingSettings(const FRageRayTracingSettings& NewSettings);

	UFUNCTION(BlueprintPure, Category = "Rage|Video")
	const FRageRayTracingSettings& GetPendingRayTracingSettings() const;

	UFUNCTION(BlueprintPure, Category = "Rage|Video")
	bool IsRayTracingSupported() const;

	UFUNCTION(BlueprintCallable, Category = "Rage|Video")
	void SetPendingUpscalerSettings(const FRageUpscalerSettings& NewSettings);

	UFUNCTION(BlueprintPure, Category = "Rage|Video")
	const FRageUpscalerSettings& GetPendingUpscalerSettings() const;

	UFUNCTION(BlueprintPure, Category = "Rage|Video")
	TArray<ERageUpscalerMethod> GetAvailableUpscalerMethods() const;
	
	UFUNCTION(BlueprintCallable, Category = "Rage|Video")
	void ClampUpscalerToSupported();

	UFUNCTION(BlueprintPure, Category = "Rage|Video")
	bool IsDLSSSupported() const;

	UFUNCTION(BlueprintPure, Category = "Rage|Video")
	bool IsDLSSRRSupported() const;

	UFUNCTION(BlueprintPure, Category = "Rage|Video")
	bool IsDLSSFrameGenSupported() const;

	UFUNCTION(BlueprintPure, Category = "Rage|Video")
	bool IsFSRSupported() const;

	UFUNCTION(BlueprintPure, Category = "Rage|Video")
	bool IsXeSSSupported() const;

	UFUNCTION(BlueprintPure, Category = "Rage|Video")
	bool IsXeSSFrameGenSupported() const;

	UFUNCTION(BlueprintPure, Category = "Rage|Video")
	bool IsXeLLSupported() const;

	UFUNCTION(BlueprintPure, Category = "Rage|Video")
	bool IsReflexSupported() const;
	
	UFUNCTION(BlueprintPure, Category = "Rage|Video")
	TArray<ERageAntiAliasingMethod> GetAvailableAntiAliasingMethods() const;

	UFUNCTION(BlueprintPure, Category = "Rage|Video")
	bool IsAntiAliasingMethodSupported(ERageAntiAliasingMethod Method) const;
	
	UFUNCTION(BlueprintPure, Category = "Rage|Video")
	bool IsThirdPartyUpscalerActive() const;

	UFUNCTION(BlueprintPure, Category = "Rage|Video")
	bool IsAntiAliasingMethodOverriddenByUpscaler() const;
	
	UFUNCTION(BlueprintPure, Category = "Rage|Video")
	ERageFeatureSupport QueryDLSSRRSupport() const;

	UFUNCTION(BlueprintPure, Category = "Rage|Video")
	ERageFeatureSupport QueryDLSSFrameGenSupport() const;

	UFUNCTION(BlueprintPure, Category = "Rage|Video")
	ERageFeatureSupport QueryReflexSupport() const;

	UFUNCTION(BlueprintPure, Category = "Rage|Video")
	TArray<ERageDLSSMode> GetSupportedDLSSModes() const;

	UFUNCTION(BlueprintPure, Category = "Rage|Video")
	TArray<ERageXeSSMode> GetSupportedXeSSModes() const;

	UFUNCTION(BlueprintPure, Category = "Rage|Video")
	TArray<ERageFrameGenerationMode> GetSupportedDLSSFrameGenModes() const;

	UFUNCTION(BlueprintPure, Category = "Rage|Video")
	TArray<ERageFrameGenerationMode> GetSupportedXeSSFrameGenModes() const;
	
	UFUNCTION(BlueprintPure, Category = "Rage|Video")
	bool IsRHISelectionSupported() const;
	
	UFUNCTION(BlueprintPure, Category = "Rage|Video")
	TArray<ERageRHIType> GetSelectableRHITypes() const;

	UFUNCTION(BlueprintPure, Category = "Rage|Video")
	ERageRHIType GetActiveRHIType() const;
	
	UFUNCTION(BlueprintPure, Category = "Rage|Video")
	bool IsRestartRequiredForRHI() const;
	
	UFUNCTION(BlueprintPure, Category = "Rage|Video")
	bool IsRestartRequiredForFrameGeneration() const;
	
	UFUNCTION(BlueprintPure, Category = "Rage|Video")
	bool IsRestartRequiredForMonitor() const;

	UFUNCTION(BlueprintPure, Category = "Rage|Video")
	bool IsRestartRequired() const;

	UFUNCTION(BlueprintPure, Category = "Rage|Video")
	URageVideoSettings* GetPendingSettings() const { return Pending; }

	UFUNCTION(BlueprintPure, Category = "Rage|Video")
	URageVideoSettings* GetDefaultSettings() const { return Defaults; }
#pragma endregion

	UPROPERTY(BlueprintAssignable, Category = "Rage|Delegates")
	FRageCategoryDirtyStateChanged DirtyStateChangedDelegate;

	UPROPERTY(Config, BlueprintReadWrite, Category = "Rage|Video|Display")
	FIntPoint Resolution = FIntPoint(1920, 1080);
	
	UPROPERTY(Config, BlueprintReadWrite, Category = "Rage|Video|Display")
	FString PreferredMonitorId;

	UPROPERTY(Config, BlueprintReadWrite, Category = "Rage|Video|Display")
	TEnumAsByte<EWindowMode::Type> WindowMode = EWindowMode::WindowedFullscreen;

	UPROPERTY(Config, BlueprintReadWrite, Category = "Rage|Video|Display")
	bool bVSyncEnabled = false;

	// Named to avoid shadowing UGameUserSettings::FrameRateLimit (this class derives from it).
	UPROPERTY(Config, BlueprintReadWrite, Category = "Rage|Video|Display")
	float TargetFrameRateLimit = UNCAPPED_FPS;

	UPROPERTY(Config, BlueprintReadWrite, Category = "Rage|Video|Display")
	float ResolutionScaleNormalized = 1.f;

	UPROPERTY(Config, BlueprintReadWrite, Category = "Rage|Video|Display")
	float Brightness = 2.2f;

	UPROPERTY(Config, BlueprintReadWrite, Category = "Rage|Video|Display")
	float FieldOfView = 90.f;

	UPROPERTY(Config, BlueprintReadWrite, Category = "Rage|Video|Display")
	bool bHDREnabled = false;

	UPROPERTY(Config, BlueprintReadWrite, Category = "Rage|Video|Display")
	ERageHDRDisplayNits HDRDisplayNits = ERageHDRDisplayNits::Nits1000;
	
	UPROPERTY(Config, BlueprintReadWrite, Category = "Rage|Video|Display")
	ERageRHIType PreferredRHI = ERageRHIType::Auto;

	UPROPERTY(Config, BlueprintReadWrite, Category = "Rage|Video|Scalability")
	ERageQualityPreset QualityPreset = ERageQualityPreset::High;

	UPROPERTY(Config, BlueprintReadWrite, Category = "Rage|Video|Scalability")
	int32 ViewDistanceQuality = 2;

	UPROPERTY(Config, BlueprintReadWrite, Category = "Rage|Video|Scalability")
	int32 AntiAliasingQuality = 2;
	
	UPROPERTY(Config, BlueprintReadWrite, Category = "Rage|Video|Scalability")
	ERageAntiAliasingMethod AntiAliasingMethod = ERageAntiAliasingMethod::TSR;

	UPROPERTY(Config, BlueprintReadWrite, Category = "Rage|Video|Scalability")
	ERageMSAASampleCount MSAASampleCount = ERageMSAASampleCount::x4;

	UPROPERTY(Config, BlueprintReadWrite, Category = "Rage|Video|Scalability")
	int32 ShadowQuality = 2;

	UPROPERTY(Config, BlueprintReadWrite, Category = "Rage|Video|Scalability")
	int32 GlobalIlluminationQuality = 2;

	UPROPERTY(Config, BlueprintReadWrite, Category = "Rage|Video|Scalability")
	int32 ReflectionQuality = 2;

	UPROPERTY(Config, BlueprintReadWrite, Category = "Rage|Video|Scalability")
	int32 PostProcessQuality = 2;

	UPROPERTY(Config, BlueprintReadWrite, Category = "Rage|Video|Scalability")
	int32 TextureQuality = 2;

	UPROPERTY(Config, BlueprintReadWrite, Category = "Rage|Video|Scalability")
	int32 VisualEffectQuality = 2;

	UPROPERTY(Config, BlueprintReadWrite, Category = "Rage|Video|Scalability")
	int32 FoliageQuality = 2;

	UPROPERTY(Config, BlueprintReadWrite, Category = "Rage|Video|Scalability")
	int32 ShadingQuality = 2;

	UPROPERTY(Config, BlueprintReadWrite, Category = "Rage|Video|Upscaler")
	FRageRayTracingSettings RayTracing;

	UPROPERTY(Config, BlueprintReadWrite, Category = "Rage|Video|Upscaler")
	FRageUpscalerSettings Upscaler;

	UPROPERTY(Config, BlueprintReadWrite, Category = "Rage|Video|PostProcess")
	bool bMotionBlurEnabled = true;

	UPROPERTY(Config, BlueprintReadWrite, Category = "Rage|Video|PostProcess")
	bool bDepthOfFieldEnabled = true;

	UPROPERTY(Config, BlueprintReadWrite, Category = "Rage|Video|PostProcess")
	bool bFilmGrainEnabled = false;

	UPROPERTY(Config, BlueprintReadWrite, Category = "Rage|Video|PostProcess")
	bool bChromaticAberrationEnabled = false;

protected:
	UPROPERTY()
	TObjectPtr<URageVideoSettings> Pending = nullptr;

	UPROPERTY()
	TObjectPtr<URageVideoSettings> Defaults = nullptr;

private:
	void PushCurrentIntoEngineProperties();
	void ApplyRayTracingCVars(const FRageRayTracingSettings& Settings);
	void ApplyUpscalerSettings(const FRageUpscalerSettings& Settings);
	void ApplyPostProcessCVars();
	void ApplyAntiAliasingCVars();
	void ClampAntiAliasingMethodToSupported();
	void ApplyPreferredRHI();
	void ApplyPreferredMonitor();
	void RememberInitialWindowPosition(const FString& MonitorId);
	void ClampResolutionToMonitor(const FString& MonitorId);
	void ClampWindowModeToAvailable();
	void DeferredReapplyDisplayMode();
	void DeferredApplyStartupSettings();
	void ReconcilePreferredRHIWithActual();
	void BroadcastDirtyIfChanged(bool bWasDirtyBefore);

	void ClampUpscalerMethodToSupported(FRageUpscalerSettings& Settings) const;

	void DeferredApplyDLSSFrameGeneration(ERageFrameGenerationMode DesiredMode);
	void DeferredApplyXeSSFrameGeneration(ERageFrameGenerationMode DesiredMode);

	FTSTicker::FDelegateHandle DLSSFrameGenTickerHandle;
	FTSTicker::FDelegateHandle XeSSFrameGenTickerHandle;
	FTSTicker::FDelegateHandle StartupApplyTickerHandle;
	FTSTicker::FDelegateHandle DisplayModeTickerHandle;
};
