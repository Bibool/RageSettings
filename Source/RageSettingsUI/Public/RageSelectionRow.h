// Copyright (c) 2026 Abdallah Boutrif
// SPDX-License-Identifier: MIT

#pragma once

#include "CoreMinimal.h"
#include "RagePipView.h"
#include "RageRowBaseUserWidget.h"
#include "RageSelectionRow.generated.h"

class UButton;

UCLASS(Abstract, meta=(DisableNativeTick))
class RAGESETTINGSUI_API URageSelectionRow : public URageRowBaseUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Rage|UI")
	void SetOptions(const TArray<FText>& NewOptions);

	UFUNCTION(BlueprintCallable, Category = "Rage|UI")
	void SetSelectedIndex(int32 NewIndex, bool bNotify = false);

	UFUNCTION(BlueprintPure, Category = "Rage|UI")
	int32 GetSelectedIndex() const;

	UFUNCTION(BlueprintPure, Category = "Rage|UI")
	int32 GetOptionCount() const;

	UPROPERTY(EditDefaultsOnly, Category="Rage|UI")
	bool bWrapAround = false;
	
	UPROPERTY(EditDefaultsOnly, Category="Rage|UI")
	TOptional<ERagePipStyle> PipStyleOverride;

protected:
	virtual void NativeOnInitialized() override;
	virtual void NativeConstruct() override;
	
	UFUNCTION(BlueprintNativeEvent, Category = "Rage|UI")
	void RefreshSelection();
	virtual void RefreshSelection_Implementation();

	virtual FString GetValueKey() const override;
	virtual FText GetValueText() const override;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> ValueText = nullptr;

	/* For a subclass drawing the value with something other than ValueText. */
	const TArray<FText>& GetOptions() const;

	/** +1 or -1 while RefreshSelection runs for a step through the arrows, 0 when the index was set directly
	 * or by a pip. Wrapping makes the direction impossible to recover from the two indices alone. */
	int8 GetStepDirection() const;

	UButton* GetLeftButton() const;
	UButton* GetRightButton() const;

private:
	UFUNCTION()
	void HandleLeftClicked();

	UFUNCTION()
	void HandleRightClicked();
	
	UFUNCTION()
	void HandlePipClicked(int32 PipIndex);

	void CycleSelection(int8 Direction);

	UPROPERTY(Transient, BlueprintReadOnly, meta=(BindWidget, AllowPrivateAccess="true"))
	TObjectPtr<UButton> Left_Button = nullptr;

	UPROPERTY(Transient, BlueprintReadOnly, meta=(BindWidget, AllowPrivateAccess="true"))
	TObjectPtr<UButton> Right_Button = nullptr;

	UPROPERTY(Transient, BlueprintReadOnly, meta=(BindWidget, AllowPrivateAccess="true"))
	TObjectPtr<URagePipView> PipView = nullptr;

	TArray<FText> Options;

	int32 SelectedIndex = INDEX_NONE;

	int8 StepDirection = 0;

};
