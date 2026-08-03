// Copyright (c) 2026 Abdallah Boutrif
// SPDX-License-Identifier: MIT

#pragma once

#include "RageDLSSMode.h"
#include "RageFrameGenerationMode.h"
#include "RageFSRMode.h"
#include "RageReflexMode.h"
#include "RageUpscalerMethod.h"
#include "RageXeSSMode.h"
#include "RageUpscalerSettings.generated.h"

USTRUCT(BlueprintType)
struct FRageUpscalerSettings
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, Category = "Rage|Upscaling")
	ERageUpscalerMethod Method = ERageUpscalerMethod::TSR;

	UPROPERTY(BlueprintReadWrite, Category = "Rage|Upscaling")
	ERageDLSSMode DLSSMode = ERageDLSSMode::Quality;

	UPROPERTY(BlueprintReadWrite, Category = "Rage|Upscaling")
	bool bDLSSRayReconstruction = false;

	UPROPERTY(BlueprintReadWrite, Category = "Rage|Upscaling")
	ERageFrameGenerationMode DLSSFrameGenMode = ERageFrameGenerationMode::Off;

	UPROPERTY(BlueprintReadWrite, Category = "Rage|Upscaling")
	ERageFSRMode FSRMode = ERageFSRMode::Quality;

	UPROPERTY(BlueprintReadWrite, Category = "Rage|Upscaling")
	float FSRSharpness = 0.f;

	UPROPERTY(BlueprintReadWrite, Category = "Rage|Upscaling")
	bool bFSRFrameInterpolation = false;

	UPROPERTY(BlueprintReadWrite, Category = "Rage|Upscaling")
	ERageXeSSMode XeSSMode = ERageXeSSMode::Quality;

	UPROPERTY(BlueprintReadWrite, Category = "Rage|Upscaling")
	ERageFrameGenerationMode XeSSFrameGenMode = ERageFrameGenerationMode::Off;

	UPROPERTY(BlueprintReadWrite, Category = "Rage|Upscaling")
	ERageReflexMode ReflexMode = ERageReflexMode::Enabled;
	
	UPROPERTY(BlueprintReadWrite, Category = "Rage|Upscaling")
	bool bXeLLEnabled = false;
};
