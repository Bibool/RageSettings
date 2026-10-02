// Copyright (c) 2026 Abdallah Boutrif
// SPDX-License-Identifier: MIT

#pragma once

#include "Blueprint/UserWidget.h"
#include "RageModalBase.generated.h"

class UTextBlock;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FRageModalChoiceMade);

/* One line of a modal's change list: a setting going from one value to another. */
USTRUCT(BlueprintType)
struct RAGESETTINGSUI_API FRageModalChange
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rage|UI")
	FText Label = FText::GetEmpty();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rage|UI")
	FText From = FText::GetEmpty();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rage|UI")
	FText To = FText::GetEmpty();

	/* A change the player did not ask for but would get anyway, like the other binding in a swap. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rage|UI")
	bool bWarning = false;
};

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
	void SetTitle(const FText& NewTitle);

	UFUNCTION(BlueprintCallable, Category = "Rage|UI")
	void SetMessage(const FText& NewMessage);

	/* What the choice would change. An empty list is a modal with nothing to list. */
	UFUNCTION(BlueprintCallable, Category = "Rage|UI")
	void SetChanges(const TArray<FRageModalChange>& NewChanges);

	UFUNCTION(BlueprintPure, Category = "Rage|UI")
	const FText& GetTitle() const;

	UFUNCTION(BlueprintPure, Category = "Rage|UI")
	const FText& GetMessage() const;

	UFUNCTION(BlueprintPure, Category = "Rage|UI")
	const TArray<FRageModalChange>& GetChanges() const;

	UFUNCTION(BlueprintImplementableEvent, Category = "Rage|UI")
	void OnTitleSet(const FText& NewTitle);

	UFUNCTION(BlueprintImplementableEvent, Category = "Rage|UI")
	void OnMessageSet(const FText& NewMessage);

	UFUNCTION(BlueprintImplementableEvent, Category = "Rage|UI")
	void OnChangesSet(const TArray<FRageModalChange>& NewChanges);

protected:
	virtual void NativeConstruct() override;

	/* The title, the message or the change list changed. */
	virtual void NativeOnContentChanged();

	virtual void NativeOnOpenStateChanged(bool bIsModalOpen);

	UFUNCTION(BlueprintImplementableEvent, Category = "Rage|UI")
	void OnModalOpenStateChanged(bool bIsModalOpen);

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> TitleText = nullptr;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> MessageText = nullptr;

private:
	bool bIsOpen = false;

	FText Title = FText::GetEmpty();
	FText Message = FText::GetEmpty();
	TArray<FRageModalChange> Changes;
};
