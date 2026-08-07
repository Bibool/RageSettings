// Copyright (c) 2026 Abdallah Boutrif
// SPDX-License-Identifier: MIT

#include "RageRHISupport.h"

#include "DynamicRHI.h"
#include "CoreGlobals.h"
#include "HAL/PlatformMisc.h"
#include "HAL/PlatformProcess.h"
#include "Misc/ConfigCacheIni.h"
#include "RageSettingsShared/Public/RageSettingsSharedDebug.h"

#if PLATFORM_WINDOWS
THIRD_PARTY_INCLUDES_START
#include "Windows/AllowWindowsPlatformTypes.h"
#include <d3d11.h>
#include <d3d12.h>
#include "Windows/HideWindowsPlatformTypes.h"
#include "vulkan_core.h"
THIRD_PARTY_INCLUDES_END
#endif

namespace
{
	/** This code was made possible with a variety of resources; 
	 * https://codeberg.org/jerobarraco/JUtils/src/commit/baeb50e68dfa5520c09e5470676792a7c200161d/Source/JUtils/Misc/JUtilsSys.cpp
	 * https://learn.microsoft.com/en-us/windows/win32/api/d3d12/nf-d3d12-d3d12createdevice
	 * WindowsDynamicRHI.cpp
	 * WindowsD3D12Device.cpp
	 * vulkan_core.h */
	const TCHAR* PreferenceSection = TEXT("D3DRHIPreference");
	const TCHAR* PreferenceKey = TEXT("PreferredRHI");
	const TCHAR* TargetSettingsSection = TEXT("/Script/WindowsTargetPlatform.WindowsTargetSettings");
	constexpr ERageRHIType SelectableCandidates[] = { ERageRHIType::DirectX11, ERageRHIType::DirectX12, ERageRHIType::Vulkan };

	FString ToPreferenceValue(const ERageRHIType Type)
	{
		switch (Type)
		{
			case ERageRHIType::DirectX11: return TEXT("dx11");
			case ERageRHIType::DirectX12: return TEXT("dx12");
			case ERageRHIType::Vulkan:    return TEXT("vulkan");
			default:                      return FString();
		}
	}

	ERageRHIType FromPreferenceValue(const FString& Value)
	{
		if (Value == TEXT("dx11"))   return ERageRHIType::DirectX11;
		if (Value == TEXT("dx12"))   return ERageRHIType::DirectX12;
		if (Value == TEXT("vulkan")) return ERageRHIType::Vulkan;
		
		return ERageRHIType::Auto;
	}

	const TCHAR* TargetedShaderFormatsKey(ERageRHIType Type)
	{
		switch (Type)
		{
			case ERageRHIType::DirectX11: return TEXT("D3D11TargetedShaderFormats");
			case ERageRHIType::DirectX12: return TEXT("D3D12TargetedShaderFormats");
			case ERageRHIType::Vulkan:    return TEXT("VulkanTargetedShaderFormats");
			default:                      return nullptr;
		}
	}

	/** Checks whether the project ships shaders for an API, the same TargetedShaderFormats arrays
	 * ParseWindowsDynamicRHIConfig() reads. A cooked build that is asked for an API it has no shader
	 * format for dies (read crash) in ChooseFeatureLevel()	 */
	bool HasTargetedShaderFormats(const ERageRHIType Type)
	{
		const TCHAR* Key = TargetedShaderFormatsKey(Type);
		if (!Key || !GConfig)
		{
			return false;
		}

		TArray<FString> Formats;
		GConfig->GetArray(TargetSettingsSection, Key, Formats, GEngineIni);
		if (Formats.Num() > 0)
		{
			return true;
		}

		GConfig->GetArray(TargetSettingsSection, TEXT("TargetedRHIs"), Formats, GEngineIni);
		return Formats.Num() > 0;
	}

#if PLATFORM_WINDOWS
	bool ProbeD3D11()
	{
		void* Dll = FPlatformProcess::GetDllHandle(TEXT("d3d11.dll"));
		if (!Dll)
		{
			return false;
		}

		using FCreateDevice = HRESULT(WINAPI*)(IDXGIAdapter*, D3D_DRIVER_TYPE, HMODULE, UINT, const D3D_FEATURE_LEVEL*, UINT, UINT, ID3D11Device**, D3D_FEATURE_LEVEL*, ID3D11DeviceContext**);
		const FCreateDevice CreateDevice = reinterpret_cast<FCreateDevice>(FPlatformProcess::GetDllExport(Dll, TEXT("D3D11CreateDevice")));

		bool bSupported = false;
		if (CreateDevice)
		{
			constexpr D3D_FEATURE_LEVEL Requested[] = { D3D_FEATURE_LEVEL_11_0 };
			D3D_FEATURE_LEVEL Supported = static_cast<D3D_FEATURE_LEVEL>(0);
			const HRESULT Result = CreateDevice(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, 0, Requested, UE_ARRAY_COUNT(Requested), D3D11_SDK_VERSION, nullptr, &Supported, nullptr);

			bSupported = SUCCEEDED(Result) && Supported >= D3D_FEATURE_LEVEL_11_0;
		}

		FPlatformProcess::FreeDllHandle(Dll);
		return bSupported;
	}

