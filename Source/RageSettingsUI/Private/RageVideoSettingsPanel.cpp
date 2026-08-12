// Copyright (c) 2026 Abdallah Boutrif
// SPDX-License-Identifier: MIT

#include "RageVideoSettingsPanel.h"
#include "RageComboRow.h"

#include "RageMacros.h"
#include "RageScalabilityCategory.h"
#include "RageSelectionRow.h"
#include "RageSettingsDeveloperSettings.h"
#include "RageSettingsUIStatics.h"
#include "RageVideoSettings.h"
#include "RageSettingsSubsystem.h"
#include "RageSettingsUIDeveloperSettings.h"
#include "RageSliderDisplayFormat.h"
#include "RageSliderRow.h"
#include "RageToggleRow.h"
#include "Components/PanelWidget.h"
#include "Components/WidgetSwitcher.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(RageVideoSettingsPanel)

#define IF_VALID(Obj) if (Obj) {
#define END_IF }

namespace
{
	FText ResolveSupportReason(ERageFeatureSupport Support)
	{
		/* Supported should not have a reason, is should simply be enabled. */
		if (Support == ERageFeatureSupport::Supported)
		{
			return FText::GetEmpty();
		}

		return RageSettingsUI::ResolveLocTextForEnum(Support);
	}

	constexpr float AspectRatioTolerance = 0.03f;

	FIntPoint ResolveAspectRatio(const FIntPoint& Resolution)
	{
		static const FIntPoint CommonAspectRatios[] = {
			{ 5, 4 }, { 4, 3 }, { 3, 2 }, { 16, 10 }, { 5, 3 }, { 16, 9 }, { 21, 9 }, { 32, 9 }
		};

		if (Resolution.X <= 0 || Resolution.Y <= 0)
		{
			return FIntPoint::ZeroValue;
		}

		const float Ratio = static_cast<float>(Resolution.X) / static_cast<float>(Resolution.Y);

		FIntPoint Nearest = FIntPoint::ZeroValue;
		float NearestError = AspectRatioTolerance;
		for (const FIntPoint& Candidate : CommonAspectRatios)
		{
			const float CandidateRatio = static_cast<float>(Candidate.X) / static_cast<float>(Candidate.Y);
			const float Error = FMath::Abs(Ratio - CandidateRatio) / CandidateRatio;
			if (Error < NearestError)
			{
				NearestError = Error;
				Nearest = Candidate;
			}
		}

		if (Nearest != FIntPoint::ZeroValue)
		{
			return Nearest;
		}

		const int32 Divisor = FMath::Max(FMath::GreatestCommonDivisor(Resolution.X, Resolution.Y), 1);
		return FIntPoint(Resolution.X / Divisor, Resolution.Y / Divisor);
	}
}

