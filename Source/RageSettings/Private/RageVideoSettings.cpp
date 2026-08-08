// Copyright (c) 2026 Abdallah Boutrif
// SPDX-License-Identifier: MIT

#include "RageVideoSettings.h"

#include "RageSettingsDeveloperSettings.h"
#include "RageRHISupport.h"
#include "RageScalabilityCategory.h"
#include "RageSettingsReflectionUtils.h"
#include "RHI.h"
#include "RenderUtils.h"
#include "SceneUtils.h"
#include "Scalability.h"
#include "HAL/IConsoleManager.h"
#include "Misc/Optional.h"
#include "Containers/Ticker.h"
#include "Kismet/KismetSystemLibrary.h"
#include "RageSettingsShared/Public/RageSettingsSharedDebug.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(RageVideoSettings)

#if WITH_DLSS
#include "DLSSLibrary.h"
#endif
#if WITH_STREAMLINE_DLSSG
#include "StreamlineLibraryDLSSG.h"
#endif
#if WITH_STREAMLINE_REFLEX
#include "StreamlineLibraryReflex.h"
#endif
#if WITH_FSR
#include "FFXFSRSettings.h"
#endif
#if WITH_XESS
#include "XeSSBlueprintLibrary.h"
#endif
#if WITH_XEFG
#include "XeFGBlueprintLibrary.h"
#endif
#if WITH_XELL
#include "XeLLBlueprintLibrary.h"
#endif

namespace RageVideoCVars
{
	/* UE Native Cvars */
	static const TCHAR* RayTracingMaster = TEXT("r.RayTracing.Enable");
	static const TCHAR* RayTracingShadows = TEXT("r.RayTracing.Shadows");
	static const TCHAR* RayTracingReflections = TEXT("r.RayTracing.Reflections");
	static const TCHAR* RayTracingGI = TEXT("r.RayTracing.GlobalIllumination");
	static const TCHAR* RayTracingAO = TEXT("r.RayTracing.AmbientOcclusion");
	static const TCHAR* RayTracingTranslucency = TEXT("r.RayTracing.Translucency");
	static const TCHAR* MotionBlurQuality = TEXT("r.MotionBlurQuality");
	static const TCHAR* DepthOfFieldQuality = TEXT("r.DepthOfFieldQuality");
	static const TCHAR* FilmGrain = TEXT("r.FilmGrain");
	static const TCHAR* SceneColorFringe = TEXT("r.SceneColorFringeQuality");
	static const TCHAR* Gamma = TEXT("r.Gamma");
	static const TCHAR* ScreenPercentage = TEXT("r.ScreenPercentage");
	static const TCHAR* AntiAliasingMethod = TEXT("r.AntiAliasingMethod");
	static const TCHAR* MSAACount = TEXT("r.MSAACount");

	/* Vendor plugin Cvars */
	static const TCHAR* DLSSEnable = TEXT("r.NGX.DLSS.Enable");
	static const TCHAR* FSREnabled = TEXT("r.FidelityFX.FSR.Enabled");
	static const TCHAR* FSRQualityMode = TEXT("r.FidelityFX.FSR.QualityMode");
	static const TCHAR* FSRSharpness = TEXT("r.FidelityFX.FSR.Sharpness");
	static const TCHAR* FSRFrameInterpolation = TEXT("r.FidelityFX.FI.Enabled");
	static const TCHAR* ReflexMode = TEXT("r.Reflex.Mode");
	static const TCHAR* XeSSEnable = TEXT("r.XeSS.Enabled");

	static void SetInt(const TCHAR* Name, int32 Value)
	{
		if (IConsoleVariable* CVar = IConsoleManager::Get().FindConsoleVariable(Name))
		{
			CVar->Set(Value, ECVF_SetByGameSetting);
		}
		else
		{
			S_LOG(Verbose, "Rage Settings: Cvar {name} not found, skipping (plugin/feature not present?)", Name);
		}
	}

	static void SetFloat(const TCHAR* Name, float Value)
	{
		if (IConsoleVariable* CVar = IConsoleManager::Get().FindConsoleVariable(Name))
		{
			CVar->Set(Value, ECVF_SetByGameSetting);
		}
		else
		{
			S_LOG(Verbose, "Rage Settings: Cvar {name} not found, skipping (plugin/feature not present?)", Name);
		}
	}

	static void SetBool(const TCHAR* Name, bool bValue)
	{
		SetInt(Name, bValue);
	}
}


namespace RageAntiAliasingMapping
{
	static int32 ToVendor(ERageAntiAliasingMethod Method)
	{
		switch (Method)
		{
			case ERageAntiAliasingMethod::None: return AAM_None;
			case ERageAntiAliasingMethod::FXAA: return AAM_FXAA;
			case ERageAntiAliasingMethod::TAA:  return AAM_TemporalAA;
			case ERageAntiAliasingMethod::MSAA: return AAM_MSAA;
			case ERageAntiAliasingMethod::TSR:  return AAM_TSR;
			case ERageAntiAliasingMethod::SMAA: return AAM_SMAA;
		}
		return AAM_TSR;
	}

	static int32 ToSampleCount(ERageMSAASampleCount Count)
	{
		switch (Count)
		{
			case ERageMSAASampleCount::x2: return 2;
			case ERageMSAASampleCount::x4: return 4;
			case ERageMSAASampleCount::x8: return 8;
		}
		return 4;
	}
}

/* Converts Rage <-> vendor enums. This is because each vendor (ofc) has different ordering or extra values, therefore we must never StaticCast between both. */
namespace RageUpscalerMapping
{
#if WITH_DLSS
	static UDLSSMode ToVendor(ERageDLSSMode Mode)
	{
		switch (Mode)
		{
			case ERageDLSSMode::UltraPerformance: return UDLSSMode::UltraPerformance;
			case ERageDLSSMode::Performance:      return UDLSSMode::Performance;
			case ERageDLSSMode::Balanced:         return UDLSSMode::Balanced;
			case ERageDLSSMode::Quality:          return UDLSSMode::Quality;
			case ERageDLSSMode::UltraQuality:     return UDLSSMode::UltraQuality;
			case ERageDLSSMode::DLAA:             return UDLSSMode::DLAA;
		}
		return UDLSSMode::Off;
	}

