// Copyright (c) 2026 Abdallah Boutrif
// SPDX-License-Identifier: MIT

#pragma once

#include "RageRowBaseUserWidget.h"
#include "RageToggleRow.generated.h"

class UCheckBox;

UCLASS(Abstract, meta=(DisableNativeTick))
class RAGESETTINGSUI_API URageToggleRow : public URageRowBaseUserWidget
{
	GENERATED_BODY()
	
public:
	UFUNCTION(BlueprintCallable, Category = "Rage|UI") 
	void SetValue(bool bChecked, bool bNotify = false);
	
	UFUNCTION(BlueprintPure, Category = "Rage|UI") 
	bool GetValue() const;

protected:
	virtual void NativeConstruct() override;

	/** Called whenever the check state changes, from SetValue as well as from a click. UCheckBox only
	 * broadcasts its own delegate for a click, so this is the one place a subclass sees both. */
	virtual void NativeOnToggleChanged(bool bChecked);

	virtual FString GetValueKey() const override;
	virtual FText GetValueText() const override;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UCheckBox> CheckBox = nullptr;
	
private:
	UFUNCTION() 
	void HandleCheckStateChanged(bool bIsChecked);
	
	bool bSuppressNotify = false;
};