void URageVideoSettingsPanel::NativeConstruct()
{
	Super::NativeConstruct();
	
	IF_VALID(ResolutionRow)
		ResolutionRow->SetLabel(RAGE_LOC("Resolution"));
		ResolutionRow->ValueChangedDelegate.AddUObject(this, &URageVideoSettingsPanel::HandleResolutionChanged);
	END_IF
	
	IF_VALID(MonitorRow)
		MonitorRow->SetLabel(RAGE_LOC("Monitor"));
		MonitorRow->ValueChangedDelegate.AddUObject(this, &URageVideoSettingsPanel::HandleMonitorChanged);
	END_IF

	IF_VALID(WindowModeRow)
		WindowModeRow->SetLabel(RAGE_LOC("WindowMode"));
		WindowModeRow->ValueChangedDelegate.AddUObject(this, &URageVideoSettingsPanel::HandleWindowModeChanged);
	END_IF
	
	IF_VALID(VSyncRow)
		VSyncRow->SetLabel(RAGE_LOC("VSync"));
		VSyncRow->ValueChangedDelegate.AddUObject(this, &URageVideoSettingsPanel::HandleVSyncChanged);
	END_IF
	
	IF_VALID(FrameRateLimitRow)
		FrameRateLimitRow->SetLabel(RAGE_LOC("FrameRateLimit"));
		FrameRateLimitRow->SetRange(0.f, SETTINGS->FpsMax);

		FrameRateLimitRow->SetDisplayFormat(ERageSliderDisplayFormat::Integer);
		FrameRateLimitRow->ValueChangedDelegate.AddUObject(this, &URageVideoSettingsPanel::HandleFrameRateLimitChanged);
	END_IF
	
	IF_VALID(ResolutionScaleRow)
		ResolutionScaleRow->SetLabel(RAGE_LOC("ResolutionScale"));
		ResolutionScaleRow->SetRange(0.1f, 1.f);
	END_IF
	
	IF_VALID(ResolutionScaleRow)
		ResolutionScaleRow->SetDisplayFormat(ERageSliderDisplayFormat::Percent);
		ResolutionScaleRow->ValueChangedDelegate.AddUObject(this, &URageVideoSettingsPanel::HandleResolutionScaleChanged);
	END_IF
	
	IF_VALID(BrightnessRow)
		BrightnessRow->SetLabel(RAGE_LOC("Brightness"));
		BrightnessRow->SetRange(SETTINGS->BrightnessMin, SETTINGS->BrightnessMax);
	END_IF
	
	IF_VALID(BrightnessRow)
		BrightnessRow->SetDisplayFormat(ERageSliderDisplayFormat::Raw);
		BrightnessRow->ValueChangedDelegate.AddUObject(this, &URageVideoSettingsPanel::HandleBrightnessChanged);
	END_IF
	
	IF_VALID(FieldOfViewRow)
		FieldOfViewRow->SetLabel(RAGE_LOC("FieldOfView"));
		FieldOfViewRow->SetRange(SETTINGS->FieldOfViewMin, SETTINGS->FieldOfViewMax);
		FieldOfViewRow->SetDisplayFormat(ERageSliderDisplayFormat::Integer);
		FieldOfViewRow->ValueChangedDelegate.AddUObject(this, &URageVideoSettingsPanel::HandleFieldOfViewChanged);
	END_IF
	
	IF_VALID(HDRRow)
		HDRRow->SetLabel(RAGE_LOC("HDR"));
		HDRRow->ValueChangedDelegate.AddUObject(this, &URageVideoSettingsPanel::HandleHDRChanged);
	END_IF

	IF_VALID(HDRNitsRow)
		HDRNitsRow->SetLabel(RAGE_LOC("HDRNits"));
		HDRNitsRow->SetOptions(RageSettingsUI::BuildEnumOptionsTexts<ERageHDRDisplayNits>(
		{ ERageHDRDisplayNits::Nits1000, ERageHDRDisplayNits::Nits2000 }));
		HDRNitsRow->ValueChangedDelegate.AddUObject(this, &URageVideoSettingsPanel::HandleHDRNitsChanged);
	END_IF
	
	IF_VALID(GraphicsAPIRow)
		GraphicsAPIRow->SetLabel(RAGE_LOC("GraphicsAPI"));
		GraphicsAPIRow->ValueChangedDelegate.AddUObject(this, &URageVideoSettingsPanel::HandleGraphicsAPIChanged);
	END_IF

	IF_VALID(AntiAliasingMethodRow)
		AntiAliasingMethodRow->SetLabel(RAGE_LOC("AntiAliasingMethod"));
		AntiAliasingMethodRow->ValueChangedDelegate.AddUObject(this, &URageVideoSettingsPanel::HandleAntiAliasingMethodChanged);
	END_IF

	IF_VALID(MSAASampleCountRow)
		MSAASampleCountRow->SetLabel(RAGE_LOC("MSAASampleCount"));
		MSAASampleCountRow->SetOptions(RageSettingsUI::BuildEnumOptionsTexts<ERageMSAASampleCount>(
			{ ERageMSAASampleCount::x2, ERageMSAASampleCount::x4, ERageMSAASampleCount::x8 }));
		MSAASampleCountRow->ValueChangedDelegate.AddUObject(this, &URageVideoSettingsPanel::HandleMSAASampleCountChanged);
	END_IF

	IF_VALID(QualityPresetRow)
		QualityPresetRow->SetLabel(RAGE_LOC("QualityPreset"));
		QualityPresetRow->SetOptions(RageSettingsUI::BuildEnumOptionsTexts<ERageQualityPreset>(
		{ ERageQualityPreset::Low, ERageQualityPreset::Medium, ERageQualityPreset::High, ERageQualityPreset::Epic, ERageQualityPreset::Custom }));
		QualityPresetRow->ValueChangedDelegate.AddUObject(this, &URageVideoSettingsPanel::HandleQualityPresetChanged);
	END_IF
	
	BuildScalabilityRows();

	IF_VALID(RayTracingMasterRow)
		RayTracingMasterRow->SetLabel(RAGE_LOC("RayTracingMaster"));
		RayTracingMasterRow->ValueChangedDelegate.AddUObject(this, &URageVideoSettingsPanel::HandleRayTracingMasterChanged);
	END_IF
	
	IF_VALID(RTShadowsRow)
		RTShadowsRow->SetLabel(RAGE_LOC("RayTracingShadows"));
		RTShadowsRow->ValueChangedDelegate.AddUObject(this, &URageVideoSettingsPanel::HandleRTShadowsChanged);
	END_IF
	
	IF_VALID(RTReflectionsRow)
		RTReflectionsRow->SetLabel(RAGE_LOC("RayTracingReflections"));
		RTReflectionsRow->ValueChangedDelegate.AddUObject(this, &URageVideoSettingsPanel::HandleRTReflectionsChanged);
	END_IF
	
	IF_VALID(RTGlobalIlluminationRow)
		RTGlobalIlluminationRow->SetLabel(RAGE_LOC("RayTracingGlobalIllumination"));
		RTGlobalIlluminationRow->ValueChangedDelegate.AddUObject(this, &URageVideoSettingsPanel::HandleRTGlobalIlluminationChanged);
	END_IF
	
	IF_VALID(RTAmbientOcclusionRow)
		RTAmbientOcclusionRow->SetLabel(RAGE_LOC("RayTracingAmbientOcclusion"));
		RTAmbientOcclusionRow->ValueChangedDelegate.AddUObject(this, &URageVideoSettingsPanel::HandleRTAmbientOcclusionChanged);
	END_IF
	
	IF_VALID(RTTranslucencyRow)
		RTTranslucencyRow->SetLabel(RAGE_LOC("RayTracingTranslucency"));
		RTTranslucencyRow->ValueChangedDelegate.AddUObject(this, &URageVideoSettingsPanel::HandleRTTranslucencyChanged);
	END_IF
	
	IF_VALID(UpscalerMethodRow)
		UpscalerMethodRow->SetLabel(RAGE_LOC("UpscalerMethod"));
		UpscalerMethodRow->ValueChangedDelegate.AddUObject(this, &URageVideoSettingsPanel::HandleUpscalerMethodChanged);
	END_IF
	
	IF_VALID(DLSSModeRow)
		DLSSModeRow->SetLabel(RAGE_LOC("DLSSMode"));
		DLSSModeRow->ValueChangedDelegate.AddUObject(this, &URageVideoSettingsPanel::HandleDLSSModeChanged);
	END_IF
	
	IF_VALID(DLSSFrameGenRow)
		DLSSFrameGenRow->SetLabel(RAGE_LOC("DLSSFrameGeneration"));
		DLSSFrameGenRow->ValueChangedDelegate.AddUObject(this, &URageVideoSettingsPanel::HandleDLSSFrameGenChanged);
	END_IF
	
	IF_VALID(DLSSRayReconstructionRow)
		DLSSRayReconstructionRow->SetLabel(RAGE_LOC("DLSSRayReconstruction"));
		DLSSRayReconstructionRow->ValueChangedDelegate.AddUObject(this, &URageVideoSettingsPanel::HandleDLSSRayReconstructionChanged);
	END_IF
	
	IF_VALID(FSRModeRow)
		FSRModeRow->SetLabel(RAGE_LOC("FSRMode"));
		FSRModeRow->SetOptions(RageSettingsUI::BuildEnumOptionsTexts<ERageFSRMode>(
			{ ERageFSRMode::UltraPerformance, ERageFSRMode::Performance, ERageFSRMode::Balanced, ERageFSRMode::Quality, ERageFSRMode::NativeAA }));
		FSRModeRow->ValueChangedDelegate.AddUObject(this, &URageVideoSettingsPanel::HandleFSRModeChanged);
	END_IF
	
	IF_VALID(FSRSharpnessRow)
		FSRSharpnessRow->SetLabel(RAGE_LOC("FSRSharpness"));
		FSRSharpnessRow->SetRange(0.f, 1.f);
		FSRSharpnessRow->SetDisplayFormat(ERageSliderDisplayFormat::Percent);
		FSRSharpnessRow->ValueChangedDelegate.AddUObject(this, &URageVideoSettingsPanel::HandleFSRSharpnessChanged);
	END_IF
	
	IF_VALID(FSRFrameInterpolationRow)
		FSRFrameInterpolationRow->SetLabel(RAGE_LOC("FSRFrameInterpolation"));
		FSRFrameInterpolationRow->ValueChangedDelegate.AddUObject(this, &URageVideoSettingsPanel::HandleFSRFrameInterpolationChanged);
	END_IF
	
	IF_VALID(XeSSModeRow)
		XeSSModeRow->SetLabel(RAGE_LOC("XESSMode"));
		XeSSModeRow->ValueChangedDelegate.AddUObject(this, &URageVideoSettingsPanel::HandleXeSSModeChanged);
	END_IF
	
	IF_VALID(XeSSFrameGenRow)
		XeSSFrameGenRow->SetLabel(RAGE_LOC("XESSFrameGeneration"));
		XeSSFrameGenRow->ValueChangedDelegate.AddUObject(this, &URageVideoSettingsPanel::HandleXeSSFrameGenChanged);
	END_IF
	
	IF_VALID(ReflexModeRow)
		ReflexModeRow->SetLabel(RAGE_LOC("ReflexMode"));
		ReflexModeRow->SetOptions(RageSettingsUI::BuildEnumOptionsTexts<ERageReflexMode>(
			{ ERageReflexMode::Disabled, ERageReflexMode::Enabled, ERageReflexMode::EnabledPlusBoost }));
		ReflexModeRow->ValueChangedDelegate.AddUObject(this, &URageVideoSettingsPanel::HandleReflexModeChanged);
	END_IF
	
	IF_VALID(XeLLRow)
		XeLLRow->SetLabel(RAGE_LOC("XELL"));
		XeLLRow->ValueChangedDelegate.AddUObject(this, &URageVideoSettingsPanel::HandleXeLLChanged);
	END_IF
	
	IF_VALID(MotionBlurRow)
		MotionBlurRow->SetLabel(RAGE_LOC("MotionBlur"));
		MotionBlurRow->ValueChangedDelegate.AddUObject(this, &URageVideoSettingsPanel::HandleMotionBlurChanged);
	END_IF
	
	IF_VALID(DepthOfFieldRow)
		DepthOfFieldRow->SetLabel(RAGE_LOC("DepthOfField"));
		DepthOfFieldRow->ValueChangedDelegate.AddUObject(this, &URageVideoSettingsPanel::HandleDepthOfFieldChanged);
	END_IF
	
	IF_VALID(FilmGrainRow)
		FilmGrainRow->SetLabel(RAGE_LOC("FilmGrain"));
		FilmGrainRow->ValueChangedDelegate.AddUObject(this, &URageVideoSettingsPanel::HandleFilmGrainChanged);
	END_IF
	
	IF_VALID(ChromaticAberrationRow)
		ChromaticAberrationRow->SetLabel(RAGE_LOC("ChromaticAberration"));
		ChromaticAberrationRow->ValueChangedDelegate.AddUObject(this, &URageVideoSettingsPanel::HandleChromaticAberrationChanged);
	END_IF
}

