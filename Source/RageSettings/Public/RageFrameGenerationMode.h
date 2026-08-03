// Copyright (c) 2026 Abdallah Boutrif
// SPDX-License-Identifier: MIT

#pragma once

UENUM(BlueprintType)
enum class ERageFrameGenerationMode : uint8
{
	Off UMETA(DisplayName = "Off"),
	x2  UMETA(DisplayName = "2x"),
	x3  UMETA(DisplayName = "3x"),
	x4  UMETA(DisplayName = "4x")
};
