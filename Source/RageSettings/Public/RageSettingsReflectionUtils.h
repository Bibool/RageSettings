// Copyright (c) 2026 Abdallah Boutrif
// SPDX-License-Identifier: MIT

#pragma once

#include "CoreMinimal.h"

namespace RageSettings
{
	RAGESETTINGS_API bool AreObjectsEqual(const UObject* A, const UObject* B);
	RAGESETTINGS_API TArray<FName> CollectChangedProperties(const UObject* A, const UObject* B);
	RAGESETTINGS_API void CopyObjectProperties(UObject* Dest, const UObject* Src);
	RAGESETTINGS_API UObject* CreateShadowInstance(UObject* Outer, const UObject* Source, FName Name = NAME_None);
}
