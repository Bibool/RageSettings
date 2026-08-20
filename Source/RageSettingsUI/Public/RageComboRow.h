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

	/** The same list, kept as text. A UComboBoxString can only show strings, so the text still has to
	 * be flattened to put it in the box, but holding on to it means the row can build those strings
	 * again in the language the player just switched to. */
	UFUNCTION(BlueprintCallable, Category = "Rage|UI")
	void SetOptionTexts(const TArray<FText>& NewOptions);

	UFUNCTION(BlueprintCallable, Category = "Rage|UI")
	void SetSelectedIndex(int32 NewIndex, bool bNotify = false);

	UFUNCTION(BlueprintPure, Category = "Rage|UI")
	int32 GetSelectedIndex() const;

protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UComboBoxString> ComboBox = nullptr;

private:
	UFUNCTION()
	void HandleSelectionChanged(FString SelectedItem, ESelectInfo::Type SelectionType);

	void HandleCultureChanged();

	void RefreshOptionStrings();

	void ApplyOptionStrings(const TArray<FString>& NewOptions);

	bool bSuppressNotify = false;

	/* Empty for a row whose options came in as plain strings, which is a row with nothing to retranslate. */
	TArray<FText> OptionTexts;

	FDelegateHandle CultureChangedHandle;
};