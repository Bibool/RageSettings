// Copyright (c) 2026 Abdallah Boutrif
// SPDX-License-Identifier: MIT

#pragma once

#include "RageVariant.h"
#include "Blueprint/UserWidget.h"
#include "RageRowBaseUserWidget.generated.h"

class UTextBlock;

DECLARE_MULTICAST_DELEGATE_TwoParams(FRageValueChanged, FName, FRageVariant);

#define VAL(Type, Value) FRageVariant(TInPlaceType<Type>(), Value)

UCLASS(Abstract, meta=(DisableNativeTick))
class RAGESETTINGSUI_API URageRowBaseUserWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	UFUNCTION(BlueprintCallable, Category = "Rage|UI") 
	void SetLabel(const FText& NewLabel);
	
	UFUNCTION(BlueprintCallable, Category = "Rage|UI")
	void SetRowId(FName NewRowId);
	
	UFUNCTION(BlueprintPure, Category = "Rage|UI") 
	FName GetRowId() const;
	
	FRageValueChanged ValueChangedDelegate;

protected:
	virtual void NativePreConstruct() override;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rage|UI")
	FName RowId = NAME_None;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rage|UI")
	FText LabelText = FText::GetEmpty();
	
private:
	UPROPERTY(Transient, BlueprintReadOnly, meta=(BindWidget, AllowPrivateAccess="true"))
	TObjectPtr<UTextBlock> Label = nullptr;

};
