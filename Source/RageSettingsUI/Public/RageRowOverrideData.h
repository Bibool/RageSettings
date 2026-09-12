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
	bool bCreateWidget = true;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(EditCondition="bCreateWidget", EditConditionHides))
	TSubclassOf<UUserWidget> WidgetClass = nullptr;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(EditCondition="bCreateWidget", EditConditionHides))
	FText DesiredLabel = FText::GetEmpty();
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(EditCondition="bCreateWidget", EditConditionHides))
	TMap<FName, FText> OptionLabels;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Instanced, meta=(EditCondition="bCreateWidget", EditConditionHides))
	TObjectPtr<URageRowOverrideObject> OverrideObject = nullptr;
};