	/* Off/Auto have no Rage equivalent (Off is modeled via ERageUpscalerMethod, Auto is a screen-percentage query helper, not a real mode) unset means "don't offer this". */
	static TOptional<ERageDLSSMode> FromVendor(UDLSSMode Mode)
	{
		switch (Mode)
		{
			case UDLSSMode::UltraPerformance: return ERageDLSSMode::UltraPerformance;
			case UDLSSMode::Performance:      return ERageDLSSMode::Performance;
			case UDLSSMode::Balanced:         return ERageDLSSMode::Balanced;
			case UDLSSMode::Quality:          return ERageDLSSMode::Quality;
			case UDLSSMode::UltraQuality:     return ERageDLSSMode::UltraQuality;
			case UDLSSMode::DLAA:             return ERageDLSSMode::DLAA;
			default:                          return NullOpt;
		}
	}
#endif

#if WITH_DLSS
	static ERageFeatureSupport FromVendor(UDLSSSupport Support)
	{
		switch (Support)
		{
			case UDLSSSupport::Supported:                            return ERageFeatureSupport::Supported;
			case UDLSSSupport::NotSupportedIncompatibleHardware:     return ERageFeatureSupport::IncompatibleHardware;
			case UDLSSSupport::NotSupportedDriverOutOfDate:          return ERageFeatureSupport::DriverOutOfDate;
			case UDLSSSupport::NotSupportedOperatingSystemOutOfDate: return ERageFeatureSupport::OperatingSystemOutOfDate;
			default:                                                 return ERageFeatureSupport::NotSupported;
		}
	}
#endif

#if WITH_STREAMLINE_DLSSG || WITH_STREAMLINE_REFLEX
	static ERageFeatureSupport FromVendorStreamline(EStreamlineFeatureSupport Support)
	{
		switch (Support)
		{
			case EStreamlineFeatureSupport::Supported:                             		return ERageFeatureSupport::Supported;
			case EStreamlineFeatureSupport::NotSupportedIncompatibleHardware:      		return ERageFeatureSupport::IncompatibleHardware;
			case EStreamlineFeatureSupport::NotSupportedDriverOutOfDate:           		return ERageFeatureSupport::DriverOutOfDate;
			case EStreamlineFeatureSupport::NotSupportedOperatingSystemOutOfDate:  		return ERageFeatureSupport::OperatingSystemOutOfDate;
			case EStreamlineFeatureSupport::NotSupportedByRHI:                     		return ERageFeatureSupport::NotSupportedByRHI;
			case EStreamlineFeatureSupport::NotSupportedHardewareSchedulingDisabled:	return ERageFeatureSupport::HardwareSchedulingDisabled;
			default:																	return ERageFeatureSupport::NotSupported;
		}
	}
#endif

#if WITH_STREAMLINE_DLSSG
	static EStreamlineDLSSGMode ToVendorDLSSG(ERageFrameGenerationMode Mode)
	{
		switch (Mode)
		{
			case ERageFrameGenerationMode::x2: return EStreamlineDLSSGMode::On2X;
			case ERageFrameGenerationMode::x3: return EStreamlineDLSSGMode::On3X;
			case ERageFrameGenerationMode::x4: return EStreamlineDLSSGMode::On4X;
			default:                           return EStreamlineDLSSGMode::Off;
		}
	}

	/* Auto/OnDynamic/On5X/On6X have no Rage equivalent - Plugin only exposes fixed x2/x3/x4 tiers. */
	static TOptional<ERageFrameGenerationMode> FromVendorDLSSG(EStreamlineDLSSGMode Mode)
	{
		switch (Mode)
		{
			case EStreamlineDLSSGMode::Off:  return ERageFrameGenerationMode::Off;
			case EStreamlineDLSSGMode::On2X: return ERageFrameGenerationMode::x2;
			case EStreamlineDLSSGMode::On3X: return ERageFrameGenerationMode::x3;
			case EStreamlineDLSSGMode::On4X: return ERageFrameGenerationMode::x4;
			default:                         return NullOpt;
		}
	}
#endif

#if WITH_FSR
	static int32 ToVendor(ERageFSRMode Mode)
	{
		switch (Mode)
		{
			case ERageFSRMode::UltraPerformance: return StaticCast<int32>(EFFXFSRQualityMode::UltraPerformance);
			case ERageFSRMode::Performance:      return StaticCast<int32>(EFFXFSRQualityMode::Performance);
			case ERageFSRMode::Balanced:         return StaticCast<int32>(EFFXFSRQualityMode::Balanced);
			case ERageFSRMode::Quality:          return StaticCast<int32>(EFFXFSRQualityMode::Quality);
			case ERageFSRMode::NativeAA:         return StaticCast<int32>(EFFXFSRQualityMode::NativeAA);
		}
		return StaticCast<int32>(EFFXFSRQualityMode::Quality);
	}
#endif

#if WITH_XESS
	static EXeSSQualityMode ToVendor(ERageXeSSMode Mode)
	{
		switch (Mode)
		{
			case ERageXeSSMode::UltraPerformance: return EXeSSQualityMode::UltraPerformance;
			case ERageXeSSMode::Performance:      return EXeSSQualityMode::Performance;
			case ERageXeSSMode::Balanced:         return EXeSSQualityMode::Balanced;
			case ERageXeSSMode::Quality:          return EXeSSQualityMode::Quality;
			case ERageXeSSMode::UltraQuality:     return EXeSSQualityMode::UltraQuality;
			case ERageXeSSMode::UltraQualityPlus: return EXeSSQualityMode::UltraQualityPlus;
			case ERageXeSSMode::AntiAliasing:     return EXeSSQualityMode::AntiAliasing;
		}
		return EXeSSQualityMode::Off;
	}

