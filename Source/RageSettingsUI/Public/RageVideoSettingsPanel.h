// Copyright (c) 2026 Abdallah Boutrif
// SPDX-License-Identifier: MIT

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "RageSettingsPanelInterface.h"
#include "RageVariant.h"
#include "RageVideoSettingsPanel.generated.h"

class URageSelectionRow;
class URageRowBaseUserWidget;
class URageVideoSettings;
class URageToggleRow;
class URageSliderRow;
class URageComboRow;
class UPanelWidget;
class UWidgetSwitcher;
struct FRageRayTracingSettings;
struct FRageUpscalerSettings;
enum class ERageUpscalerMethod : uint8;
enum class ERageScalabilityCategory : uint8;
enum class ERageDLSSMode : uint8;
enum class ERageXeSSMode : uint8;
enum class ERageFrameGenerationMode : uint8;

/** Implements majority of video settings used by Unreal Engine games.
 * For the sake of allowing what settings gets created here, all widgets are BindOptional, meaning you can define which you want. 
 * For instance if you don't want foliage, simply don't define it in UMG. 
 * For scalability, go to RageSettingsUIDeveloperSettings and define which scalability settings you want to be generated.
 * It's advised to keep all defined and rather undefine which you don't want, and keep in mind the logic of how some setting interact with another.
 * E.g. Masters (RayTracing, Upscalers), Presets (and Scalability), etc.
 */
UCLASS()
class RAGESETTINGSUI_API URageVideoSettingsPanel : public UUserWidget, public IRageSettingsPanelInterface
{
	GENERATED_BODY()

public:
//~ Begin IRageSettingsPanel
	virtual void InitializePanel(URageSettingsSubsystem* InSubsystem) override;
	virtual void RefreshFromSettings() override;
	virtual ERageSettingsCategory GetCategoryId() const override { return ERageSettingsCategory::Video; }
//~ End IRageSettingsPanel

protected:
	virtual void NativeConstruct() override;

	UPROPERTY(EditDefaultsOnly) 
	TSubclassOf<URageRowBaseUserWidget> ScalabilityRowClass = nullptr;
	
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional)) 
	TObjectPtr<URageComboRow> ResolutionRow = nullptr;
	
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional)) 
	TObjectPtr<URageSelectionRow> WindowModeRow = nullptr;
	
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional)) 
	TObjectPtr<URageToggleRow> VSyncRow = nullptr;
	
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional)) 
	TObjectPtr<URageSliderRow> FrameRateLimitRow = nullptr;
	
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional)) 
	TObjectPtr<URageSliderRow> ResolutionScaleRow = nullptr;
	
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional)) 
	TObjectPtr<URageSliderRow> BrightnessRow = nullptr;
	
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<URageSliderRow> FieldOfViewRow = nullptr;
	
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional)) 
	TObjectPtr<URageToggleRow> HDRRow = nullptr;
	
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional)) 
	TObjectPtr<URageSelectionRow> HDRNitsRow = nullptr;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional)) 
	TObjectPtr<URageSelectionRow> QualityPresetRow = nullptr;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional)) 
	TObjectPtr<UPanelWidget> ScalabilityContainer = nullptr;
	
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional)) 
	TObjectPtr<URageToggleRow> RayTracingMasterRow = nullptr;
	
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional)) 
	TObjectPtr<URageToggleRow> RTShadowsRow = nullptr;
	
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional)) 
	TObjectPtr<URageToggleRow> RTReflectionsRow = nullptr;
	
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional)) 
	TObjectPtr<URageToggleRow> RTGlobalIlluminationRow = nullptr;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional)) 
	TObjectPtr<URageToggleRow> RTAmbientOcclusionRow = nullptr;
	
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional)) 
	TObjectPtr<URageToggleRow> RTTranslucencyRow = nullptr;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional)) 
	TObjectPtr<URageSelectionRow> UpscalerMethodRow = nullptr;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional)) 
	TObjectPtr<UWidgetSwitcher> UpscalerOptionsSwitcher = nullptr;
	
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional)) 
	TObjectPtr<URageSelectionRow> DLSSModeRow = nullptr;
	
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional)) 
	TObjectPtr<URageToggleRow> DLSSRayReconstructionRow = nullptr;
	
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional)) 
	TObjectPtr<URageSelectionRow> DLSSFrameGenRow = nullptr;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional)) 
	TObjectPtr<URageSelectionRow> FSRModeRow = nullptr;
	
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional)) 
	TObjectPtr<URageSliderRow> FSRSharpnessRow = nullptr;
	
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional)) 
	TObjectPtr<URageToggleRow> FSRFrameInterpolationRow = nullptr;
	
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<URageSelectionRow> XeSSModeRow = nullptr;
	
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional)) 
	TObjectPtr<URageSelectionRow> XeSSFrameGenRow = nullptr;
	
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional)) 
	TObjectPtr<URageSelectionRow> ReflexModeRow = nullptr;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional)) 
	TObjectPtr<URageToggleRow> XeLLRow = nullptr;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional)) 
	TObjectPtr<URageToggleRow> MotionBlurRow = nullptr;
	
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional)) 
	TObjectPtr<URageToggleRow> DepthOfFieldRow = nullptr;
	
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional)) 
	TObjectPtr<URageToggleRow> FilmGrainRow = nullptr;
	
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional)) 
	TObjectPtr<URageToggleRow> ChromaticAberrationRow = nullptr;

