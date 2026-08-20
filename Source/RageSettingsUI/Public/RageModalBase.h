// Copyright (c) 2026 Abdallah Boutrif
// SPDX-License-Identifier: MIT

#pragma once

#include "Blueprint/UserWidget.h"
#include "RageModalBase.generated.h"

class UTextBlock;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FRageModalChoiceMade);

UCLASS(Abstract, meta=(DisableNativeTick))
class RAGESETTINGSUI_API URageModalBase : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Rage|UI")
	void Open();

	UFUNCTION(BlueprintCallable, Category = "Rage|UI")
	void Close();

	UFUNCTION(BlueprintPure, Category = "Rage|UI")
	bool IsOpen() const;

	UFUNCTION(BlueprintCallable, Category = "Rage|UI")
	void SetMessage(const FText& NewMessage);
	
	UFUNCTION(BlueprintImplementableEvent, Category = "Rage|UI")
	void OnMessageSet(const FText& NewMessage);

protected:
	virtual void NativeConstruct() override;

	UFUNCTION(BlueprintImplementableEvent, Category = "Rage|UI")
	void OnModalOpenStateChanged(bool bIsModalOpen);

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> MessageText = nullptr;

private:
	bool bIsOpen = false;

};
