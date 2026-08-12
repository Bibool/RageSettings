// Copyright (c) 2026 Abdallah Boutrif
// SPDX-License-Identifier: MIT

#include "RageMonitorSupport.h"

#include "Engine/Engine.h"
#include "Engine/GameViewportClient.h"
#include "GenericPlatform/GenericApplication.h"
#include "Widgets/SWindow.h"
#include "RageSettingsShared/Public/RageSettingsSharedDebug.h"

namespace
{
	TSharedPtr<SWindow> GetGameWindow()
	{
		return GEngine && GEngine->GameViewport ? GEngine->GameViewport->GetWindow() : nullptr;
	}

	const FRageMonitorInfo* FindById(const TArray<FRageMonitorInfo>& Monitors, const FString& Id)
	{
		return Monitors.FindByPredicate([&Id](const FRageMonitorInfo& Monitor) { return Monitor.Id == Id; });
	}
}

TArray<FRageMonitorInfo> RageMonitor::GetMonitors()
{
	FDisplayMetrics DisplayMetrics;
	FDisplayMetrics::RebuildDisplayMetrics(DisplayMetrics);

	TArray<FRageMonitorInfo> Monitors;
	Monitors.Reserve(DisplayMetrics.MonitorInfo.Num());

	for (const FMonitorInfo& Info : DisplayMetrics.MonitorInfo)
	{
		FRageMonitorInfo& Monitor = Monitors.AddDefaulted_GetRef();
		
		Monitor.Id = Info.ID.IsEmpty() ? FString::Printf(TEXT("%d,%d"), Info.DisplayRect.Left, Info.DisplayRect.Top) : Info.ID;
		Monitor.Name = Info.Name;
		Monitor.Position = FIntPoint(Info.DisplayRect.Left, Info.DisplayRect.Top);
		Monitor.DisplaySize = FIntPoint(Info.DisplayRect.Right - Info.DisplayRect.Left,Info.DisplayRect.Bottom - Info.DisplayRect.Top);
		Monitor.NativeResolution = FIntPoint(Info.NativeWidth, Info.NativeHeight);
		Monitor.bPrimary = Info.bIsPrimary;
	}

	return Monitors;
}

bool RageMonitor::IsSelectionSupported()
{
	return !GIsEditor && GetMonitors().Num() > 1;
}

FString RageMonitor::GetActiveMonitorId()
{
	const TSharedPtr<SWindow> Window = GetGameWindow();
	if (!Window.IsValid())
	{
		return FString();
	}
	
	const FVector2D WindowCentre = FVector2D(Window->GetPositionInScreen()) + FVector2D(Window->GetSizeInScreen()) * 0.5;

	for (const FRageMonitorInfo& Monitor : GetMonitors())
	{
		const FIntPoint Max = Monitor.Position + Monitor.DisplaySize;
		if (WindowCentre.X >= Monitor.Position.X && WindowCentre.X < Max.X
			&& WindowCentre.Y >= Monitor.Position.Y && WindowCentre.Y < Max.Y)
		{
			return Monitor.Id;
		}
	}

	return FString();
}

FString RageMonitor::ResolveMonitorId(const FString& PreferredId)
{
	const TArray<FRageMonitorInfo> Monitors = GetMonitors();
	if (Monitors.IsEmpty())
	{
		return FString();
	}

	if (!PreferredId.IsEmpty() && FindById(Monitors, PreferredId))
	{
		return PreferredId;
	}

	for (const FRageMonitorInfo& Monitor : Monitors)
	{
		if (Monitor.bPrimary)
		{
			return Monitor.Id;
		}
	}

	return Monitors[0].Id;
}

bool RageMonitor::MoveGameWindowToMonitor(const FString& MonitorId)
{
	const TSharedPtr<SWindow> Window = GetGameWindow();
	if (!Window.IsValid())
	{
		return false;
	}

	if (!FindById(GetMonitors(), MonitorId))
	{
		S_LOG(Warning, "Rage Settings: monitor {id} is no longer attached, leaving the window where it is.", MonitorId);
		return false;
	}

	if (GetActiveMonitorId() == MonitorId)
	{
		return false;
	}
	
	if (Window->GetWindowMode() == EWindowMode::Fullscreen)
	{
		Window->SetWindowMode(EWindowMode::WindowedFullscreen);
	}
	
	const TArray<FRageMonitorInfo> Monitors = GetMonitors();
	const FRageMonitorInfo* Target = FindById(Monitors, MonitorId);
	if (!Target)
	{
		return false;
	}

	const FVector2D TargetTopLeft(Target->Position.X, Target->Position.Y);
	
	Window->MoveWindowTo(TargetTopLeft);
	Window->ReshapeWindow(TargetTopLeft, FVector2D(Target->DisplaySize.X, Target->DisplaySize.Y));
	
	S_LOG(Log, "Rage Settings: moved the game window to monitor {id}; slate {slate}, client {client}, mode {mode}.",
		MonitorId,
		FString::Printf(TEXT("%gx%g @ %g,%g"),
			Window->GetSizeInScreen().X, Window->GetSizeInScreen().Y,
			Window->GetPositionInScreen().X, Window->GetPositionInScreen().Y),
		FString::Printf(TEXT("%gx%g"),
			Window->GetClientSizeInScreen().X, Window->GetClientSizeInScreen().Y),
		StaticCast<int32>(Window->GetWindowMode()));

	return true;
}