	/** Off has no Rage equivalent (modeled via ERageUpscalerMethod). */
	static TOptional<ERageXeSSMode> FromVendor(EXeSSQualityMode Mode)
	{
		switch (Mode)
		{
			case EXeSSQualityMode::UltraPerformance: return ERageXeSSMode::UltraPerformance;
			case EXeSSQualityMode::Performance:      return ERageXeSSMode::Performance;
			case EXeSSQualityMode::Balanced:         return ERageXeSSMode::Balanced;
			case EXeSSQualityMode::Quality:          return ERageXeSSMode::Quality;
			case EXeSSQualityMode::UltraQuality:     return ERageXeSSMode::UltraQuality;
			case EXeSSQualityMode::UltraQualityPlus: return ERageXeSSMode::UltraQualityPlus;
			case EXeSSQualityMode::AntiAliasing:     return ERageXeSSMode::AntiAliasing;
			default:                                 return NullOpt;
		}
	}
#endif

#if WITH_XEFG
	static EXeFGMode ToVendorXeFG(ERageFrameGenerationMode Mode)
	{
		switch (Mode)
		{
			case ERageFrameGenerationMode::x2: return EXeFGMode::On2x;
			case ERageFrameGenerationMode::x3: return EXeFGMode::On3x;
			case ERageFrameGenerationMode::x4: return EXeFGMode::On4x;
			default:                           return EXeFGMode::Off;
		}
	}

	static TOptional<ERageFrameGenerationMode> FromVendorXeFG(EXeFGMode Mode)
	{
		switch (Mode)
		{
			case EXeFGMode::Off:  return ERageFrameGenerationMode::Off;
			/** "On" is a legacy alias kept for backward compatibility - it interpolates 1 frame,
			* same as On2x, so it maps to the same Rage tier. */
			case EXeFGMode::On:
			case EXeFGMode::On2x: return ERageFrameGenerationMode::x2;
			case EXeFGMode::On3x: return ERageFrameGenerationMode::x3;
			case EXeFGMode::On4x: return ERageFrameGenerationMode::x4;
		}
		return NullOpt;
	}
#endif
}

void URageVideoSettings::LoadSettings()
{
	Super::LoadSettings(true);

	ReconcilePreferredRHIWithActual();

	Pending = CastChecked<URageVideoSettings>(RageSettings::CreateShadowInstance(this, this));
	Defaults = CastChecked<URageVideoSettings>(RageSettings::CreateShadowInstance(this, GetClass()->GetDefaultObject()));

	PushCurrentIntoEngineProperties();
	DeferredApplyStartupSettings();
}

void URageVideoSettings::ApplySettings()
{
	RageSettings::CopyObjectProperties(this, Pending);
	ClampUpscalerMethodToSupported(Upscaler);
	PushCurrentIntoEngineProperties();

	Super::ApplySettings(false);

	ApplyRayTracingCVars(RayTracing);
	ApplyUpscalerSettings(Upscaler);
	ApplyPostProcessCVars();
	ApplyAntiAliasingCVars();
	ApplyPreferredRHI();
	RageVideoCVars::SetFloat(RageVideoCVars::Gamma, Brightness);

	const int32 DisplayNits = RageHDRDisplayNits::ToInt32(HDRDisplayNits);
	EnableHDRDisplayOutput(bHDREnabled, DisplayNits);
	
	RageSettings::CopyObjectProperties(Pending, this);

	DirtyStateChangedDelegate.Broadcast(ERageSettingsCategory::Video, false);
}

void URageVideoSettings::SaveSettings()
{
	Super::SaveSettings();
}

void URageVideoSettings::ResetToDefault()
{
	const bool bWasDirty = IsDirty();
	RageSettings::CopyObjectProperties(Pending, Defaults);
	BroadcastDirtyIfChanged(bWasDirty);
}

void URageVideoSettings::RevertPendingChanges()
{
	const bool bWasDirty = IsDirty();
	RageSettings::CopyObjectProperties(Pending, this);
	BroadcastDirtyIfChanged(bWasDirty);
}

bool URageVideoSettings::IsDirty() const
{
	return !RageSettings::AreObjectsEqual(this, Pending);
}

void URageVideoSettings::SetPendingResolution(FIntPoint NewResolution)
{
	const bool bWasDirty = IsDirty();
	Pending->Resolution = NewResolution;
	BroadcastDirtyIfChanged(bWasDirty);
}

void URageVideoSettings::SetPendingWindowMode(EWindowMode::Type NewMode)
{
	const bool bWasDirty = IsDirty();
	Pending->WindowMode = NewMode;
	BroadcastDirtyIfChanged(bWasDirty);
}

void URageVideoSettings::SetPendingVSyncEnabled(bool bEnabled)
{
	const bool bWasDirty = IsDirty();
	Pending->bVSyncEnabled = bEnabled;
	BroadcastDirtyIfChanged(bWasDirty);
}

void URageVideoSettings::SetPendingFrameRateLimit(float NewLimit)
{
	const bool bWasDirty = IsDirty();
	Pending->TargetFrameRateLimit = FMath::Max(0.f, NewLimit);
	BroadcastDirtyIfChanged(bWasDirty);
}

void URageVideoSettings::SetPendingResolutionScale(float NormalizedScale)
{
	const bool bWasDirty = IsDirty();
	Pending->ResolutionScaleNormalized = FMath::Clamp(NormalizedScale, 0.1f, 1.f);
	BroadcastDirtyIfChanged(bWasDirty);
}

void URageVideoSettings::SetPendingBrightness(float NewBrightness)
{
	const bool bWasDirty = IsDirty();
	Pending->Brightness = FMath::Clamp(NewBrightness, SETTINGS->BrightnessMin, SETTINGS->BrightnessMax);
	BroadcastDirtyIfChanged(bWasDirty);
}

void URageVideoSettings::SetPendingFieldOfView(float NewFOV)
{
	const bool bWasDirty = IsDirty();
	Pending->FieldOfView = FMath::Clamp(NewFOV, SETTINGS->FieldOfViewMin, SETTINGS->FieldOfViewMax);
	BroadcastDirtyIfChanged(bWasDirty);
}

void URageVideoSettings::SetPendingHDREnabled(bool bEnabled)
{
	const bool bWasDirty = IsDirty();
	Pending->bHDREnabled = bEnabled;
	BroadcastDirtyIfChanged(bWasDirty);
}

