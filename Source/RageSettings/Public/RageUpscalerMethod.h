// Copyright (c) 2026 Abdallah Boutrif
// SPDX-License-Identifier: MIT

#pragma once

UENUM(BlueprintType)
enum class ERageUpscalerMethod : uint8
{
	Off,
	TAAU,
	TSR,
	FSR,
	DLSS,
	XeSS
};
