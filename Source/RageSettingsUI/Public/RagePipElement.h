// Copyright (c) 2026 Abdallah Boutrif
// SPDX-License-Identifier: MIT

#pragma once

#include "Blueprint/UserWidget.h"
#include "RagePipElement.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FRagePipClicked, int32, PipIndex);

/* How one pip reads under the strip's current selection. Current implies filled in both pip
 * styles - it is called out separately so a Blueprint can mark the selected pip differently from
 * the ones merely behind it. */
UENUM(BlueprintType)
enum class ERagePipState : uint8
{
	/* At or past the selection. */
	Empty,
	/* Behind the selection in a Cumulative strip. A Progressive strip never produces this. */
	Filled,
	/* The selected option. */
	Current
};

/**
 * One pip in a URagePipView strip. Carries no visuals of its own - it is told where it sits and
 * what state it is in, and the Blueprint decides what that looks like in OnPipStateChanged.
 */
UCLASS(Abstract, meta=(DisableNativeTick))
class RAGESETTINGSUI_API URagePipElement : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Rage|UI")
	void Setup(int32 InPipIndex);

	UFUNCTION(BlueprintCallable, Category = "Rage|UI")
	void SetPipState(ERagePipState NewState);

	UFUNCTION(BlueprintPure, Category = "Rage|UI")
	ERagePipState GetPipState() const;

	/** Position in the strip, left to right. */
	UFUNCTION(BlueprintPure, Category = "Rage|UI")
	int32 GetPipIndex() const;

	UFUNCTION(BlueprintPure, Category = "Rage|UI")
	bool IsFilled() const;
	
	UFUNCTION(BlueprintImplementableEvent, Category = "Rage|UI")
	void OnSlotAssigned(UPanelSlot* BaseSlot);
	
	UPROPERTY(Blueprintable, BlueprintCallable, BlueprintAssignable)
	FRagePipClicked PipClickedDelegate;

protected:
	virtual void NativeConstruct() override;
	
	UFUNCTION(BlueprintImplementableEvent, Category = "Rage|UI")
	void OnPipStateChanged(ERagePipState NewState);

private:
	int32 PipIndex = INDEX_NONE;

	ERagePipState PipState = ERagePipState::Empty;

};
