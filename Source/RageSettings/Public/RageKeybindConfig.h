// Copyright (c) 2026 Abdallah Boutrif
// SPDX-License-Identifier: MIT

#pragma once

#include "GameplayTagContainer.h"
#include "RageKeybindConfig.generated.h"

USTRUCT(BlueprintType)
struct FRageKeybindConfig
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rage|Input")
	FName MappingName = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rage|Input")
	FText DisplayName = FText::GetEmpty();
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rage|Input")
	FGameplayTag Category = FGameplayTag::EmptyTag;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rage|Input")
	int32 Priority = 0;
};
