// Copyright (c) 2026 Abdallah Boutrif
// SPDX-License-Identifier: MIT

#pragma once

#include "UObject/Object.h"
#include "RageRowOverrideObject.generated.h"

class URageRowBaseUserWidget;

UCLASS(EditInlineNew, DefaultToInstanced)
class URageRowOverrideObject : public UObject
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintNativeEvent)
	void ManipulateGeneratedWidget(URageRowBaseUserWidget* InWidget);
	virtual void ManipulateGeneratedWidget_Implementation(URageRowBaseUserWidget* InWidget) {};
};