void URageVideoSettingsPanel::InitializePanel(URageSettingsSubsystem* InSubsystem)
{
	VideoSettings = InSubsystem->GetVideoSettings();
	RefreshResolutionOptions();
	RefreshMonitorOptions();
	RefreshWindowModeOptions();
	RefreshGraphicsAPIOptions();
	RefreshAntiAliasingOptions();
	RefreshUpscalerModeOptions();
}

void URageVideoSettingsPanel::RefreshFromSettings()
{
	if (!IsValid(VideoSettings))
	{
		return;
	}
	
	const URageVideoSettings* Pending = VideoSettings->GetPendingSettings();

	IF_VALID(ResolutionRow)
		/* Falls back to the largest mode rather than to index 0. A staged resolution the current monitor
		 * cannot show used to land on the smallest entry in the list, which both misreported the pending
		 * value and read as the settings menu volunteering 1024x768. Staging keeps the two in step now,
		 * so this only covers a config carrying a resolution no attached display has. */
		int32 Index = CachedResolutions.IndexOfByKey(Pending->Resolution);
		if (Index == INDEX_NONE)
		{
			Index = FMath::Max(CachedResolutions.IndexOfByKey(VideoSettings->GetLargestSupportedResolution()), 0);
		}
		ResolutionRow->SetSelectedIndex(Index);
	END_IF

	IF_VALID(MonitorRow)
		MonitorRow->SetSelectedIndex(FMath::Max(CachedMonitorIds.IndexOfByKey(Pending->PreferredMonitorId), 0));
	END_IF

	RefreshResolutionRowEnabled();
	
	IF_VALID(WindowModeRow)
		WindowModeRow->SetSelectedIndex(FMath::Max(SupportedWindowModes.IndexOfByKey(Pending->WindowMode.GetValue()), 0));
	END_IF

	IF_VALID(VSyncRow)
		VSyncRow->SetValue(Pending->bVSyncEnabled);
	END_IF
	
	IF_VALID(FrameRateLimitRow)
		FrameRateLimitRow->SetValue(Pending->TargetFrameRateLimit);
	END_IF

	IF_VALID(ResolutionScaleRow)
		ResolutionScaleRow->SetValue(Pending->ResolutionScaleNormalized);
	END_IF

	IF_VALID(BrightnessRow)
		BrightnessRow->SetValue(Pending->Brightness);
	END_IF

	IF_VALID(FieldOfViewRow)
		FieldOfViewRow->SetValue(Pending->FieldOfView);
	END_IF
	
	const bool bHDRSupported = VideoSettings->IsHDRSupported();

	IF_VALID(HDRRow)
		HDRRow->SetValue(Pending->bHDREnabled);
		HDRRow->SetRowEnabled(bHDRSupported, RAGE_LOC("RequiresWindowsHDR"));
	END_IF

	IF_VALID(HDRNitsRow)
		HDRNitsRow->SetSelectedIndex(static_cast<int32>(Pending->HDRDisplayNits));
		HDRNitsRow->SetRowEnabled(bHDRSupported && Pending->bHDREnabled, FText::GetEmpty());
	END_IF

	IF_VALID(GraphicsAPIRow)
		GraphicsAPIRow->SetSelectedIndex(FMath::Max(SelectableRHITypes.IndexOfByKey(Pending->PreferredRHI), 0));
	END_IF

	IF_VALID(AntiAliasingMethodRow)
		AntiAliasingMethodRow->SetSelectedIndex(FMath::Max(SupportedAntiAliasingMethods.IndexOfByKey(Pending->AntiAliasingMethod), 0));
	END_IF

	IF_VALID(MSAASampleCountRow)
		MSAASampleCountRow->SetSelectedIndex(static_cast<int32>(Pending->MSAASampleCount));
	END_IF

	RefreshAntiAliasingRowsEnabled();

	RefreshQualityPresetRow();
	RefreshScalabilityRows();

	const bool bRayTracingSupported = VideoSettings->IsRayTracingSupported();
	const FRageRayTracingSettings& RT = Pending->RayTracing;

	IF_VALID(RayTracingMasterRow)
		RayTracingMasterRow->SetValue(RT.bEnabled);
		RayTracingMasterRow->SetRowEnabled(bRayTracingSupported,
			ResolveSupportReason(VideoSettings->GetActiveRHIType() == ERageRHIType::DirectX11
				? ERageFeatureSupport::NotSupportedByRHI
				: ERageFeatureSupport::IncompatibleHardware));
	END_IF
	
	IF_VALID(RTShadowsRow)
		RTShadowsRow->SetValue(RT.bShadows);
	END_IF
	
	IF_VALID(RTReflectionsRow)
		RTReflectionsRow->SetValue(RT.bReflections);
	END_IF
	
	IF_VALID(RTGlobalIlluminationRow)
		RTGlobalIlluminationRow->SetValue(RT.bGlobalIllumination);
	END_IF
	
	IF_VALID(RTAmbientOcclusionRow)
		RTAmbientOcclusionRow->SetValue(RT.bAmbientOcclusion);
	END_IF
	
	IF_VALID(RTTranslucencyRow)
		RTTranslucencyRow->SetValue(RT.bTranslucency);
	END_IF
	
	RefreshRayTracingSubRowsEnabled(RT.bEnabled && bRayTracingSupported);
	
	const FRageUpscalerSettings& Up = Pending->Upscaler;
	
	IF_VALID(UpscalerMethodRow)
		UpscalerMethodRow->SetSelectedIndex(FMath::Max(SupportedUpscalerMethods.IndexOfByKey(Up.Method), 0));
	END_IF

	IF_VALID(DLSSModeRow)
		DLSSModeRow->SetSelectedIndex(FMath::Max(SupportedDLSSModes.IndexOfByKey(Up.DLSSMode), 0));
	END_IF
	
	IF_VALID(DLSSFrameGenRow)
		DLSSFrameGenRow->SetSelectedIndex(FMath::Max(SupportedDLSSFrameGenModes.IndexOfByKey(Up.DLSSFrameGenMode), 0));
	END_IF

	IF_VALID(DLSSRayReconstructionRow)
		DLSSRayReconstructionRow->SetValue(Up.bDLSSRayReconstruction);
	END_IF
	RefreshDLSSRayReconstructionRowEnabled();

	IF_VALID(FSRModeRow)
		FSRModeRow->SetSelectedIndex(static_cast<int32>(Up.FSRMode));
	END_IF
	
	IF_VALID(FSRSharpnessRow)
		FSRSharpnessRow->SetValue(Up.FSRSharpness);
	END_IF
	
	IF_VALID(FSRFrameInterpolationRow)
		FSRFrameInterpolationRow->SetValue(Up.bFSRFrameInterpolation);
	END_IF

	IF_VALID(XeSSModeRow)
		XeSSModeRow->SetSelectedIndex(FMath::Max(SupportedXeSSModes.IndexOfByKey(Up.XeSSMode), 0));
	END_IF
	
	IF_VALID(XeSSFrameGenRow)
		XeSSFrameGenRow->SetSelectedIndex(FMath::Max(SupportedXeSSFrameGenModes.IndexOfByKey(Up.XeSSFrameGenMode), 0));
	END_IF
	
	IF_VALID(ReflexModeRow)
		ReflexModeRow->SetSelectedIndex(static_cast<int32>(Up.ReflexMode));
		ReflexModeRow->SetRowEnabled(VideoSettings->IsReflexSupported(), ResolveSupportReason(VideoSettings->QueryReflexSupport()));
	END_IF

	IF_VALID(XeLLRow)
		XeLLRow->SetValue(Up.bXeLLEnabled);
		XeLLRow->SetRowEnabled(VideoSettings->IsXeLLSupported(), ResolveSupportReason(ERageFeatureSupport::NotSupported));
	END_IF
	
	RefreshUpscalerOptionsVisibility(Up.Method);
	
	IF_VALID(MotionBlurRow)
		MotionBlurRow->SetValue(Pending->bMotionBlurEnabled);
	END_IF
	
	IF_VALID(DepthOfFieldRow)
		DepthOfFieldRow->SetValue(Pending->bDepthOfFieldEnabled);
	END_IF
	
	IF_VALID(FilmGrainRow)
		FilmGrainRow->SetValue(Pending->bFilmGrainEnabled);
	END_IF
	
	IF_VALID(ChromaticAberrationRow)
		ChromaticAberrationRow->SetValue(Pending->bChromaticAberrationEnabled);
	END_IF
}

