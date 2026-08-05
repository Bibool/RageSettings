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
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Instanced)
	TObjectPtr<URageRowOverrideObject> OverrideObject = nullptr;
};
