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

	UFUNCTION(BlueprintPure, Category = "Rage|UI")
	float GetMinValue() const;

	UFUNCTION(BlueprintPure, Category = "Rage|UI")
	float GetMaxValue() const;

	/* A value written the way this row writes its readout, so a range label matches the number beside it. */
	UFUNCTION(BlueprintPure, Category = "Rage|UI")
	FText FormatValue(float Value) const;

protected:
	UFUNCTION(BlueprintNativeEvent)
	void RefreshValueText(float Value);
	virtual void RefreshValueText_Implementation(float Value);

	virtual void NativeConstruct() override;

	virtual FString GetValueKey() const override;
	virtual FText GetValueText() const override;

	/* The range or the display format changed, for a subclass showing either. */
	virtual void NativeOnRangeChanged();

	/* The text the readout shows for Value. */
	virtual FText NativeFormatValue(float Value) const;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<USlider> Slider = nullptr;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> ValueText = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rage|UI")
	ERageSliderDisplayFormat DisplayFormat = ERageSliderDisplayFormat::Raw;

private:
	UFUNCTION()
	void HandleSliderValueChanged(float NewValue);

	bool bSuppressNotify = false;
};
