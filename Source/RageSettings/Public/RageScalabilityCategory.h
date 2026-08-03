// Copyright (c) 2026 Abdallah Boutrif
// SPDX-License-Identifier: MIT

#pragma once

UENUM(BlueprintType)
enum class ERageScalabilityCategory : uint8
{
	ViewDistance,
	AntiAliasing,
	Shadow,
	GlobalIllumination,
	Reflection,
	PostProcess,
	Texture,
	Effects,
	Foliage,
	Shading
};

namespace RageScalabilityCategory
{
	static int32 Min = StaticCast<int32>(ERageScalabilityCategory::ViewDistance);
	static int32 Max = StaticCast<int32>(ERageScalabilityCategory::Shading);
}
