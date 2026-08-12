// Copyright (c) 2026 Abdallah Boutrif
// SPDX-License-Identifier: MIT

#pragma once

#include "CoreMinimal.h"
#include "RageMonitorInfo.generated.h"

USTRUCT(BlueprintType)
struct FRageMonitorInfo
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Rage|Video")
	FString Id = TEXT("");
	
	UPROPERTY(BlueprintReadOnly, Category = "Rage|Video")
	FString Name = TEXT("");

	UPROPERTY(BlueprintReadOnly, Category = "Rage|Video")
	FIntPoint Position = FIntPoint::ZeroValue;
	
	UPROPERTY(BlueprintReadOnly, Category = "Rage|Video")
	FIntPoint DisplaySize = FIntPoint::ZeroValue;

	UPROPERTY(BlueprintReadOnly, Category = "Rage|Video")
	FIntPoint NativeResolution = FIntPoint::ZeroValue;

	UPROPERTY(BlueprintReadOnly, Category = "Rage|Video")
	bool bPrimary = false;
};