void URageVideoSettings::SetPendingHDRDisplayNits(ERageHDRDisplayNits NewNits)
{
	const bool bWasDirty = IsDirty();
	Pending->HDRDisplayNits = NewNits;
	BroadcastDirtyIfChanged(bWasDirty);
}

void URageVideoSettings::SetPendingMotionBlurEnabled(bool bEnabled)
{
	const bool bWasDirty = IsDirty();
	Pending->bMotionBlurEnabled = bEnabled;
	BroadcastDirtyIfChanged(bWasDirty);
}

void URageVideoSettings::SetPendingDepthOfFieldEnabled(bool bEnabled)
{
	const bool bWasDirty = IsDirty();
	Pending->bDepthOfFieldEnabled = bEnabled;
	BroadcastDirtyIfChanged(bWasDirty);
}

void URageVideoSettings::SetPendingFilmGrainEnabled(bool bEnabled)
{
	const bool bWasDirty = IsDirty();
	Pending->bFilmGrainEnabled = bEnabled;
	BroadcastDirtyIfChanged(bWasDirty);
}

void URageVideoSettings::SetPendingChromaticAberrationEnabled(bool bEnabled)
{
	const bool bWasDirty = IsDirty();
	Pending->bChromaticAberrationEnabled = bEnabled;
	BroadcastDirtyIfChanged(bWasDirty);
}

void URageVideoSettings::SetPendingAntiAliasingMethod(ERageAntiAliasingMethod NewMethod)
{
	if (!IsAntiAliasingMethodSupported(NewMethod))
	{
		S_LOG(Warning, "Rage Settings: refusing to stage an unsupported anti-aliasing method, the renderer would silently swap it out.");
		return;
	}

	const bool bWasDirty = IsDirty();
	Pending->AntiAliasingMethod = NewMethod;
	BroadcastDirtyIfChanged(bWasDirty);
}

void URageVideoSettings::SetPendingMSAASampleCount(ERageMSAASampleCount NewCount)
{
	const bool bWasDirty = IsDirty();
	Pending->MSAASampleCount = NewCount;
	BroadcastDirtyIfChanged(bWasDirty);
}

void URageVideoSettings::SetPendingPreferredRHI(ERageRHIType NewRHI)
{
	if (!RageRHI::IsTypeSelectable(NewRHI))
	{
		S_LOG(Warning, "Rage Settings: refusing to stage an unselectable graphics API - this build or this machine cannot boot it.");
		return;
	}

	const bool bWasDirty = IsDirty();
	Pending->PreferredRHI = NewRHI;
	BroadcastDirtyIfChanged(bWasDirty);
}

bool URageVideoSettings::IsHDRSupported() const
{
	/* Worth noting regarding HDR, your screen could support it, but if not turned on it will return false here.
	 * UE has no way of forcing the monitor to output HDR, meaning the user should close game, enable HDR, and re-open the game. */
	return Super::SupportsHDRDisplayOutput();
}

TArray<FIntPoint> URageVideoSettings::GetSupportedResolutions() const
{
	TArray<FIntPoint> Resolutions;
	UKismetSystemLibrary::GetSupportedFullscreenResolutions(Resolutions);
	return Resolutions;
}

void URageVideoSettings::SetPendingQualityPreset(ERageQualityPreset NewPreset)
{
	const bool bWasDirty = IsDirty();
	Pending->QualityPreset = NewPreset;

	if (NewPreset != ERageQualityPreset::Custom)
	{
		const int32 Level = StaticCast<int32>(NewPreset);
		Pending->ViewDistanceQuality = Level;
		Pending->AntiAliasingQuality = Level;
		Pending->ShadowQuality = Level;
		Pending->GlobalIlluminationQuality = Level;
		Pending->ReflectionQuality = Level;
		Pending->PostProcessQuality = Level;
		Pending->TextureQuality = Level;
		Pending->VisualEffectQuality = Level;
		Pending->FoliageQuality = Level;
		Pending->ShadingQuality = Level;
	}

	BroadcastDirtyIfChanged(bWasDirty);
}

void URageVideoSettings::SetPendingScalabilityLevel(ERageScalabilityCategory ScalabilityCategory, int32 Level)
{
	const bool bWasDirty = IsDirty();
	Level = FMath::Clamp(Level, RageQualityPreset::Min, RageQualityPreset::MaxQuality);

	switch (ScalabilityCategory)
	{
		case ERageScalabilityCategory::ViewDistance:       Pending->ViewDistanceQuality = Level;		break;
		case ERageScalabilityCategory::AntiAliasing:       Pending->AntiAliasingQuality = Level;		break;
		case ERageScalabilityCategory::Shadow:             Pending->ShadowQuality = Level;				break;
		case ERageScalabilityCategory::GlobalIllumination: Pending->GlobalIlluminationQuality = Level;	break;
		case ERageScalabilityCategory::Reflection:         Pending->ReflectionQuality = Level;			break;
		case ERageScalabilityCategory::PostProcess:        Pending->PostProcessQuality = Level;			break;
		case ERageScalabilityCategory::Texture:            Pending->TextureQuality = Level;				break;
		case ERageScalabilityCategory::Effects:            Pending->VisualEffectQuality = Level;		break;
		case ERageScalabilityCategory::Foliage:            Pending->FoliageQuality = Level; 			break;
		case ERageScalabilityCategory::Shading:            Pending->ShadingQuality = Level; 			break;
	}

	Pending->QualityPreset = ERageQualityPreset::Custom;

	BroadcastDirtyIfChanged(bWasDirty);
}

int32 URageVideoSettings::GetPendingScalabilityLevel(ERageScalabilityCategory ScalabilityCategory) const
{
	switch (ScalabilityCategory)
	{
		case ERageScalabilityCategory::ViewDistance:       return Pending->ViewDistanceQuality;
		case ERageScalabilityCategory::AntiAliasing:       return Pending->AntiAliasingQuality;
		case ERageScalabilityCategory::Shadow:             return Pending->ShadowQuality;
		case ERageScalabilityCategory::GlobalIllumination: return Pending->GlobalIlluminationQuality;
		case ERageScalabilityCategory::Reflection:         return Pending->ReflectionQuality;
		case ERageScalabilityCategory::PostProcess:        return Pending->PostProcessQuality;
		case ERageScalabilityCategory::Texture:            return Pending->TextureQuality;
		case ERageScalabilityCategory::Effects:            return Pending->VisualEffectQuality;
		case ERageScalabilityCategory::Foliage:            return Pending->FoliageQuality;
		case ERageScalabilityCategory::Shading:            return Pending->ShadingQuality;
	}
	
	return 0;
}