void URageVideoSettingsPanel::RefreshResolutionOptions()
{
	if (!IsValid(ResolutionRow))
	{
		return;
	}
	
	CachedResolutions = VideoSettings->GetSupportedResolutions();
	TArray<FString> Options;
	Options.Reserve(CachedResolutions.Num());
	for (const FIntPoint& Res : CachedResolutions)
	{
		const FIntPoint Aspect = ResolveAspectRatio(Res);
		Options.Add(Aspect != FIntPoint::ZeroValue
			? FString::Printf(TEXT("%dx%d (%d:%d)"), Res.X, Res.Y, Aspect.X, Aspect.Y)
			: FString::Printf(TEXT("%dx%d"), Res.X, Res.Y));
	}
	ResolutionRow->SetOptions(Options);
}

void URageVideoSettingsPanel::RefreshGraphicsAPIOptions()
{
	IF_VALID(GraphicsAPIRow)
		SelectableRHITypes = VideoSettings->GetSelectableRHITypes();
		GraphicsAPIRow->SetOptions(RageSettingsUI::BuildEnumOptionsTexts(SelectableRHITypes));
		GraphicsAPIRow->SetVisibility(SelectableRHITypes.Num() > 1 ? ESlateVisibility::SelfHitTestInvisible : ESlateVisibility::Collapsed);
	END_IF
}

