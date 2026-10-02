// Copyright (c) 2026 Abdallah Boutrif
// SPDX-License-Identifier: MIT

#pragma once

#include "RageRowBaseUserWidget.h"
#include "InputCoreTypes.h"
#include "RageKeybindRow.generated.h"

class UTextBlock;
class UButton;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FRageKeybindRowChanged, FName, MappingName, FKey, NewKey);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FRageKeybindResetRequested, FName, MappingName);

UCLASS(Abstract, meta=(DisableNativeTick))
class RAGESETTINGSUI_API URageKeybindRow : public URageRowBaseUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Rage|UI")
	void Setup(FName InMappingName, const FText& NewLabel);

	UFUNCTION(BlueprintCallable, Category = "Rage|UI")
	void SetCurrentKey(FKey NewKey);

	UFUNCTION(BlueprintPure, Category = "Rage|UI")
	FKey GetCurrentKey() const;

	UFUNCTION(BlueprintPure, Category = "Rage|UI")
	FName GetMappingName() const;

	UFUNCTION(BlueprintPure, Category = "Rage|UI")
	bool IsListeningForInput() const;

	/* Starts waiting for the next key, the same as clicking the key button. */
	UFUNCTION(BlueprintCallable, Category = "Rage|UI")
	void BeginListening();

	/* Asks for the default key back, the same as clicking the reset button. */
	UFUNCTION(BlueprintCallable, Category = "Rage|UI")
	void RequestResetToDefault();

	UFUNCTION(BlueprintImplementableEvent)
	void OnKeyTextSet(const FText& NewKeyText);

	UPROPERTY(BlueprintAssignable, Category = "Rage|Delegates")
	FRageKeybindRowChanged KeyRemappedDelegate;

	UPROPERTY(BlueprintAssignable, Category = "Rage|Delegates")
	FRageKeybindResetRequested ResetToDefaultRequestedDelegate;

protected:
	virtual void NativeConstruct() override;
	virtual FReply NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent) override;
	virtual FReply NativeOnPreviewMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	virtual void NativeOnFocusLost(const FFocusEvent& InFocusEvent) override;

	virtual FString GetValueKey() const override;
	virtual FText GetValueText() const override;

	/* Listening started or stopped, for a subclass to swap the key cap for a prompt. */
	virtual void NativeOnListeningChanged(bool bListening);

	/* The key cap's text: the bound key's name, or the unbound or listening prompt. */
	virtual void NativeOnKeyTextChanged(const FText& NewKeyText);

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UButton> RemapButton = nullptr;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> KeyText = nullptr;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UButton> ResetButton = nullptr;

private:
	UFUNCTION()
	void HandleRemapButtonClicked();

	UFUNCTION()
	void HandleResetButtonClicked();

	void EndListening(bool bCancelled);
	void SetButtonsEnabled(bool bEnabled);
	void TryCommitKey(FKey NewKey);
	void ShowKeyText(const FText& NewKeyText);

	FName MappingName = NAME_None;
	FKey CurrentKey = FKey();
	bool bListeningForInput = false;
};