void URageVideoSettings::SetPendingRayTracingSettings(const FRageRayTracingSettings& NewSettings)
{
	const bool bWasDirty = IsDirty();
	Pending->RayTracing = NewSettings;
	BroadcastDirtyIfChanged(bWasDirty);
}

const FRageRayTracingSettings& URageVideoSettings::GetPendingRayTracingSettings() const
{
	return Pending->RayTracing;
}

bool URageVideoSettings::IsRayTracingSupported() const
{
	return GIsRHIInitialized && IsRayTracingAllowed();
}

void URageVideoSettings::SetPendingUpscalerSettings(const FRageUpscalerSettings& NewSettings)
{
	const bool bWasDirty = IsDirty();
	Pending->Upscaler = NewSettings;
	BroadcastDirtyIfChanged(bWasDirty);
}

const FRageUpscalerSettings& URageVideoSettings::GetPendingUpscalerSettings() const
{
	return Pending->Upscaler;
}

TArray<ERageUpscalerMethod> URageVideoSettings::GetAvailableUpscalerMethods() const
{
	TArray<ERageUpscalerMethod> Methods = { ERageUpscalerMethod::Off };

	if (IsFSRSupported())
	{
		Methods.Add(ERageUpscalerMethod::FSR);
	}
	if (IsDLSSSupported())
	{
		Methods.Add(ERageUpscalerMethod::DLSS);
	}
	if (IsXeSSSupported())
	{
		Methods.Add(ERageUpscalerMethod::XeSS);
	}
	return Methods;
}

void URageVideoSettings::ClampUpscalerToSupported()
{
	ClampUpscalerMethodToSupported(Upscaler);
	
	if (IsValid(Pending))
	{
		ClampUpscalerMethodToSupported(Pending->Upscaler);
	}
}

bool URageVideoSettings::IsDLSSSupported() const
{
#if WITH_DLSS
	return UDLSSLibrary::IsDLSSSupported();
#else
	/** Without the plugin linked we can't query the hardware/driver truthfully, so default to
	* "not supported" rather than let the UI offer an option that will silently no-op. */
	return false;
#endif
}

bool URageVideoSettings::IsDLSSRRSupported() const
{
#if WITH_DLSS
	return UDLSSLibrary::IsDLSSSupported() && UDLSSLibrary::IsDLSSRRSupported();
#else
	return false;
#endif
}

bool URageVideoSettings::IsDLSSFrameGenSupported() const
{
#if WITH_STREAMLINE_DLSSG
	return UStreamlineLibraryDLSSG::IsDLSSGSupported();
#else
	return false;
#endif
}

bool URageVideoSettings::IsFSRSupported() const
{
#if WITH_FSR
	/** FSR is hardware-agnostic - it runs on any vendor's GPU and internally downgrades to a version
	 * the card can handle (FSR4 being the exception that does need specific hardware). It is not
	 * RHI-agnostic though; the FFX plugin only ships a D3D12 backend with no support for D3D11 nor Vulkan.
	 * This wasn't an issue before as game only ran in auto RHI (d3d12), but now we can switch so we must account for it. */
	return RageRHI::GetActiveType() == ERageRHIType::DirectX12;
#else
	return false;
#endif
}

bool URageVideoSettings::IsReflexSupported() const
{
#if WITH_STREAMLINE_REFLEX
	return UStreamlineLibraryReflex::IsReflexSupported();
#else
	return false;
#endif
}

bool URageVideoSettings::IsXeSSSupported() const
{
#if WITH_XESS
	return UXeSSBlueprintLibrary::IsXeSSSupported();
#else
	return false;
#endif
}

bool URageVideoSettings::IsXeSSFrameGenSupported() const
{
#if WITH_XEFG
	return UXeFGBlueprintLibrary::IsXeFGSupported();
#else
	return false;
#endif
}

bool URageVideoSettings::IsXeLLSupported() const
{
#if WITH_XELL
	return UXeLLBlueprintLibrary::IsXeLLSupported() && UXeLLBlueprintLibrary::IsXeLLAvailable();
#else
	return false;
#endif
}

ERageFeatureSupport URageVideoSettings::QueryDLSSRRSupport() const
{
#if WITH_DLSS
	/** D3D12 only as NGXD3D12RHI.cpp:57 returns true where NGXD3D11RHI.cpp:54 and NGXVulkanRHI.cpp:55
	 * return false. Checked here because UDLSSSupport has no by-RHI value, so the vendor reports this
	 * as a plain NotSupported (Which doesn't tell the user much). */
	if (RageRHI::GetActiveType() != ERageRHIType::DirectX12)
	{
		return ERageFeatureSupport::NotSupportedByRHI;
	}

	return RageUpscalerMapping::FromVendor(UDLSSLibrary::QueryDLSSRRSupport());
#else
	return ERageFeatureSupport::NotSupported;
#endif
}

ERageFeatureSupport URageVideoSettings::QueryDLSSFrameGenSupport() const
{
#if WITH_STREAMLINE_DLSSG
	return RageUpscalerMapping::FromVendorStreamline(UStreamlineLibraryDLSSG::QueryDLSSGSupport());
#else
	return ERageFeatureSupport::NotSupported;
#endif
}

ERageFeatureSupport URageVideoSettings::QueryReflexSupport() const
{
#if WITH_STREAMLINE_REFLEX
	/** Streamline ships no Vulkan backend, but StreamlineLibraryReflex.cpp:132 only ever answers
	 * Supported or NotSupported, unlike DLSS-G it never reports NotSupportedByRHI itself. */
	if (RageRHI::GetActiveType() == ERageRHIType::Vulkan)
	{
		return ERageFeatureSupport::NotSupportedByRHI;
	}

	return RageUpscalerMapping::FromVendorStreamline(UStreamlineLibraryReflex::QueryReflexSupport());
#else
	return ERageFeatureSupport::NotSupported;
#endif
}