	bool ProbeD3D12()
	{
		if (!FPlatformMisc::VerifyWindowsVersion(10, 0, 15063))
		{
			return false;
		}

		void* Dll = FPlatformProcess::GetDllHandle(TEXT("d3d12.dll"));
		if (!Dll)
		{
			return false;
		}

		using FCreateDevice = HRESULT(WINAPI*)(IUnknown*, D3D_FEATURE_LEVEL, REFIID, void**);
		const FCreateDevice CreateDevice = reinterpret_cast<FCreateDevice>(FPlatformProcess::GetDllExport(Dll, TEXT("D3D12CreateDevice")));

		bool bSupported = false;
		if (CreateDevice)
		{
			bSupported = SUCCEEDED(CreateDevice(nullptr, D3D_FEATURE_LEVEL_11_0, __uuidof(ID3D12Device), nullptr));
		}

		FPlatformProcess::FreeDllHandle(Dll);
		return bSupported;
	}

	bool ProbeVulkan()
	{
		void* Dll = FPlatformProcess::GetDllHandle(TEXT("vulkan-1.dll"));
		if (!Dll)
		{
			return false;
		}

		using FCreateInstance = VkResult(VKAPI_PTR*)(const VkInstanceCreateInfo*, const VkAllocationCallbacks*, VkInstance*);
		using FDestroyInstance = void(VKAPI_PTR*)(VkInstance, const VkAllocationCallbacks*);
		using FEnumeratePhysicalDevices = VkResult(VKAPI_PTR*)(VkInstance, uint32_t*, VkPhysicalDevice*);

		const auto CreateInstance = reinterpret_cast<FCreateInstance>(FPlatformProcess::GetDllExport(Dll, TEXT("vkCreateInstance")));
		const auto DestroyInstance = reinterpret_cast<FDestroyInstance>(FPlatformProcess::GetDllExport(Dll, TEXT("vkDestroyInstance")));
		const auto EnumeratePhysicalDevices = reinterpret_cast<FEnumeratePhysicalDevices>(FPlatformProcess::GetDllExport(Dll, TEXT("vkEnumeratePhysicalDevices")));

		bool bSupported = false;
		if (CreateInstance && DestroyInstance && EnumeratePhysicalDevices)
		{
			VkApplicationInfo AppInfo = {};
			AppInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
			AppInfo.apiVersion = VK_API_VERSION_1_0;

			VkInstanceCreateInfo InstanceInfo = {};
			InstanceInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
			InstanceInfo.pApplicationInfo = &AppInfo;

			VkInstance Instance = VK_NULL_HANDLE;
			if (CreateInstance(&InstanceInfo, nullptr, &Instance) == VK_SUCCESS)
			{
				uint32_t DeviceCount = 0;
				bSupported = EnumeratePhysicalDevices(Instance, &DeviceCount, nullptr) == VK_SUCCESS && DeviceCount > 0;
				DestroyInstance(Instance, nullptr);
			}
		}

		FPlatformProcess::FreeDllHandle(Dll);
		return bSupported;
	}
#endif
	
