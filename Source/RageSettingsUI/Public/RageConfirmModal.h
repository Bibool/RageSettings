// Copyright (c) 2026 Abdallah Boutrif
// SPDX-License-Identifier: MIT

#pragma once

#include "RageModalBase.h"
#include "RageConfirmModal.generated.h"

class UButton;

/**
 * Generic choice modal - the caller sets the message and reacts to the answer. Used for the keybind
 * conflict prompt; nothing about it is keybind-specific.
 *
 * Confirm and cancel are always there. The alternate is a third answer the caller can offer or withhold,
 * such as unbinding the other action instead of swapping with it. The buttons are optional so a subclass
 * can draw its own and answer through the Choose functions.
 */
UCLASS(Abstract, meta=(DisableNativeTick))
class RAGESETTINGSUI_API URageConfirmModal : public URageModalBase
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Rage|UI")
	void ChooseConfirm();

	UFUNCTION(BlueprintCallable, Category = "Rage|UI")
	void ChooseAlternate();

	UFUNCTION(BlueprintCallable, Category = "Rage|UI")
	void ChooseCancel();

	UFUNCTION(BlueprintCallable, Category = "Rage|UI")
	void SetAlternateChoiceOffered(bool bOffered);

	UFUNCTION(BlueprintPure, Category = "Rage|UI")
	bool OffersAlternateChoice() const;

	/* What the confirm and alternate choices say, for a caller whose choices mean different things each time. Empty keeps the authored text. */
	UFUNCTION(BlueprintCallable, Category = "Rage|UI")
	void SetChoiceLabels(const FText& NewConfirmLabel, const FText& NewAlternateLabel);

	UFUNCTION(BlueprintPure, Category = "Rage|UI")
	const FText& GetConfirmLabel() const;

	UFUNCTION(BlueprintPure, Category = "Rage|UI")
	const FText& GetAlternateLabel() const;

	UFUNCTION(BlueprintImplementableEvent, Category = "Rage|UI")
	void OnChoiceLabelsSet(const FText& NewConfirmLabel, const FText& NewAlternateLabel);

	UPROPERTY(BlueprintAssignable, Category = "Rage|Delegates")
	FRageModalChoiceMade ConfirmChosenDelegate;

	UPROPERTY(BlueprintAssignable, Category = "Rage|Delegates")
	FRageModalChoiceMade AlternateChosenDelegate;

	UPROPERTY(BlueprintAssignable, Category = "Rage|Delegates")
	FRageModalChoiceMade CancelChosenDelegate;

protected:
	virtual void NativeConstruct() override;

	/* For a subclass drawing its own alternate button. */
	virtual void NativeOnAlternateChoiceOfferedChanged(bool bOffered);

	virtual void NativeOnChoiceLabelsChanged();

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UButton> ConfirmButton = nullptr;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UButton> AlternateButton = nullptr;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UButton> CancelButton = nullptr;

private:
	UFUNCTION()
	void HandleConfirmClicked();

	UFUNCTION()
	void HandleAlternateClicked();

	UFUNCTION()
	void HandleCancelClicked();

	bool bAlternateChoiceOffered = false;

	FText ConfirmLabel = FText::GetEmpty();
	FText AlternateLabel = FText::GetEmpty();
};