TArray<ERageDLSSMode> URageVideoSettings::GetSupportedDLSSModes() const
{
	TArray<ERageDLSSMode> Result;
#if WITH_DLSS
	for (const UDLSSMode VendorMode : UDLSSLibrary::GetSupportedDLSSModes())
	{
		if (TOptional<ERageDLSSMode> Mapped = RageUpscalerMapping::FromVendor(VendorMode))
		{
			Result.Add(*Mapped);
		}
	}
#endif
	return Result;
}

TArray<ERageXeSSMode> URageVideoSettings::GetSupportedXeSSModes() const
{
	TArray<ERageXeSSMode> Result;
#if WITH_XESS
	for (EXeSSQualityMode VendorMode : UXeSSBlueprintLibrary::GetSupportedXeSSQualityModes())
	{
		if (TOptional<ERageXeSSMode> Mapped = RageUpscalerMapping::FromVendor(VendorMode))
		{
			Result.Add(*Mapped);
		}
	}
#endif
	return Result;
}

TArray<ERageFrameGenerationMode> URageVideoSettings::GetSupportedDLSSFrameGenModes() const
{
	/* Off is always a valid choice regardless of hardware. Only x2/x3/x4 are hardware-gated. */
	TArray<ERageFrameGenerationMode> Result = { ERageFrameGenerationMode::Off };
#if WITH_STREAMLINE_DLSSG
	for (EStreamlineDLSSGMode VendorMode : UStreamlineLibraryDLSSG::GetSupportedDLSSGModes())
	{
		if (TOptional<ERageFrameGenerationMode> Mapped = RageUpscalerMapping::FromVendorDLSSG(VendorMode))
		{
			Result.AddUnique(*Mapped);
		}
	}
#endif
	return Result;
}

TArray<ERageFrameGenerationMode> URageVideoSettings::GetSupportedXeSSFrameGenModes() const
{
	/* Off is always a valid choice regardless of hardware. Only x2/x3/x4 are hardware-gated. */
	TArray<ERageFrameGenerationMode> Result = { ERageFrameGenerationMode::Off };
#if WITH_XEFG
	for (EXeFGMode VendorMode : UXeFGBlueprintLibrary::GetSupportedXeFGModes())
	{
		if (TOptional<ERageFrameGenerationMode> Mapped = RageUpscalerMapping::FromVendorXeFG(VendorMode))
		{
			Result.AddUnique(*Mapped);
		}
	}
#endif
	return Result;
}

bool URageVideoSettings::IsAntiAliasingMethodSupported(ERageAntiAliasingMethod Method) const
{
	if (!GIsRHIInitialized)
	{
		return false;
	}

	switch (Method)
	{
		case ERageAntiAliasingMethod::None:
		case ERageAntiAliasingMethod::FXAA:
		case ERageAntiAliasingMethod::SMAA: return true;
		case ERageAntiAliasingMethod::TAA:  return SupportsGen4TAA(GMaxRHIShaderPlatform);
		case ERageAntiAliasingMethod::TSR:  return SupportsTSR(GMaxRHIShaderPlatform);
		case ERageAntiAliasingMethod::MSAA: return IsForwardShadingEnabled(GMaxRHIShaderPlatform);
	}

	return false;
}

TArray<ERageAntiAliasingMethod> URageVideoSettings::GetAvailableAntiAliasingMethods() const
{
	TArray<ERageAntiAliasingMethod> Methods;
	for (const ERageAntiAliasingMethod Candidate : { ERageAntiAliasingMethod::None, ERageAntiAliasingMethod::FXAA,
		ERageAntiAliasingMethod::TAA, ERageAntiAliasingMethod::MSAA, ERageAntiAliasingMethod::TSR, ERageAntiAliasingMethod::SMAA })
	{
		if (IsAntiAliasingMethodSupported(Candidate))
		{
			Methods.Add(Candidate);
		}
	}

	return Methods;
}

bool URageVideoSettings::IsAntiAliasingMethodOverriddenByUpscaler() const
{
	return IsThirdPartyUpscalerActive();
}

bool URageVideoSettings::IsThirdPartyUpscalerActive() const
{
	const ERageUpscalerMethod Method = IsValid(Pending) ? Pending->Upscaler.Method : Upscaler.Method;
	return Method == ERageUpscalerMethod::DLSS
		|| Method == ERageUpscalerMethod::FSR
		|| Method == ERageUpscalerMethod::XeSS;
}

bool URageVideoSettings::IsRHISelectionSupported() const
{
	return RageRHI::IsSelectionSupported();
}

TArray<ERageRHIType> URageVideoSettings::GetSelectableRHITypes() const
{
	return RageRHI::GetSelectableTypes();
}

ERageRHIType URageVideoSettings::GetActiveRHIType() const
{
	return RageRHI::GetActiveType();
}

bool URageVideoSettings::IsRestartRequiredForRHI() const
{
	if (!RageRHI::IsSelectionSupported() || GIsEditor)
	{
		return false;
	}

	const ERageRHIType Active = RageRHI::GetActiveType();
	return Active != ERageRHIType::Auto && RageRHI::ResolveEffectiveType(PreferredRHI) != Active;
}

bool URageVideoSettings::IsRestartRequiredForFrameGeneration() const
{
#if WITH_XEFG
	return UXeFGBlueprintLibrary::IfRelaunchRequiredByXeFG();
#else
	return false;
#endif
}

bool URageVideoSettings::IsRestartRequired() const
{
	return IsRestartRequiredForRHI() || IsRestartRequiredForFrameGeneration();
}