void URageVideoSettingsPanel::RefreshMonitorOptions()
{
	IF_VALID(MonitorRow)
		CachedMonitorIds.Reset();

		TArray<FString> Options;
		int32 DisplayIndex = 0;
		for (const FRageMonitorInfo& Monitor : VideoSettings->GetAvailableMonitors())
		{
			++DisplayIndex;
			CachedMonitorIds.Add(Monitor.Id);
			
			const FString Label = Monitor.Name.IsEmpty()
				? FString::Printf(TEXT("[%d]"), DisplayIndex - 1)
				: FString::Printf(TEXT("%s [%d]"), *Monitor.Name, DisplayIndex - 1);

			Options.Add(Label);
		}

		MonitorRow->SetOptions(Options);

		MonitorRow->SetVisibility(VideoSettings->IsMonitorSelectionSupported()
			? ESlateVisibility::Visible
			: ESlateVisibility::Collapsed);
	END_IF
}

void URageVideoSettingsPanel::RefreshWindowModeOptions()
{
	IF_VALID(WindowModeRow)
		SupportedWindowModes.Reset();
		for (const TEnumAsByte<EWindowMode::Type>& Mode : VideoSettings->GetAvailableWindowModes())
		{
			SupportedWindowModes.Add(Mode.GetValue());
		}

		WindowModeRow->SetOptions(RageSettingsUI::BuildEnumOptionsTexts(SupportedWindowModes));
	END_IF
}

void URageVideoSettingsPanel::RefreshResolutionRowEnabled()
{
	IF_VALID(ResolutionRow)
		/* Borderless sizes itself to the monitor hence. */
		ResolutionRow->SetRowEnabled(VideoSettings->GetPendingSettings()->WindowMode != EWindowMode::WindowedFullscreen,RAGE_LOC("UsingWindowsRes"));
	END_IF
}

void URageVideoSettingsPanel::RefreshAntiAliasingOptions()
{
	IF_VALID(AntiAliasingMethodRow)
		SupportedAntiAliasingMethods = VideoSettings->GetAvailableAntiAliasingMethods();
		AntiAliasingMethodRow->SetOptions(RageSettingsUI::BuildEnumOptionsTexts(SupportedAntiAliasingMethods));
	END_IF

	IF_VALID(MSAASampleCountRow)
		MSAASampleCountRow->SetVisibility(SupportedAntiAliasingMethods.Contains(ERageAntiAliasingMethod::MSAA)
			? ESlateVisibility::SelfHitTestInvisible
			: ESlateVisibility::Collapsed);
	END_IF
}

void URageVideoSettingsPanel::RefreshAntiAliasingRowsEnabled()
{
	const URageVideoSettings* Pending = VideoSettings->GetPendingSettings();

	const bool bUpscalerActive = VideoSettings->IsThirdPartyUpscalerActive();

	IF_VALID(AntiAliasingMethodRow)
		AntiAliasingMethodRow->SetRowEnabled(!bUpscalerActive, RAGE_LOC("ControlledByUpscaler"));
	END_IF

	IF_VALID(ResolutionScaleRow)
		/* DLSS/FSR/XeSS overrides the render resolution. */
		ResolutionScaleRow->SetRowEnabled(!bUpscalerActive, RAGE_LOC("ControlledByUpscaler"));
	END_IF

	IF_VALID(MSAASampleCountRow)
		/* No reason text - the method row that gates this one sits directly above it. */
		MSAASampleCountRow->SetRowEnabled(Pending->AntiAliasingMethod == ERageAntiAliasingMethod::MSAA, FText::GetEmpty());
	END_IF
}

void URageVideoSettingsPanel::RefreshUpscalerModeOptions()
{
	VideoSettings->ClampUpscalerToSupported();

	IF_VALID(UpscalerMethodRow)
		SupportedUpscalerMethods = VideoSettings->GetAvailableUpscalerMethods();
		UpscalerMethodRow->SetOptions(RageSettingsUI::BuildEnumOptionsTexts(SupportedUpscalerMethods));
	END_IF

	IF_VALID(DLSSModeRow)
		SupportedDLSSModes = VideoSettings->GetSupportedDLSSModes();
		DLSSModeRow->SetOptions(RageSettingsUI::BuildEnumOptionsTexts(SupportedDLSSModes));
	END_IF

	IF_VALID(DLSSFrameGenRow)
		SupportedDLSSFrameGenModes = VideoSettings->GetSupportedDLSSFrameGenModes();
		DLSSFrameGenRow->SetOptions(RageSettingsUI::BuildEnumOptionsTexts(SupportedDLSSFrameGenModes));
		DLSSFrameGenRow->SetRowEnabled(VideoSettings->IsDLSSFrameGenSupported(),
			ResolveSupportReason(VideoSettings->QueryDLSSFrameGenSupport()));
	END_IF

	IF_VALID(XeSSModeRow)
		SupportedXeSSModes = VideoSettings->GetSupportedXeSSModes();
		XeSSModeRow->SetOptions(RageSettingsUI::BuildEnumOptionsTexts(SupportedXeSSModes));
	END_IF

	IF_VALID(XeSSFrameGenRow)
		SupportedXeSSFrameGenModes = VideoSettings->GetSupportedXeSSFrameGenModes();
		XeSSFrameGenRow->SetOptions(RageSettingsUI::BuildEnumOptionsTexts(SupportedXeSSFrameGenModes));
		XeSSFrameGenRow->SetRowEnabled(VideoSettings->IsXeSSFrameGenSupported(), ResolveSupportReason(ERageFeatureSupport::NotSupported));
	END_IF
}