private:
	void BuildScalabilityRows();
	void RefreshScalabilityRows();
	void RefreshQualityPresetRow();
	void RefreshResolutionOptions();
	void RefreshUpscalerModeOptions();
	void RefreshRayTracingSubRowsEnabled(bool bMasterEnabled);
	void RefreshUpscalerOptionsVisibility(ERageUpscalerMethod Method);
	void RefreshDLSSRayReconstructionRowEnabled();
	void ModifyPendingRayTracing(TFunctionRef<void(FRageRayTracingSettings&)> Mutator);
	void ModifyPendingUpscaler(TFunctionRef<void(FRageUpscalerSettings&)> Mutator);

	void HandleResolutionChanged(FName RowId, FRageVariant NewIndex);
	void HandleWindowModeChanged(FName RowId, FRageVariant NewIndex);
	void HandleVSyncChanged(FName RowId, FRageVariant bNewValue);
	void HandleFrameRateLimitChanged(FName RowId, FRageVariant NewValue);
	void HandleResolutionScaleChanged(FName RowId, FRageVariant NewValue);
	void HandleBrightnessChanged(FName RowId, FRageVariant NewValue);
	void HandleFieldOfViewChanged(FName RowId, FRageVariant NewValue);
	void HandleHDRChanged(FName RowId, FRageVariant bNewValue);
	void HandleHDRNitsChanged(FName RowId, FRageVariant NewIndex);
	void HandleQualityPresetChanged(FName RowId, FRageVariant NewIndex);
	void HandleScalabilityChanged(FName RowId, FRageVariant NewIndex);
	void HandleRayTracingMasterChanged(FName RowId, FRageVariant bNewValue);
	void HandleRTShadowsChanged(FName RowId, FRageVariant bNewValue);
	void HandleRTReflectionsChanged(FName RowId, FRageVariant bNewValue);
	void HandleRTGlobalIlluminationChanged(FName RowId, FRageVariant bNewValue);
	void HandleRTAmbientOcclusionChanged(FName RowId, FRageVariant bNewValue);
	void HandleRTTranslucencyChanged(FName RowId, FRageVariant bNewValue);
	void HandleUpscalerMethodChanged(FName RowId, FRageVariant NewIndex);
	void HandleDLSSModeChanged(FName RowId, FRageVariant NewIndex);
	void HandleDLSSFrameGenChanged(FName RowId, FRageVariant NewIndex);
	void HandleDLSSRayReconstructionChanged(FName RowId, FRageVariant bNewValue);
	void HandleFSRModeChanged(FName RowId, FRageVariant NewIndex);
	void HandleFSRSharpnessChanged(FName RowId, FRageVariant NewValue);
	void HandleFSRFrameInterpolationChanged(FName RowId, FRageVariant bNewValue);
	void HandleXeSSModeChanged(FName RowId, FRageVariant NewIndex);
	void HandleXeSSFrameGenChanged(FName RowId, FRageVariant NewIndex);
	void HandleReflexModeChanged(FName RowId, FRageVariant NewIndex);
	void HandleXeLLChanged(FName RowId, FRageVariant bNewValue);
	void HandleMotionBlurChanged(FName RowId, FRageVariant bNewValue);
	void HandleDepthOfFieldChanged(FName RowId, FRageVariant bNewValue);
	void HandleFilmGrainChanged(FName RowId, FRageVariant bNewValue);
	void HandleChromaticAberrationChanged(FName RowId, FRageVariant bNewValue);
	
	UPROPERTY()
	TObjectPtr<URageVideoSettings> VideoSettings = nullptr;

	UPROPERTY()
	TArray<TObjectPtr<URageSelectionRow>> ScalabilityRows;
	
	TMap<FName, ERageScalabilityCategory> ScalabilityCategoryByRowId;
	TArray<FIntPoint> CachedResolutions;
	TArray<ERageUpscalerMethod> SupportedUpscalerMethods;
	TArray<ERageDLSSMode> SupportedDLSSModes;
	TArray<ERageXeSSMode> SupportedXeSSModes;
	TArray<ERageFrameGenerationMode> SupportedDLSSFrameGenModes;
	TArray<ERageFrameGenerationMode> SupportedXeSSFrameGenModes;
};
