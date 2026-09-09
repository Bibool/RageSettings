// Copyright (c) 2026 Abdallah Boutrif
// SPDX-License-Identifier: MIT

#include "RageFrameGenerationProvider.h"

#include "CoreGlobals.h"
#include "Features/IModularFeatures.h"
#include "RHI.h"
#include "HAL/IConsoleManager.h"
#include "Misc/CommandLine.h"
#include "Misc/ConfigCacheIni.h"
#include "Misc/ConfigUtilities.h"
#include "RageSettingsSharedDebug.h"

#if PLATFORM_WINDOWS
#include "Windows/IDXGISwapchainProvider.h"
#endif

namespace
{
	const TCHAR* PreferenceSection = TEXT("RageFrameGeneration");
	const TCHAR* PreferenceKey = TEXT("Provider");
	const TCHAR* XeFGProviderCVar = TEXT("r.XeFG.OverrideSwapChain");
	const TCHAR* StreamlineNoProviderSwitch = TEXT("slnoswapchainprovider");

	FDelegateHandle FSRProviderWatchHandle;

	FString ToPreferenceValue(RageFrameGeneration::EProvider Provider)
	{
		switch (Provider)
		{
			case RageFrameGeneration::EProvider::DLSS: return TEXT("dlss");
			case RageFrameGeneration::EProvider::FSR:  return TEXT("fsr");
			case RageFrameGeneration::EProvider::XeSS: return TEXT("xess");
			default:                                   return TEXT("none");
		}
	}

	TOptional<RageFrameGeneration::EProvider> FromPreferenceValue(const FString& Value)
	{
		if (Value.Equals(TEXT("dlss"))) return RageFrameGeneration::EProvider::DLSS;
		if (Value.Equals(TEXT("fsr")))  return RageFrameGeneration::EProvider::FSR;
		if (Value.Equals(TEXT("xess"))) return RageFrameGeneration::EProvider::XeSS;
		if (Value.Equals(TEXT("none"))) return RageFrameGeneration::EProvider::None;

		return NullOpt;
	}

	void SetProviderCVar(const TCHAR* Name, bool bAllowed)
	{
		UE::ConfigUtilities::OnSetCVarFromIniEntry(*GGameUserSettingsIni, Name, bAllowed ? TEXT("1") : TEXT("0"), ECVF_SetByGameOverride);
	}

	/** NVidia and Intel expose a switch to skip registration but ofc AMD does not so we have to get hacky
	 * and intercept the registration call. */
	void SuppressFSRSwapChainProvider()
	{
#if PLATFORM_WINDOWS
		FSRProviderWatchHandle = IModularFeatures::Get().OnModularFeatureRegistered().AddLambda(
			[](const FName& Type, IModularFeature* Feature)
			{
				if (Type != IDXGISwapchainProvider::GetModularFeatureName() || !Feature)
				{
					return;
				}

				IDXGISwapchainProvider* Provider = static_cast<IDXGISwapchainProvider*>(Feature);
				if (FCString::Strcmp(Provider->GetProviderName(), RageFrameGeneration::FSRSwapChainProviderName) != 0)
				{
					return;
				}
				
				IModularFeatures::Get().UnregisterModularFeature(Type, Feature);

				S_LOG(Log, "Rage Settings:FSR swap chain provider removed as another vendor has the swap chain this session.");
			});
#endif
	}
}

RageFrameGeneration::EProvider RageFrameGeneration::ProviderFromSwapChainProviderName(const FString& Name)
{
	if (Name == StreamlineSwapChainProviderName) return EProvider::DLSS;
	if (Name == FSRSwapChainProviderName)        return EProvider::FSR;
	if (Name == XeFGSwapChainProviderName)       return EProvider::XeSS;

	return EProvider::None;
}

FString RageFrameGeneration::GetSelectedSwapChainProviderName()
{
#if PLATFORM_WINDOWS
	if (RHIGetInterfaceType() != ERHIInterfaceType::D3D12)
	{
		return FString();
	}

	/* Mirrors FD3D12Viewport::Init: the first registered provider that claims D3D12 wins and the rest are never consulted. */
	IModularFeatures::FScopedLockModularFeatureList Lock;
	for (const IDXGISwapchainProvider* Provider : IModularFeatures::Get().GetModularFeatureImplementations<IDXGISwapchainProvider>(IDXGISwapchainProvider::GetModularFeatureName()))
	{
		if (Provider && Provider->SupportsRHI(ERHIInterfaceType::D3D12))
		{
			return Provider->GetProviderName();
		}
	}
#endif

	return FString();
}

RageFrameGeneration::EProvider RageFrameGeneration::GetSwapChainOwner()
{
	return ProviderFromSwapChainProviderName(GetSelectedSwapChainProviderName());
}

const TCHAR* RageFrameGeneration::ToString(EProvider Provider)
{
	switch (Provider)
	{
		case EProvider::DLSS: return TEXT("DLSS");
		case EProvider::FSR:  return TEXT("FSR");
		case EProvider::XeSS: return TEXT("XeSS");
		default:              return TEXT("None");
	}
}

TOptional<RageFrameGeneration::EProvider> RageFrameGeneration::ReadPreference()
{
	FString Value;
	if (GConfig && GConfig->GetString(PreferenceSection, PreferenceKey, Value, GGameUserSettingsIni))
	{
		return FromPreferenceValue(Value);
	}

	return NullOpt;
}

void RageFrameGeneration::WritePreference(EProvider Provider)
{
	if (!GConfig)
	{
		return;
	}
	
	if (Provider == EProvider::None)
	{
		return;
	}

	const TOptional<EProvider> Stored = ReadPreference();
	if (Stored.IsSet() && *Stored == Provider)
	{
		return;
	}

	GConfig->SetString(PreferenceSection, PreferenceKey, *ToPreferenceValue(Provider), GGameUserSettingsIni);
	
	GConfig->Flush(false, GGameUserSettingsIni);
}

void RageFrameGeneration::ApplyStartupGate()
{
	const TOptional<EProvider> Preference = ReadPreference();
	
	if (!Preference.IsSet() || *Preference == EProvider::None)
	{
		return;
	}

	const EProvider Provider = *Preference;

	if (Provider != EProvider::DLSS)
	{
		FCommandLine::Append(TEXT(" -"));
		FCommandLine::Append(StreamlineNoProviderSwitch);
	}

	if (Provider != EProvider::FSR)
	{
		SuppressFSRSwapChainProvider();
	}

	SetProviderCVar(XeFGProviderCVar, Provider == EProvider::XeSS);
}

void RageFrameGeneration::ReleaseStartupGate()
{
	if (FSRProviderWatchHandle.IsValid())
	{
		IModularFeatures::Get().OnModularFeatureRegistered().Remove(FSRProviderWatchHandle);
		FSRProviderWatchHandle.Reset();
	}
}