void URageVideoSettingsPanel::BuildScalabilityRows()
{
	if (!IsValid(ScalabilityContainer))
	{
		return;
	}
	
	ScalabilityContainer->ClearChildren();
	ScalabilityRows.Reset();
	ScalabilityCategoryByRowId.Reset();

	const TArray<TPair<ERageScalabilityCategory, FText>> Definitions = {
		{ ERageScalabilityCategory::ViewDistance, RAGE_LOC("ViewDistance") },
		{ ERageScalabilityCategory::AntiAliasing, RAGE_LOC("AntiAliasing") },
		{ ERageScalabilityCategory::Shadow, RAGE_LOC("Shadow") },
		{ ERageScalabilityCategory::GlobalIllumination, RAGE_LOC("GlobalIllumination") },
		{ ERageScalabilityCategory::Reflection, RAGE_LOC("Reflection") },
		{ ERageScalabilityCategory::PostProcess, RAGE_LOC("PostProcess") },
		{ ERageScalabilityCategory::Texture, RAGE_LOC("Texture") },
		{ ERageScalabilityCategory::Effects, RAGE_LOC("Effects") },
		{ ERageScalabilityCategory::Foliage, RAGE_LOC("Foliage") },
		{ ERageScalabilityCategory::Shading, RAGE_LOC("Shading") },
	};

	const UEnum* CategoryEnum = StaticEnum<ERageScalabilityCategory>();

	for (const TPair<ERageScalabilityCategory, FText>& Def : Definitions)
	{
		const bool bShouldCreate = SETTINGS_UI->ScalabilityOverrides.FindRef(Def.Key);
		if (!bShouldCreate)
		{
			continue;
		}
		
		URageSelectionRow* Row = CreateWidget<URageSelectionRow>(this, ScalabilityRowClass);
		if (!IsValid(Row))
		{
			continue;
		}

		Row->SetLabel(Def.Value);
		Row->SetOptions(RageSettingsUI::BuildEnumOptionsTexts<ERageQualityPreset>(
			{ ERageQualityPreset::Low, ERageQualityPreset::Medium, ERageQualityPreset::High, ERageQualityPreset::Epic }));
		Row->ValueChangedDelegate.AddUObject(this, &URageVideoSettingsPanel::HandleScalabilityChanged);

		const FName RowId = IsValid(CategoryEnum) ? FName(*CategoryEnum->GetNameStringByValue(static_cast<int64>(Def.Key))) : NAME_None;
		Row->SetRowId(RowId);

		ScalabilityContainer->AddChild(Row);
		ScalabilityRows.Add(Row);
		ScalabilityCategoryByRowId.Add(RowId, Def.Key);
	}
}

void URageVideoSettingsPanel::RefreshScalabilityRows()
{
	for (URageSelectionRow* Row : ScalabilityRows)
	{
		if (!IsValid(Row))
		{
			continue;
		}

		if (const ERageScalabilityCategory* Category = ScalabilityCategoryByRowId.Find(Row->GetRowId()))
		{
			// The rows are built with one option per quality level, so the level is the option index.
			Row->SetSelectedIndex(VideoSettings->GetPendingScalabilityLevel(*Category));
		}
	}
}

void URageVideoSettingsPanel::RefreshQualityPresetRow()
{
	IF_VALID(QualityPresetRow)
		QualityPresetRow->SetSelectedIndex(static_cast<int32>(VideoSettings->GetPendingSettings()->QualityPreset));
	END_IF
}

void URageVideoSettingsPanel::RefreshRayTracingSubRowsEnabled(bool bMasterEnabled)
{
	for (URageToggleRow* Row : { RTShadowsRow.Get(), RTReflectionsRow.Get(), RTGlobalIlluminationRow.Get(), RTAmbientOcclusionRow.Get(), RTTranslucencyRow.Get() })
	{
		if (!IsValid(Row))
		{
			continue;
		}

		Row->SetIsEnabled(bMasterEnabled);
	}
}

void URageVideoSettingsPanel::RefreshUpscalerOptionsVisibility(ERageUpscalerMethod Method)
{
	if (!IsValid(UpscalerOptionsSwitcher))
	{
		return;
	}

	switch (Method)
	{
		case ERageUpscalerMethod::DLSS: UpscalerOptionsSwitcher->SetActiveWidgetIndex(1); break;
		case ERageUpscalerMethod::FSR:  UpscalerOptionsSwitcher->SetActiveWidgetIndex(2); break;
		case ERageUpscalerMethod::XeSS: UpscalerOptionsSwitcher->SetActiveWidgetIndex(3); break;
		default:                        UpscalerOptionsSwitcher->SetActiveWidgetIndex(0); break;
	}
}

void URageVideoSettingsPanel::RefreshDLSSRayReconstructionRowEnabled()
{
	if (!IsValid(DLSSRayReconstructionRow))
	{
		return;
	}

	const URageVideoSettings* Pending = VideoSettings->GetPendingSettings();
	const bool bRayTracingActive = Pending->RayTracing.bEnabled && VideoSettings->IsRayTracingSupported();
	const bool bCanUseRR = Pending->Upscaler.Method == ERageUpscalerMethod::DLSS
		&& bRayTracingActive
		&& VideoSettings->IsDLSSRRSupported();
	
	FText Reason = FText::GetEmpty();
	if (!bCanUseRR)
	{
		const ERageFeatureSupport RRSupport = VideoSettings->QueryDLSSRRSupport();
		if (RRSupport == ERageFeatureSupport::NotSupportedByRHI)
		{
			Reason = ResolveSupportReason(RRSupport);
		}
		else if (!bRayTracingActive)
		{
			Reason = RAGE_LOC("RequiresRayTracing");
		}
		else if (Pending->Upscaler.Method != ERageUpscalerMethod::DLSS)
		{
			Reason = RAGE_LOC("RequiresDLSS");
		}
		else
		{
			Reason = ResolveSupportReason(RRSupport);
		}
	}

	DLSSRayReconstructionRow->SetRowEnabled(bCanUseRR, Reason);
}

void URageVideoSettingsPanel::ModifyPendingRayTracing(TFunctionRef<void(FRageRayTracingSettings&)> Mutator)
{
	FRageRayTracingSettings Settings = VideoSettings->GetPendingRayTracingSettings();
	Mutator(Settings);
	VideoSettings->SetPendingRayTracingSettings(Settings);
}

