// Copyright (c) 2026 Abdallah Boutrif
// SPDX-License-Identifier: MIT

#pragma once

#include "CoreMinimal.h"
#include "RageMonitorInfo.h"


namespace RageMonitor
{
	RAGESETTINGS_API TArray<FRageMonitorInfo> GetMonitors();
	RAGESETTINGS_API bool IsSelectionSupported();
	RAGESETTINGS_API FString GetActiveMonitorId();
	RAGESETTINGS_API FString ResolveMonitorId(const FString& PreferredId);
	RAGESETTINGS_API bool MoveGameWindowToMonitor(const FString& MonitorId);
}