void URageVideoSettings::PushCurrentIntoEngineProperties()
{
	SetScreenResolution(Resolution);
	SetFullscreenMode(WindowMode);
	SetVSyncEnabled(bVSyncEnabled);
	SetFrameRateLimit(TargetFrameRateLimit);

	ScalabilityQuality.ResolutionQuality = ResolutionScaleNormalized * 100.f;
	ScalabilityQuality.ViewDistanceQuality = ViewDistanceQuality;
	ScalabilityQuality.AntiAliasingQuality = AntiAliasingQuality;
	ScalabilityQuality.ShadowQuality = ShadowQuality;
	ScalabilityQuality.GlobalIlluminationQuality = GlobalIlluminationQuality;
	ScalabilityQuality.ReflectionQuality = ReflectionQuality;
	ScalabilityQuality.PostProcessQuality = PostProcessQuality;
	ScalabilityQuality.TextureQuality = TextureQuality;
	ScalabilityQuality.EffectsQuality = VisualEffectQuality;
	ScalabilityQuality.FoliageQuality = FoliageQuality;
	ScalabilityQuality.ShadingQuality = ShadingQuality;
}

void URageVideoSettings::ApplyRayTracingCVars(const FRageRayTracingSettings& Settings)
{
	const bool bMasterEnabled = Settings.bEnabled && IsRayTracingSupported();

	RageVideoCVars::SetBool(RageVideoCVars::RayTracingMaster, bMasterEnabled);
	RageVideoCVars::SetBool(RageVideoCVars::RayTracingShadows, bMasterEnabled && Settings.bShadows);
	RageVideoCVars::SetBool(RageVideoCVars::RayTracingReflections, bMasterEnabled && Settings.bReflections);
	RageVideoCVars::SetBool(RageVideoCVars::RayTracingGI, bMasterEnabled && Settings.bGlobalIllumination);
	RageVideoCVars::SetBool(RageVideoCVars::RayTracingAO, bMasterEnabled && Settings.bAmbientOcclusion);
	RageVideoCVars::SetBool(RageVideoCVars::RayTracingTranslucency, bMasterEnabled && Settings.bTranslucency);
}

void URageVideoSettings::ApplyUpscalerSettings(const FRageUpscalerSettings& Settings)
{
	const bool bWantsDLSS = Settings.Method == ERageUpscalerMethod::DLSS;
	const bool bWantsFSR = Settings.Method == ERageUpscalerMethod::FSR;
	const bool bWantsXeSS = Settings.Method == ERageUpscalerMethod::XeSS;
	
	RageVideoCVars::SetBool(RageVideoCVars::FSREnabled, bWantsFSR);

	if (bWantsFSR)
	{
		RageVideoCVars::SetInt(RageVideoCVars::FSRQualityMode, RageUpscalerMapping::ToVendor(Settings.FSRMode));
		RageVideoCVars::SetFloat(RageVideoCVars::FSRSharpness, Settings.FSRSharpness);
	}

	RageVideoCVars::SetBool(RageVideoCVars::FSRFrameInterpolation, bWantsFSR && Settings.bFSRFrameInterpolation);
	RageVideoCVars::SetInt(RageVideoCVars::ReflexMode, StaticCast<int32>(Settings.ReflexMode));

#if WITH_DLSS
	UDLSSLibrary::EnableDLSS(bWantsDLSS);
	if (bWantsDLSS)
	{
		const UDLSSMode VendorMode = RageUpscalerMapping::ToVendor(Settings.DLSSMode);
		bool bIsModeSupported = false;
		bool bIsFixedScreenPercentage = false;
		float OptimalScreenPercentage = 100.f;
		float MinScreenPercentage = 100.f;
		float MaxScreenPercentage = 100.f;
		float OptimalSharpness_DEPRECATED = 0.f; /* Dlss lib deprecates this. */
		UDLSSLibrary::GetDLSSModeInformation(
			VendorMode,
			FVector2D(StaticCast<float>(Resolution.X), StaticCast<float>(Resolution.Y)),
			bIsModeSupported, OptimalScreenPercentage, bIsFixedScreenPercentage,
			MinScreenPercentage, MaxScreenPercentage, OptimalSharpness_DEPRECATED);

		if (bIsModeSupported)
		{
			RageVideoCVars::SetFloat(RageVideoCVars::ScreenPercentage, OptimalScreenPercentage);
		}
	}

	const bool bRayTracingActive = RayTracing.bEnabled && IsRayTracingSupported();
	if (bWantsDLSS && bRayTracingActive && UDLSSLibrary::IsDLSSRRSupported())
	{
		UDLSSLibrary::EnableDLSSRR(Settings.bDLSSRayReconstruction);
	}
	else
	{
		UDLSSLibrary::EnableDLSSRR(false);
	}
#else
	RageVideoCVars::SetBool(RageVideoCVars::DLSSEnable, false);
#endif

#if WITH_STREAMLINE_DLSSG
	DeferredApplyDLSSFrameGeneration((bWantsDLSS && UStreamlineLibraryDLSSG::IsDLSSGSupported())
		                                 ? Settings.DLSSFrameGenMode
		                                 : ERageFrameGenerationMode::Off);
#endif

#if WITH_XESS
	UXeSSBlueprintLibrary::SetXeSSQualityMode(bWantsXeSS ? RageUpscalerMapping::ToVendor(Settings.XeSSMode) : EXeSSQualityMode::Off);
#else
	RageVideoCVars::SetBool(RageVideoCVars::XeSSEnable, false);
#endif

#if WITH_XEFG
	DeferredApplyXeSSFrameGeneration((bWantsXeSS && UXeFGBlueprintLibrary::IsXeFGSupported())
		                                 ? Settings.XeSSFrameGenMode
		                                 : ERageFrameGenerationMode::Off);
#endif

#if WITH_XELL
	if (UXeLLBlueprintLibrary::IsXeLLSupported() && UXeLLBlueprintLibrary::IsXeLLAvailable())
	{
		UXeLLBlueprintLibrary::SetXeLLMode(Settings.bXeLLEnabled ? EXeLLMode::On : EXeLLMode::Off);
	}
#endif
}

void URageVideoSettings::ApplyPostProcessCVars()
{
	RageVideoCVars::SetInt(RageVideoCVars::MotionBlurQuality, bMotionBlurEnabled ? PostProcessQuality : 0);
	RageVideoCVars::SetInt(RageVideoCVars::DepthOfFieldQuality, bDepthOfFieldEnabled ? PostProcessQuality : 0);
	RageVideoCVars::SetBool(RageVideoCVars::FilmGrain, bFilmGrainEnabled);
	RageVideoCVars::SetBool(RageVideoCVars::SceneColorFringe, bChromaticAberrationEnabled);
}

