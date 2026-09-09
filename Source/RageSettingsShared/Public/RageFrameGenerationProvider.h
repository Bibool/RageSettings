// Copyright (c) 2026 Abdallah Boutrif
// SPDX-License-Identifier: MIT

#pragma once

#include "CoreMinimal.h"
#include "Misc/Optional.h"

/* Decides which vendor is allowed to own the D3D12 swap chain this session. */
namespace RageFrameGeneration
{
	enum class EProvider : uint8
	{
		None,
		DLSS,
		FSR,
		XeSS
	};
	
	inline const TCHAR* StreamlineSwapChainProviderName = TEXT("FStreamlineD3D12DXGISwapchainProvider");
	inline const TCHAR* FSRSwapChainProviderName = TEXT("FSRSwapchainProvider");
	inline const TCHAR* XeFGSwapChainProviderName = TEXT("FXeFGDXGISwapChainProvider");

	RAGESETTINGSSHARED_API EProvider ProviderFromSwapChainProviderName(const FString& Name);
	RAGESETTINGSSHARED_API FString GetSelectedSwapChainProviderName();
	RAGESETTINGSSHARED_API EProvider GetSwapChainOwner();
	RAGESETTINGSSHARED_API TOptional<EProvider> ReadPreference();
	RAGESETTINGSSHARED_API void WritePreference(EProvider Provider);
	RAGESETTINGSSHARED_API void ApplyStartupGate();
	RAGESETTINGSSHARED_API void ReleaseStartupGate();
	RAGESETTINGSSHARED_API const TCHAR* ToString(EProvider Provider);
}
