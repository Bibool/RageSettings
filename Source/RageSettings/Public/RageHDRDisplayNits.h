// Copyright (c) 2026 Abdallah Boutrif
// SPDX-License-Identifier: MIT

#pragma once

/** Engine only maps out to 1000 / 2000 nits. 
 * @see WindowsPlatformMisc.h, FGenericPlatformMisc::ChooseHDRDeviceAndColorGamut() */
UENUM(BlueprintType)
enum class ERageHDRDisplayNits : uint8
{
	Nits1000 UMETA(DisplayName = "1000 nits"),
	Nits2000 UMETA(DisplayName = "2000 nits")
};

namespace RageHDRDisplayNits
{
	static int32 Nits1000 = 1000;
	static int32 Nits2000 = 2000;
	
	static int32 ToInt32(const ERageHDRDisplayNits& InVal)
	{
		return InVal == ERageHDRDisplayNits::Nits1000 ? Nits1000 : Nits2000;
	}
}