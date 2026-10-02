// Copyright (c) 2026 Abdallah Boutrif
// SPDX-License-Identifier: MIT

#pragma once

#include "RageRowBaseUserWidget.h"
#include "RageComboRow.generated.h"

class UComboBoxString;

/** A pick-one-of-many row. The row owns the list and the selection, so a subclass can draw them with its
 * own dropdown and leave ComboBox out of the tree entirely; the hooks below are where it would redraw. */
UCLASS(Abstract, meta=(DisableNativeTick))
class RAGESETTINGSUI_API URageComboRow : public URageRowBaseUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Rage|UI")
	void SetOptions(const TArray<FString>& NewOptions);

	/** The same list, kept as text. A UComboBoxString can only show strings, so the text still has to
	 * be flattened to put it in the box, but holding on to it means the row can build those strings
	 * again in the language the player just switched to. */
	UFUNCTION(BlueprintCallable, Category = "Rage|UI")
	void SetOptionTexts(const TArray<FText>& NewOptions);

	UFUNCTION(BlueprintCallable, Category = "Rage|UI")
	void SetSelectedIndex(int32 NewIndex, bool bNotify = false);

	UFUNCTION(BlueprintPure, Category = "Rage|UI")
	int32 GetSelectedIndex() const;

	UFUNCTION(BlueprintPure, Category = "Rage|UI")
	int32 GetOptionCount() const;

	UFUNCTION(BlueprintPure, Category = "Rage|UI")
	FText GetOptionText(int32 Index) const;

protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

	virtual FString GetValueKey() const override;
	virtual FText GetValueText() const override;

	/* The list was replaced or retranslated. The selection is already settled when this runs. */
	virtual void NativeOnOptionsChanged();

	/* The selection moved, by the player or by code. */
	virtual void NativeOnSelectionChanged();

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UComboBoxString> ComboBox = nullptr;

private:
	UFUNCTION()
	void HandleSelectionChanged(FString SelectedItem, ESelectInfo::Type SelectionType);

	void HandleCultureChanged();

	void ApplyOptions(const TArray<FText>& NewOptions);

	void SyncComboBoxOptions();

	bool bSuppressNotify = false;

	TArray<FText> Options;

	/* False for a row whose options came in as plain strings, which is a row with nothing to retranslate. */
	bool bOptionsAreText = false;

	int32 SelectedIndex = INDEX_NONE;

	FDelegateHandle CultureChangedHandle;
};
