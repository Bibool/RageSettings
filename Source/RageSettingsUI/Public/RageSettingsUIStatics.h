// Copyright (c) 2026 Abdallah Boutrif
// SPDX-License-Identifier: MIT

#pragma once

#include "CoreMinimal.h"
#include "RageMacros.h"
#include "InputCoreTypes.h"
#include "RageQualityPreset.h"
#include "RageSettingsUIStatics.generated.h"

namespace RageSettingsUI
{
	template <typename TEnum>
	FText ResolveLocTextForEnum(TEnum Value)
	{
		if (const UEnum* EnumPtr = StaticEnum<TEnum>())
		{
			return RAGE_LOC_Str(EnumPtr->GetAuthoredNameStringByValue(static_cast<int64>(Value)));
		}
		
		return FText::GetEmpty();
	}
	
	template <typename TEnum>
	TArray<FString> BuildEnumOptionsStrings(const TArray<TEnum>& Values)
	{
		TArray<FString> Options;
		Options.Reserve(Values.Num());
		for (TEnum Value : Values)
		{
			Options.Add(ResolveLocTextForEnum(Value).ToString());
		}
		return Options;
	}
	
	template <typename TEnum>
	TArray<FText> BuildEnumOptionsTexts(const TArray<TEnum>& Values)
	{
		TArray<FText> Options;
		Options.Reserve(Values.Num());
		for (TEnum Value : Values)
		{
			Options.Add(ResolveLocTextForEnum(Value));
		}
		return Options;
	}
}

UCLASS()
class URageSettingsUIStatic : public UObject
{
	GENERATED_BODY()

public:
	UFUNCTION(Blueprintable, BlueprintPure, Category="Rage|UI")
	static FText GetLocTextForQualityPreset(int64 Value)
	{
		return RageSettingsUI::ResolveLocTextForEnum(StaticCast<ERageQualityPreset>(Value));
	}
};