	bool IsHardwareCapable(ERageRHIType Type)
	{
		/* Engine boots by default with a support type, so meaning the default is supported. */
		if (Type == RageRHI::GetActiveType())
		{
			return true;
		}

#if PLATFORM_WINDOWS
		switch (Type)
		{
			case ERageRHIType::DirectX11: { static const bool bCached = ProbeD3D11();  return bCached; }
			case ERageRHIType::DirectX12: { static const bool bCached = ProbeD3D12();  return bCached; }
			case ERageRHIType::Vulkan:    { static const bool bCached = ProbeVulkan(); return bCached; }
			default: break;
		}
#endif

		return false;
	}
}

bool RageRHI::IsSelectionSupported()
{
#if PLATFORM_WINDOWS
	const ERageRHIType ProjectDefault = GetProjectDefaultType();
	return ProjectDefault == ERageRHIType::DirectX11 || ProjectDefault == ERageRHIType::DirectX12;
#else
	return false;
#endif
}

ERageRHIType RageRHI::GetActiveType()
{
	if (!GDynamicRHI)
	{
		return ERageRHIType::Auto;
	}

	switch (RHIGetInterfaceType())
	{
		case ERHIInterfaceType::D3D11:  return ERageRHIType::DirectX11;
		case ERHIInterfaceType::D3D12:  return ERageRHIType::DirectX12;
		case ERHIInterfaceType::Vulkan: return ERageRHIType::Vulkan;
		default:                        return ERageRHIType::Auto;
	}
}

ERageRHIType RageRHI::GetProjectDefaultType()
{
	FString DefaultGraphicsRHI;
	if (GConfig && GConfig->GetString(TargetSettingsSection, TEXT("DefaultGraphicsRHI"), DefaultGraphicsRHI, GEngineIni))
	{
		if (DefaultGraphicsRHI == TEXT("DefaultGraphicsRHI_DX11"))   return ERageRHIType::DirectX11;
		if (DefaultGraphicsRHI == TEXT("DefaultGraphicsRHI_DX12"))   return ERageRHIType::DirectX12;
		if (DefaultGraphicsRHI == TEXT("DefaultGraphicsRHI_Vulkan")) return ERageRHIType::Vulkan;
	}
	
	for (const ERageRHIType Candidate : { ERageRHIType::DirectX12, ERageRHIType::DirectX11, ERageRHIType::Vulkan })
	{
		if (HasTargetedShaderFormats(Candidate))
		{
			return Candidate;
		}
	}

	return ERageRHIType::DirectX11; /* Last resort, we shouldnt reach here but safety first */
}

ERageRHIType RageRHI::ResolveEffectiveType(const ERageRHIType Type)
{
	return Type == ERageRHIType::Auto ? GetProjectDefaultType() : Type;
}

TArray<ERageRHIType> RageRHI::GetSelectableTypes()
{
	TArray<ERageRHIType> Types;
	if (!IsSelectionSupported())
	{
		return Types;
	}

	Types.Add(ERageRHIType::Auto);
	for (const ERageRHIType Candidate : SelectableCandidates)
	{
		if (IsTypeSelectable(Candidate))
		{
			Types.Add(Candidate);
		}
	}

	return Types;
}

bool RageRHI::IsTypeSelectable(ERageRHIType Type)
{
	if (Type == ERageRHIType::Auto)
	{
		return IsSelectionSupported();
	}

	return HasTargetedShaderFormats(Type) && IsHardwareCapable(Type);
}

ERageRHIType RageRHI::ReadPreference()
{
	FString Value;
	if (GConfig && GConfig->GetString(PreferenceSection, PreferenceKey, Value, GGameUserSettingsIni))
	{
		return FromPreferenceValue(Value);
	}

	return ERageRHIType::Auto;
}

void RageRHI::WritePreference(ERageRHIType Type)
{
	if (!GConfig)
	{
		return;
	}

	if (Type == ERageRHIType::Auto)
	{
		GConfig->RemoveKey(PreferenceSection, PreferenceKey, GGameUserSettingsIni);
	}
	else
	{
		GConfig->SetString(PreferenceSection, PreferenceKey, *ToPreferenceValue(Type), GGameUserSettingsIni);
	}

	/* RHI is a special case, we don't write it to our own settings ini but directly to the GConfig, this value will be read on boot! */
	GConfig->Flush(false, GGameUserSettingsIni);

	const FString Written = Type == ERageRHIType::Auto ? FString(TEXT("<project default>")) : ToPreferenceValue(Type);
	S_LOG(Log, "Rage Settings: preferred RHI set to '{value}', taking effect on next launch.", Written);
}
