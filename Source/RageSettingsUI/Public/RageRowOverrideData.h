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
	
	/* The short line under the label. Without one, a string table entry keyed <Property>_Hint is used. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(EditCondition="bCreateWidget", EditConditionHides))
	FText DesiredHint = FText::GetEmpty();

	/* What a details pane says about the row. Without one, a string table entry keyed <Property>_Description is used. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(EditCondition="bCreateWidget", EditConditionHides, MultiLine="true"))
	FText DesiredDescription = FText::GetEmpty();
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(EditCondition="bCreateWidget", EditConditionHides))
	TMap<FName, FText> OptionLabels;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Instanced, meta=(EditCondition="bCreateWidget", EditConditionHides))
	TObjectPtr<URageRowOverrideObject> OverrideObject = nullptr;
};
