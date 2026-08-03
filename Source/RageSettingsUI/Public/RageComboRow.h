// Copyright (c) 2026 Abdallah Boutrif
// SPDX-License-Identifier: MIT

#pragma once

#include "RageRowBaseUserWidget.h"
#include "RageComboRow.generated.h"

class UComboBoxString;

UCLASS(Abstract, meta=(DisableNativeTick))
class RAGESETTINGSUI_API URageComboRow : public URageRowBaseUserWidget
{
	GENERATED_BODY()
	
public:
	UFUNCTION(BlueprintCallable, Category = "Rage|UI") 
	void SetOptions(const TArray<FString>& NewOptions);
	
	UFUNCTION(BlueprintCallable, Category = "Rage|UI") 
	void SetSelectedIndex(int32 NewIndex, bool bNotify = false);
	
	UFUNCTION(BlueprintPure, Category = "Rage|UI")
	int32 GetSelectedIndex() const;

protected:
	virtual void NativeConstruct() override;
	
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget)) 
	TObjectPtr<UComboBoxString> ComboBox = nullptr;

private:
	UFUNCTION() 
	void HandleSelectionChanged(FString SelectedItem, ESelectInfo::Type SelectionType);
	
	bool bSuppressNotify = false;
};