void URageVideoSettingsPanel::ModifyPendingUpscaler(TFunctionRef<void(FRageUpscalerSettings&)> Mutator)
{
	FRageUpscalerSettings Settings = VideoSettings->GetPendingUpscalerSettings();
	Mutator(Settings);
	VideoSettings->SetPendingUpscalerSettings(Settings);
}

void URageVideoSettingsPanel::HandleResolutionChanged(FName RowId, FRageVariant NewIndex)
{
	if (CachedResolutions.IsValidIndex(NewIndex.Get<int32>()))
	{
		VideoSettings->SetPendingResolution(CachedResolutions[NewIndex.Get<int32>()]);
	}
}

void URageVideoSettingsPanel::HandleMonitorChanged(FName RowId, FRageVariant NewIndex)
{
	if (CachedMonitorIds.IsValidIndex(NewIndex.Get<int32>()))
	{
		VideoSettings->SetPendingMonitorId(CachedMonitorIds[NewIndex.Get<int32>()]);
		
		RefreshWindowModeOptions();
		RefreshResolutionOptions();
		RefreshFromSettings();
	}
}

void URageVideoSettingsPanel::HandleWindowModeChanged(FName RowId, FRageVariant NewIndex)
{
	if (SupportedWindowModes.IsValidIndex(NewIndex.Get<int32>()))
	{
		VideoSettings->SetPendingWindowMode(SupportedWindowModes[NewIndex.Get<int32>()]);
		RefreshResolutionRowEnabled(); // borderless takes its size from the desktop, not from this setting
	}
}

void URageVideoSettingsPanel::HandleVSyncChanged(FName RowId, FRageVariant bNewValue)
{

	VideoSettings->SetPendingVSyncEnabled(bNewValue.Get<bool>());
}

void URageVideoSettingsPanel::HandleFrameRateLimitChanged(FName RowId, FRageVariant NewValue)
{
	VideoSettings->SetPendingFrameRateLimit(NewValue.Get<float>());
}

void URageVideoSettingsPanel::HandleResolutionScaleChanged(FName RowId, FRageVariant NewValue)
{
	VideoSettings->SetPendingResolutionScale(NewValue.Get<float>());
}

void URageVideoSettingsPanel::HandleBrightnessChanged(FName RowId, FRageVariant NewValue)
{
	VideoSettings->SetPendingBrightness(NewValue.Get<float>());
}

void URageVideoSettingsPanel::HandleFieldOfViewChanged(FName RowId, FRageVariant NewValue)
{
	VideoSettings->SetPendingFieldOfView(NewValue.Get<float>());
}

void URageVideoSettingsPanel::HandleHDRChanged(FName RowId, FRageVariant bNewValue)
{
	VideoSettings->SetPendingHDREnabled(bNewValue.Get<bool>());
	
	IF_VALID(HDRNitsRow)
		HDRNitsRow->SetRowEnabled(bNewValue.Get<bool>() && VideoSettings->IsHDRSupported(), FText::GetEmpty());
	END_IF
}

void URageVideoSettingsPanel::HandleHDRNitsChanged(FName RowId, FRageVariant NewIndex)
{
	VideoSettings->SetPendingHDRDisplayNits(static_cast<ERageHDRDisplayNits>(NewIndex.Get<int32>()));
}

void URageVideoSettingsPanel::HandleGraphicsAPIChanged(FName RowId, FRageVariant NewIndex)
{
	if (SelectableRHITypes.IsValidIndex(NewIndex.Get<int32>()))
	{
		VideoSettings->SetPendingPreferredRHI(SelectableRHITypes[NewIndex.Get<int32>()]);
	}
}

void URageVideoSettingsPanel::HandleAntiAliasingMethodChanged(FName RowId, FRageVariant NewIndex)
{
	if (SupportedAntiAliasingMethods.IsValidIndex(NewIndex.Get<int32>()))
	{
		VideoSettings->SetPendingAntiAliasingMethod(SupportedAntiAliasingMethods[NewIndex.Get<int32>()]);
		RefreshAntiAliasingRowsEnabled(); // the sample count row only matters while MSAA is picked
	}
}

void URageVideoSettingsPanel::HandleMSAASampleCountChanged(FName RowId, FRageVariant NewIndex)
{
	VideoSettings->SetPendingMSAASampleCount(static_cast<ERageMSAASampleCount>(NewIndex.Get<int32>()));
}

void URageVideoSettingsPanel::HandleQualityPresetChanged(FName RowId, FRageVariant NewIndex)
{
	VideoSettings->SetPendingQualityPreset(static_cast<ERageQualityPreset>(NewIndex.Get<int32>()));
	RefreshScalabilityRows(); // a preset stamps all sliders values
}

void URageVideoSettingsPanel::HandleScalabilityChanged(FName RowId, FRageVariant NewIndex)
{
	if (const ERageScalabilityCategory* Category = ScalabilityCategoryByRowId.Find(RowId))
	{
		VideoSettings->SetPendingScalabilityLevel(*Category, NewIndex.Get<int32>());
		RefreshQualityPresetRow(); // moving one row may have flipped the preset to Custom
	}
}

void URageVideoSettingsPanel::HandleRayTracingMasterChanged(FName RowId, FRageVariant bNewValue)
{
	ModifyPendingRayTracing([bNewValue](FRageRayTracingSettings& S) { S.bEnabled = bNewValue.Get<bool>(); });
	RefreshRayTracingSubRowsEnabled(bNewValue.Get<bool>());
	RefreshDLSSRayReconstructionRowEnabled(); // RR depends on RT being on - unlock/relock immediately
}

void URageVideoSettingsPanel::HandleRTShadowsChanged(FName RowId, FRageVariant bNewValue)
{
	ModifyPendingRayTracing([bNewValue](FRageRayTracingSettings& S) { S.bShadows = bNewValue.Get<bool>(); });
}

void URageVideoSettingsPanel::HandleRTReflectionsChanged(FName RowId, FRageVariant bNewValue)
{
	ModifyPendingRayTracing([bNewValue](FRageRayTracingSettings& S) { S.bReflections = bNewValue.Get<bool>(); });
}

