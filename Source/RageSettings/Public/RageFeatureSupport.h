// Copyright (c) 2026 Abdallah Boutrif
// SPDX-License-Identifier: MIT

#pragma once

UENUM(BlueprintType)
enum class ERageFeatureSupport : uint8
{
	Supported,

	/* Available but unsupported, ideally not used. */
	NotSupported,

	/* Hardware required. */
	IncompatibleHardware,

	/* Actionable with drive update. */
	DriverOutOfDate,

	/* Actionable with OS update. */
	OperatingSystemOutOfDate,

	/* RHI does not support this.  */
	NotSupportedByRHI,

	/* Actionable in Windows, Hardware-Accelerated GPU Scheduling is switched off. */
	HardwareSchedulingDisabled
};
