// Copyright (c) 2026 Abdallah Boutrif
// SPDX-License-Identifier: MIT

#pragma once

#include "RageRayTracingSettings.generated.h"

USTRUCT(BlueprintType)
struct FRageRayTracingSettings
{
	GENERATED_BODY()

	/* Master switch for all RayTrace options. */
	UPROPERTY(BlueprintReadWrite, Category = "Rage|RayTracing")
	bool bEnabled = false;

	UPROPERTY(BlueprintReadWrite, Category = "Rage|RayTracing")
	bool bShadows = false;

	UPROPERTY(BlueprintReadWrite, Category = "Rage|RayTracing")
	bool bReflections = false;

	UPROPERTY(BlueprintReadWrite, Category = "Rage|RayTracing")
	bool bGlobalIllumination = false;

	UPROPERTY(BlueprintReadWrite, Category = "Rage|RayTracing")
	bool bAmbientOcclusion = false;

	UPROPERTY(BlueprintReadWrite, Category = "Rage|RayTracing")
	bool bTranslucency = false;
};