void URageVideoSettingsPanel::HandleRTGlobalIlluminationChanged(FName RowId, FRageVariant bNewValue)
{
	ModifyPendingRayTracing([bNewValue](FRageRayTracingSettings& S) { S.bGlobalIllumination = bNewValue.Get<bool>(); });
}

void URageVideoSettingsPanel::HandleRTAmbientOcclusionChanged(FName RowId, FRageVariant bNewValue)
{
	ModifyPendingRayTracing([bNewValue](FRageRayTracingSettings& S) { S.bAmbientOcclusion = bNewValue.Get<bool>(); });
}

void URageVideoSettingsPanel::HandleRTTranslucencyChanged(FName RowId, FRageVariant bNewValue)
{
	ModifyPendingRayTracing([bNewValue](FRageRayTracingSettings& S) { S.bTranslucency = bNewValue.Get<bool>(); });
}

void URageVideoSettingsPanel::HandleUpscalerMethodChanged(FName RowId, FRageVariant NewIndex)
{
	if (SupportedUpscalerMethods.IsValidIndex(NewIndex.Get<int32>()))
	{
		const ERageUpscalerMethod NewMethod = SupportedUpscalerMethods[NewIndex.Get<int32>()];
		ModifyPendingUpscaler([NewMethod](FRageUpscalerSettings& S) { S.Method = NewMethod; });
		RefreshUpscalerOptionsVisibility(NewMethod);
		RefreshDLSSRayReconstructionRowEnabled(); // RR only matters while DLSS is the active method
		RefreshAntiAliasingRowsEnabled(); // DLSS/FSR/XeSS take the AA method choice away
	}
}

void URageVideoSettingsPanel::HandleDLSSModeChanged(FName RowId, FRageVariant NewIndex)
{
	if (SupportedDLSSModes.IsValidIndex(NewIndex.Get<int32>()))
	{
		const ERageDLSSMode Mode = SupportedDLSSModes[NewIndex.Get<int32>()];
		ModifyPendingUpscaler([Mode](FRageUpscalerSettings& S) { S.DLSSMode = Mode; });
	}
}

void URageVideoSettingsPanel::HandleDLSSFrameGenChanged(FName RowId, FRageVariant NewIndex)
{
	if (SupportedDLSSFrameGenModes.IsValidIndex(NewIndex.Get<int32>()))
	{
		const ERageFrameGenerationMode Mode = SupportedDLSSFrameGenModes[NewIndex.Get<int32>()];
		ModifyPendingUpscaler([Mode](FRageUpscalerSettings& S) { S.DLSSFrameGenMode = Mode; });
	}
}

void URageVideoSettingsPanel::HandleDLSSRayReconstructionChanged(FName RowId, FRageVariant bNewValue)
{
	ModifyPendingUpscaler([bNewValue](FRageUpscalerSettings& S) { S.bDLSSRayReconstruction = bNewValue.Get<bool>(); });
}

void URageVideoSettingsPanel::HandleFSRModeChanged(FName RowId, FRageVariant NewIndex)
{
	const ERageFSRMode Mode = static_cast<ERageFSRMode>(NewIndex.Get<int32>());
	ModifyPendingUpscaler([Mode](FRageUpscalerSettings& S) { S.FSRMode = Mode; });
}

void URageVideoSettingsPanel::HandleFSRSharpnessChanged(FName RowId, FRageVariant NewValue)
{
	ModifyPendingUpscaler([NewValue](FRageUpscalerSettings& S) { S.FSRSharpness = NewValue.Get<float>(); });
}

void URageVideoSettingsPanel::HandleFSRFrameInterpolationChanged(FName RowId, FRageVariant bNewValue)
{
	ModifyPendingUpscaler([bNewValue](FRageUpscalerSettings& S) { S.bFSRFrameInterpolation = bNewValue.Get<bool>(); });
}

void URageVideoSettingsPanel::HandleXeSSModeChanged(FName RowId, FRageVariant NewIndex)
{
	if (SupportedXeSSModes.IsValidIndex(NewIndex.Get<int32>()))
	{
		const ERageXeSSMode Mode = SupportedXeSSModes[NewIndex.Get<int32>()];
		ModifyPendingUpscaler([Mode](FRageUpscalerSettings& S) { S.XeSSMode = Mode; });
	}
}

void URageVideoSettingsPanel::HandleXeSSFrameGenChanged(FName RowId, FRageVariant NewIndex)
{
	if (SupportedXeSSFrameGenModes.IsValidIndex(NewIndex.Get<int32>()))
	{
		const ERageFrameGenerationMode Mode = SupportedXeSSFrameGenModes[NewIndex.Get<int32>()];
		ModifyPendingUpscaler([Mode](FRageUpscalerSettings& S) { S.XeSSFrameGenMode = Mode; });
	}
}

void URageVideoSettingsPanel::HandleReflexModeChanged(FName RowId, FRageVariant NewIndex)
{
	const ERageReflexMode Mode = static_cast<ERageReflexMode>(NewIndex.Get<int32>());
	ModifyPendingUpscaler([Mode](FRageUpscalerSettings& S) { S.ReflexMode = Mode; });
}

void URageVideoSettingsPanel::HandleXeLLChanged(FName RowId, FRageVariant bNewValue)
{
	ModifyPendingUpscaler([bNewValue](FRageUpscalerSettings& S) { S.bXeLLEnabled = bNewValue.Get<bool>(); });
}

void URageVideoSettingsPanel::HandleMotionBlurChanged(FName RowId, FRageVariant bNewValue)
{
	VideoSettings->SetPendingMotionBlurEnabled(bNewValue.Get<bool>());
}

void URageVideoSettingsPanel::HandleDepthOfFieldChanged(FName RowId, FRageVariant bNewValue)
{
	VideoSettings->SetPendingDepthOfFieldEnabled(bNewValue.Get<bool>());
}

void URageVideoSettingsPanel::HandleFilmGrainChanged(FName RowId, FRageVariant bNewValue)
{
	VideoSettings->SetPendingFilmGrainEnabled(bNewValue.Get<bool>());
}

void URageVideoSettingsPanel::HandleChromaticAberrationChanged(FName RowId, FRageVariant bNewValue)
{
	VideoSettings->SetPendingChromaticAberrationEnabled(bNewValue.Get<bool>());
}