void URageVideoSettings::ApplyAntiAliasingCVars()
{
	ClampAntiAliasingMethodToSupported();

	RageVideoCVars::SetInt(RageVideoCVars::AntiAliasingMethod, RageAntiAliasingMapping::ToVendor(AntiAliasingMethod));
	RageVideoCVars::SetInt(RageVideoCVars::MSAACount, RageAntiAliasingMapping::ToSampleCount(MSAASampleCount));
}

void URageVideoSettings::ClampAntiAliasingMethodToSupported()
{
	if (IsAntiAliasingMethodSupported(AntiAliasingMethod))
	{
		return;
	}
	
	S_LOG(Warning, "Rage Settings: persisted anti-aliasing method is not supported here, falling back to TSR.");

	AntiAliasingMethod = ERageAntiAliasingMethod::TSR;
	if (IsValid(Pending))
	{
		Pending->AntiAliasingMethod = ERageAntiAliasingMethod::TSR;
	}
}

/** Everything the player saved is only staged by LoadSettings, PushCurrentIntoEngineProperties fills
 * in UGameUserSettings' members and the ScalabilityQuality struct, but nothing reaches a cvar until something calls ApplySettings().
 * Resolution and window mode are left out on purpose since the engine already brings those up from
 * GameUserSettings during startup. */
void URageVideoSettings::DeferredApplyStartupSettings()
{
	FTSTicker::RemoveTicker(StartupApplyTickerHandle);
	StartupApplyTickerHandle = FTSTicker::GetCoreTicker().AddTicker(FTickerDelegate::CreateWeakLambda(this,
		[this](float) -> bool
		{
			ClampUpscalerMethodToSupported(Upscaler);

			Scalability::SetQualityLevels(ScalabilityQuality);

			ApplyRayTracingCVars(RayTracing);
			ApplyUpscalerSettings(Upscaler);
			ApplyPostProcessCVars();
			ApplyAntiAliasingCVars();
			RageVideoCVars::SetFloat(RageVideoCVars::Gamma, Brightness);

			EnableHDRDisplayOutput(bHDREnabled, RageHDRDisplayNits::ToInt32(HDRDisplayNits));
			
			if (IsValid(Pending))
			{
				RageSettings::CopyObjectProperties(Pending, this);
			}

			return false;
		}));
}

void URageVideoSettings::ApplyPreferredRHI()
{
	if (!RageRHI::IsSelectionSupported())
	{
		return;
	}

	if (!RageRHI::IsTypeSelectable(PreferredRHI))
	{
		S_LOG(Warning, "Rage Settings: preferred RHI is not selectable on this build/hardware, handing the choice back to the project default.");
		PreferredRHI = ERageRHIType::Auto;
		Pending->PreferredRHI = ERageRHIType::Auto;
	}

	RageRHI::WritePreference(PreferredRHI);
}

void URageVideoSettings::ReconcilePreferredRHIWithActual()
{
	if (!RageRHI::IsSelectionSupported() || GIsEditor)
	{
		return;
	}

	const ERageRHIType Active = RageRHI::GetActiveType();
	if (Active == ERageRHIType::Auto)
	{
		return;
	}

	if (RageRHI::ResolveEffectiveType(PreferredRHI) == Active)
	{
		return;
	}

	S_LOG(Log, "Rage Settings: preferred RHI did not survive startup, following the API that actually loaded instead.");
	
	PreferredRHI = RageRHI::GetProjectDefaultType() == Active ? ERageRHIType::Auto : Active;
	RageRHI::WritePreference(PreferredRHI);
}

void URageVideoSettings::BroadcastDirtyIfChanged(bool bWasDirtyBefore)
{
	const bool bIsDirtyNow = IsDirty();
	if (bWasDirtyBefore != bIsDirtyNow)
	{
		DirtyStateChangedDelegate.Broadcast(ERageSettingsCategory::Video, bIsDirtyNow);
	}
}

void URageVideoSettings::ClampUpscalerMethodToSupported(FRageUpscalerSettings& Settings) const
{
	bool bMethodStillSupported;
	switch (Settings.Method)
	{
		case ERageUpscalerMethod::DLSS: bMethodStillSupported = IsDLSSSupported();	break;
		case ERageUpscalerMethod::XeSS: bMethodStillSupported = IsXeSSSupported();	break;
		case ERageUpscalerMethod::FSR:  bMethodStillSupported = IsFSRSupported();	break;
		default:                        bMethodStillSupported = true;				break;
	}

	if (!bMethodStillSupported)
	{
		S_LOG(Warning, "Rage Settings: persisted upscaler method is no longer supported on this hardware/platform, turning upscaling off.");
		Settings.Method = ERageUpscalerMethod::Off;
	}
}

/* We defer applying FrameGeneration due to a crash that occurs when applied at the same time as resolution settings. */
void URageVideoSettings::DeferredApplyDLSSFrameGeneration(ERageFrameGenerationMode DesiredMode)
{
#if WITH_STREAMLINE_DLSSG
	FTSTicker::RemoveTicker(DLSSFrameGenTickerHandle);
	DLSSFrameGenTickerHandle = FTSTicker::GetCoreTicker().AddTicker(FTickerDelegate::CreateWeakLambda(this,
		[DesiredMode](float) -> bool
		{
			UStreamlineLibraryDLSSG::SetDLSSGMode(RageUpscalerMapping::ToVendorDLSSG(DesiredMode));
			return false;
		}));
#endif
}

/* We defer applying FrameGeneration due to a crash that occurs when applied at the same time as resolution settings. */
void URageVideoSettings::DeferredApplyXeSSFrameGeneration(ERageFrameGenerationMode DesiredMode)
{
#if WITH_XEFG
	FTSTicker::RemoveTicker(XeSSFrameGenTickerHandle);
	XeSSFrameGenTickerHandle = FTSTicker::GetCoreTicker().AddTicker(FTickerDelegate::CreateWeakLambda(this,
		[DesiredMode](float) -> bool
		{
			UXeFGBlueprintLibrary::SetXeFGMode(RageUpscalerMapping::ToVendorXeFG(DesiredMode));
			return false;
		}));
#endif
}
