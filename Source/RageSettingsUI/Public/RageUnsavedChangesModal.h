// Copyright (c) 2026 Abdallah Boutrif
// SPDX-License-Identifier: MIT

#pragma once

#include "CoreMinimal.h"
#include "RageModalBase.h"
#include "RageUnsavedChangesModal.generated.h"

class UButton;

UCLASS(Abstract, meta=(DisableNativeTick))
class RAGESETTINGSUI_API URageUnsavedChangesModal : public URageModalBase
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintAssignable, Category = "Rage|Delegates") 
	FRageModalChoiceMade ApplyAndCloseChosenDelegate;
	
	UPROPERTY(BlueprintAssignable, Category = "Rage|Delegates") 
	FRageModalChoiceMade DiscardAndCloseChosenDelegate;
	
	UPROPERTY(BlueprintAssignable, Category = "Rage|Delegates") 
	FRageModalChoiceMade CancelChosenDelegate;

protected:
	virtual void NativeConstruct() override;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget)) 
	TObjectPtr<UButton> ApplyAndCloseButton = nullptr;
	
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget)) 
	TObjectPtr<UButton> DiscardAndCloseButton = nullptr;
	
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget)) 
	TObjectPtr<UButton> CancelButton = nullptr;

private:
	UFUNCTION() 
	void HandleApplyAndCloseClicked();
	
	UFUNCTION() 
	void HandleDiscardAndCloseClicked();
	
	UFUNCTION() 
	void HandleCancelClicked();
};
