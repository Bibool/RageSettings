// Copyright (c) 2026 Abdallah Boutrif
// SPDX-License-Identifier: MIT

#pragma once

#include "RageRowBaseUserWidget.h"
#include "RageSliderDisplayFormat.h"
#include "RageSliderRow.generated.h"

class USlider;

UCLASS(Abstract, meta=(DisableNativeTick))
class RAGESETTINGSUI_API URageSliderRow : public URageRowBaseUserWidget
{
	GENERATED_BODY()
	
public:
	UFUNCTION(BlueprintCallable, Category = "Rage|UI")
	void SetRange(float NewMin, float NewMax);
	
	UFUNCTION(BlueprintCallable, Category = "Rage|UI")
	void SetDisplayFormat(ERageSliderDisplayFormat NewFormat);
	
	UFUNCTION(BlueprintCallable, Category = "Rage|UI")
	void SetValue(float NewValue, bool bNotify = false);
	
	UFUNCTION(BlueprintPure, Category = "Rage|UI") 
	float GetValue() const;

protected:
	UFUNCTION(BlueprintNativeEvent)
	void RefreshValueText(float Value);
	
	virtual void NativeConstruct() override;	

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget)) 
	TObjectPtr<USlider> Slider = nullptr;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional)) 
	TObjectPtr<UTextBlock> ValueText = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rage|UI")
	ERageSliderDisplayFormat DisplayFormat = ERageSliderDisplayFormat::Raw;
	
private:
	UFUNCTION() 
	void HandleSliderValueChanged(float NewValue);
	
	void RefreshValueText_Implementation(float Value);
	
	bool bSuppressNotify = false;
};
