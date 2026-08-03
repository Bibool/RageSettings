// Copyright (c) 2026 Abdallah Boutrif
// SPDX-License-Identifier: MIT

#pragma once

#include "RageModalBase.h"
#include "RageConfirmModal.generated.h"

class UButton;

/**
 * Generic two-choice modal - the caller sets the message and reacts to the answer. Used for the
 * keybind conflict prompt; nothing about it is keybind-specific.
 */
UCLASS(Abstract, meta=(DisableNativeTick))
class RAGESETTINGSUI_API URageConfirmModal : public URageModalBase
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintAssignable, Category = "Rage|Delegates")
	FRageModalChoiceMade ConfirmChosenDelegate;

	UPROPERTY(BlueprintAssignable, Category = "Rage|Delegates")
	FRageModalChoiceMade CancelChosenDelegate;

protected:
	virtual void NativeConstruct() override;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UButton> ConfirmButton = nullptr;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UButton> CancelButton = nullptr;

private:
	UFUNCTION()
	void HandleConfirmClicked();

	UFUNCTION()
	void HandleCancelClicked();

};
