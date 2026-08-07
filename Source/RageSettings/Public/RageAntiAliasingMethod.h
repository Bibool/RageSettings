// Copyright (c) 2026 Abdallah Boutrif
// SPDX-License-Identifier: MIT

#pragma once

UENUM(BlueprintType)
enum class ERageAntiAliasingMethod : uint8
{
	None,
	FXAA,
	TAA,
	MSAA,
	TSR,
	SMAA
};
