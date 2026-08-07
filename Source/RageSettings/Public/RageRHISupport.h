// Copyright (c) 2026 Abdallah Boutrif
// SPDX-License-Identifier: MIT

#pragma once

#include "CoreMinimal.h"
#include "RageRHIType.h"

/**
 * Reading and writing the graphics API the engine will boot with next time.
 * By default, the engine picks and creates it in PreInit, long before any game code runs.
 * Therefore settings can only affect the next boot (Write wanted RHI, on boot, reads it) */
namespace RageRHI
{
	RAGESETTINGS_API bool IsSelectionSupported();
	RAGESETTINGS_API ERageRHIType GetActiveType();
	RAGESETTINGS_API ERageRHIType GetProjectDefaultType();
	RAGESETTINGS_API ERageRHIType ResolveEffectiveType(ERageRHIType Type);
	RAGESETTINGS_API TArray<ERageRHIType> GetSelectableTypes();
	RAGESETTINGS_API bool IsTypeSelectable(ERageRHIType Type);
	RAGESETTINGS_API ERageRHIType ReadPreference();
	RAGESETTINGS_API void WritePreference(ERageRHIType Type);
}
