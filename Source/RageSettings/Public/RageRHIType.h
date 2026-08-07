// Copyright (c) 2026 Abdallah Boutrif
// SPDX-License-Identifier: MIT

#pragma once

/** Graphics API the game asks the engine to boot with.
 * @see RageRHISupport.h */
UENUM(BlueprintType)
enum class ERageRHIType : uint8
{
	/* Aka engine default. */
	Auto,
	DirectX11 UMETA(DisplayName = "DirectX 11"),
	DirectX12 UMETA(DisplayName = "DirectX 12"),
	Vulkan
};
