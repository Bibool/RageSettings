// Copyright (c) 2026 Abdallah Boutrif
// SPDX-License-Identifier: MIT

#pragma once

#include "CoreMinimal.h"
#include "RageModalBase.h"
#include "RageUnsavedChangesModal.generated.h"

class UButton;

/* Asked when the player leaves with changes pending. The buttons are optional so a subclass can draw its own and answer through the Choose functions. */
UCLASS(Abstract, meta=(DisableNativeTick))
class RAGESETTINGSUI_API URageUnsavedChangesModal : public URageModalBase
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Rage|UI")
	void ChooseApplyAndClose();

	UFUNCTION(BlueprintCallable, Category = "Rage|UI")
	void ChooseDiscardAndClose();

	UFUNCTION(BlueprintCallable, Category = "Rage|UI")
	void ChooseCancel();

	UPROPERTY(BlueprintAssignable, Category = "Rage|Delegates")
	FRageModalChoiceMade ApplyAndCloseChosenDelegate;

	UPROPERTY(BlueprintAssignable, Category = "Rage|Delegates")
	FRageModalChoiceMade DiscardAndCloseChosenDelegate;

	UPROPERTY(BlueprintAssignable, Category = "Rage|Delegates")
	FRageModalChoiceMade CancelChosenDelegate;

protected:
	virtual void NativeConstruct() override;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UButton> ApplyAndCloseButton = nullptr;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UButton> DiscardAndCloseButton = nullptr;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UButton> CancelButton = nullptr;

private:
	UFUNCTION()
	void HandleApplyAndCloseClicked();

	UFUNCTION()
	void HandleDiscardAndCloseClicked();

	UFUNCTION()
	void HandleCancelClicked();
};
