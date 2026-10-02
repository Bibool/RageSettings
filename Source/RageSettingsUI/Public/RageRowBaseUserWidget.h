// Copyright (c) 2026 Abdallah Boutrif
// SPDX-License-Identifier: MIT

#pragma once

#include "RageVariant.h"
#include "Blueprint/UserWidget.h"
#include "RageRowBaseUserWidget.generated.h"

class UTextBlock;
class URageRowBaseUserWidget;

DECLARE_MULTICAST_DELEGATE_TwoParams(FRageValueChanged, FName, FRageVariant);
DECLARE_MULTICAST_DELEGATE_OneParam(FRageRowNotify, URageRowBaseUserWidget*);

#define VAL(Type, Value) FRageVariant(TInPlaceType<Type>(), Value)

/**
 * Base of every settings row, and of the headers between them.
 *
 * Besides the label it carries two optional texts: a hint, the one line a row shows under its label, and a
 * description, the longer explanation a details pane shows for the row the player is on.
 *
 * A row also remembers the value it was showing the last time its category had nothing pending, its
 * baseline, and reports whether it has moved off it. That is what lets a view mark the exact rows a player
 * changed and list them before leaving, which the category's single dirty flag cannot. A row holding no
 * setting, such as a section header, has no value and is never modified.
 */
UCLASS(Abstract, meta=(DisableNativeTick))
class RAGESETTINGSUI_API URageRowBaseUserWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Rage|UI")
	void SetLabel(const FText& NewLabel);

	UFUNCTION(BlueprintPure, Category = "Rage|UI")
	const FText& GetRowLabel() const;

	UFUNCTION(BlueprintCallable, Category = "Rage|UI")
	void SetHint(const FText& NewHint);

	UFUNCTION(BlueprintPure, Category = "Rage|UI")
	const FText& GetHintText() const;

	UFUNCTION(BlueprintCallable, Category = "Rage|UI")
	void SetDescription(const FText& NewDescription);

	UFUNCTION(BlueprintPure, Category = "Rage|UI")
	const FText& GetDescriptionText() const;

	UFUNCTION(BlueprintCallable, Category = "Rage|UI")
	void SetRowId(FName NewRowId);

	UFUNCTION(BlueprintPure, Category = "Rage|UI")
	FName GetRowId() const;

	UFUNCTION(BlueprintCallable, Category = "Rage|UI")
	void SetRowEnabled(bool bEnabled, const FText& InDisabledReason);

	UFUNCTION(BlueprintPure, Category = "Rage|UI")
	const FText& GetDisabledReason() const { return DisabledReason; }

	/* Takes the value the row shows right now as the one it started from. */
	UFUNCTION(BlueprintCallable, Category = "Rage|UI")
	void CaptureBaseline();

	/* Whether the value differs from the baseline. Always false before a baseline is taken. */
	UFUNCTION(BlueprintPure, Category = "Rage|UI")
	bool IsModified() const;

	/* Whether this row holds a setting at all, as opposed to being a header between them. */
	UFUNCTION(BlueprintPure, Category = "Rage|UI")
	bool IsSettingRow() const;

	/* The current value as the player reads it: ON, 95%, HIGH, SPACE. */
	UFUNCTION(BlueprintPure, Category = "Rage|UI")
	FText GetValueDisplayText() const;

	/* The baseline value as the player read it when it was taken. */
	UFUNCTION(BlueprintPure, Category = "Rage|UI")
	FText GetBaselineDisplayText() const;

	UFUNCTION(BlueprintPure, Category = "Rage|UI")
	bool IsRowHovered() const;

	/* Whether keyboard focus is on the row or anything inside it. */
	UFUNCTION(BlueprintPure, Category = "Rage|UI")
	bool IsRowFocused() const;

	UFUNCTION(BlueprintImplementableEvent)
	void OnValueTextSet(const FText& NewValue);

	UFUNCTION(BlueprintImplementableEvent)
	void OnLabelTextSet(const FText& NewValue);

	UFUNCTION(BlueprintImplementableEvent)
	void OnModifiedChanged(bool bModified);

	FRageValueChanged ValueChangedDelegate;

	/* Fires when the row goes on or off its baseline. */
	FRageRowNotify ModifiedChangedDelegate;

	/* Fires when the pointer or the keyboard focus arrives on a setting row, for a details pane to follow. */
	FRageRowNotify HighlightedDelegate;

	/** Fires whenever anything a row's frame would draw from changes: its texts, whether it is enabled or
	 * modified, and whether it is hovered or focused. Lets one widget dress any kind of row. */
	FRageRowNotify PresentationChangedDelegate;

protected:
	virtual void NativePreConstruct() override;
	virtual void NativeOnMouseEnter(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	virtual void NativeOnMouseLeave(const FPointerEvent& InMouseEvent) override;
	virtual void NativeOnAddedToFocusPath(const FFocusEvent& InFocusEvent) override;
	virtual void NativeOnRemovedFromFocusPath(const FFocusEvent& InFocusEvent) override;

	/* Called after SetRowEnabled has applied, so a native subclass can redraw its own disabled look. */
	virtual void NativeOnRowEnabledChanged(bool bEnabled);

	/* Called when the label, hint, description or disabled reason changes. */
	virtual void NativeOnTextsChanged();

	virtual void NativeOnModifiedChanged(bool bNewModified);

	/** The value in a form that compares equal for equal values, used against the baseline. Empty for a row
	 * holding no setting, which is how headers opt out of being modified or highlighted. */
	virtual FString GetValueKey() const;

	/* The value as the player reads it. */
	virtual FText GetValueText() const;

	/* Rows call this whenever their value changes, whether code set it or the player did. */
	void RefreshModified();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rage|UI")
	FName RowId = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rage|UI")
	FText LabelText = FText::GetEmpty();

	/* The short line under the label. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rage|UI")
	FText HintText = FText::GetEmpty();

	/* The longer explanation a details pane shows while this row is highlighted. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rage|UI", meta = (MultiLine = "true"))
	FText DescriptionText = FText::GetEmpty();

	UPROPERTY(EditDefaultsOnly, Category = "Rage|UI")
	FText DisabledReasonFormat = FText::FromString(TEXT("{0} ({1})"));

private:
	void RefreshLabel();

	UPROPERTY(Transient, BlueprintReadOnly, meta=(BindWidgetOptional, AllowPrivateAccess="true"))
	TObjectPtr<UTextBlock> Label = nullptr;

	FText DisabledReason = FText::GetEmpty();

	FString BaselineKey;
	FText BaselineText = FText::GetEmpty();
	bool bHasBaseline = false;
	bool bIsModified = false;
	bool bRowHovered = false;
	bool bRowFocused = false;
};
