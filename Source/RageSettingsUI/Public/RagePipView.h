// Copyright (c) 2026 Abdallah Boutrif
// SPDX-License-Identifier: MIT

#pragma once

#include "RagePipElement.h"
#include "Blueprint/UserWidget.h"
#include "RagePipView.generated.h"

class UPanelWidget;

UENUM(BlueprintType)
enum class ERagePipStyle : uint8
{
	Cumulative,
	Progressive
};

UCLASS(Abstract, meta=(DisableNativeTick))
class RAGESETTINGSUI_API URagePipView : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Rage|UI")
	void SetPipCount(int32 NewPipCount);

	UFUNCTION(BlueprintCallable, Category = "Rage|UI")
	void SetSelectedIndex(int32 NewIndex);

	UFUNCTION(BlueprintCallable, Category = "Rage|UI")
	void SetPipStyle(ERagePipStyle NewStyle);

	UFUNCTION(BlueprintPure, Category = "Rage|UI")
	int32 GetPipCount() const;

	UFUNCTION(BlueprintPure, Category = "Rage|UI")
	int32 GetSelectedIndex() const;

	UFUNCTION(BlueprintPure, Category = "Rage|UI")
	ERagePipState ResolvePipState(int32 PipIndex) const;
	
	UPROPERTY(Blueprintable, BlueprintCallable, BlueprintAssignable)
	FRagePipClicked PipClickedDelegate;

protected:
	UFUNCTION()
	void HandlePipClicked(int32 PipIndex);
	
	UPROPERTY(EditDefaultsOnly, NoClear, Category = "Rage|UI")
	TSubclassOf<URagePipElement> PipElementClass = nullptr;

	UPROPERTY(EditDefaultsOnly, Category = "Rage|UI")
	ERagePipStyle PipStyle = ERagePipStyle::Cumulative;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UPanelWidget> PipContainer = nullptr;

private:
	void RefreshPipStates() const;

	UPROPERTY(Transient)
	TArray<TObjectPtr<URagePipElement>> Pips;

	int32 SelectedIndex = INDEX_NONE;

};
