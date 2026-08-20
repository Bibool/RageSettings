// Copyright (c) 2026 Abdallah Boutrif
// SPDX-License-Identifier: MIT

#pragma once

#include "Blueprint/UserWidget.h"
#include "InputCoreTypes.h"
#include "RageKeybindRow.generated.h"

class UTextBlock;
class UButton;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FRageKeybindRowChanged, FName, MappingName, FKey, NewKey);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FRageKeybindResetRequested, FName, MappingName);

UCLASS(Abstract, meta=(DisableNativeTick))
class RAGESETTINGSUI_API URageKeybindRow : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Rage|UI") 
	void Setup(FName InMappingName, const FText& NewLabel);
	
	UFUNCTION(BlueprintCallable, Category = "Rage|UI") 
	void SetCurrentKey(FKey NewKey);
	
	UFUNCTION(BlueprintPure, Category = "Rage|UI") 
	FName GetMappingName() const;
	
	UFUNCTION(BlueprintPure, Category = "Rage|UI") 
	bool IsListeningForInput() const;
	
	UFUNCTION(BlueprintImplementableEvent)
	void OnKeyTextSet(const FText& NewKeyText);
	
	UFUNCTION(BlueprintImplementableEvent)
	void OnLabelTextSet(const FText& NewLabelText);

	UPROPERTY(BlueprintAssignable, Category = "Rage|Delegates") 
	FRageKeybindRowChanged KeyRemappedDelegate;
	
	UPROPERTY(BlueprintAssignable, Category = "Rage|Delegates") 
	FRageKeybindResetRequested ResetToDefaultRequestedDelegate;

protected:
	virtual void NativeConstruct() override;
	virtual FReply NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent) override;
	virtual FReply NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	virtual void NativeOnFocusLost(const FFocusEvent& InFocusEvent) override;
	
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget)) 
	TObjectPtr<UTextBlock> Label = nullptr;
	
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget)) 
	TObjectPtr<UButton> RemapButton = nullptr;
	
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget)) 
	TObjectPtr<UTextBlock> KeyText = nullptr;
	
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional)) 
	TObjectPtr<UButton> ResetButton = nullptr;

private:
	UFUNCTION() 
	void HandleRemapButtonClicked();
	
	UFUNCTION() 
	void HandleResetButtonClicked();
	
	void BeginListening();
	void EndListening(bool bCancelled);
	void SetButtonsEnabled(bool bEnabled);
	void TryCommitKey(FKey NewKey);
	
	FName MappingName = NAME_None;
	FKey CurrentKey = FKey();
	bool bListeningForInput = false;
};
