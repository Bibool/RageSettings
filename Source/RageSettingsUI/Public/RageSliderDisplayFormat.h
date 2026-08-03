// Copyright (c) 2026 Abdallah Boutrif
// SPDX-License-Identifier: MIT

#pragma once

/** How a slider row should format its numeric readout. */
UENUM(BlueprintType)
enum class ERageSliderDisplayFormat : uint8
{
	/* 1.00 */
	Raw,
	/* 100% */
	Percent,
	/* 1.00x */
	Multiplier,
	/* 1 */
	Integer,
};
