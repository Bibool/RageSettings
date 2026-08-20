// Copyright (c) 2026 Abdallah Boutrif
// SPDX-License-Identifier: MIT

#pragma once

#include "RageRowOverrideData.generated.h"

class URageRowOverrideObject;

USTRUCT(BlueprintType)
struct FRageRowOverrideData
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSubclassOf<UUserWidget> WidgetClass;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText DesiredLabel = FText::GetEmpty();

	/** What each option of an enum-backed row is called, keyed by the enum entry name ("French", not
	 * "ERageCultures::French"). An entry left out here still resolves through the string tables, so
	 * this is for the odd option that wants naming in place rather than for translating a whole enum. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TMap<FName, FText> OptionLabels;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Instanced)
	TObjectPtr<URageRowOverrideObject> OverrideObject = nullptr;
};
