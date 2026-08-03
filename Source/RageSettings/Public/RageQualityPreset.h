// Copyright (c) 2026 Abdallah Boutrif
// SPDX-License-Identifier: MIT

#pragma once

UENUM(BlueprintType)
enum class ERageQualityPreset : uint8
{
	Low,
	Medium,
	High,
	Epic,
	Custom
};

namespace RageQualityPreset
{
	static int32 Min = StaticCast<int32>(ERageQualityPreset::Low);
	static int32 MaxQuality = StaticCast<int32>(ERageQualityPreset::Epic);
	static int32 Max = StaticCast<int32>(ERageQualityPreset::Custom);
}
