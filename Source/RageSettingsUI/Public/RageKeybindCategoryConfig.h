// Copyright (c) 2026 Abdallah Boutrif
// SPDX-License-Identifier: MIT

#pragma once

#include "Templates/SubclassOf.h"
#include "RageKeybindCategoryConfig.generated.h"

class URageRowBaseUserWidget;

USTRUCT(BlueprintType)
struct FRageKeybindCategoryConfig
{
	GENERATED_BODY()
	
	/* Priority of this category, higher priority categories will be displayed first. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rage|Input")
	int32 Priority = 0;
	
	/* The text shown for this category can be localized. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rage|Input")
	FText CategoryDisplayName = FText::GetEmpty();
	
	/* If unset, will use URageSettingsUIDeveloperSettings::DefaultCategoryWidgetClass. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rage|Input")
	TOptional<TSubclassOf<URageRowBaseUserWidget>> OptionalCategoryWidget;
};
