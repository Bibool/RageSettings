// Copyright (c) 2026 Abdallah Boutrif
// SPDX-License-Identifier: MIT

#pragma once

UENUM(BlueprintType)
enum class ERageSettingsCategory : uint8
{
	Game,
	Audio,
	Video,
	Input
};

namespace RageSettingsCategory
{
	constexpr int32 Count = 4;
}