// Copyright (c) 2026 Abdallah Boutrif
// SPDX-License-Identifier: MIT

#include "RageKeybindRow.h"

#include "RageMacros.h"
#include "Components/TextBlock.h"
#include "Components/Button.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(RageKeybindRow)

void URageKeybindRow::Setup(FName InMappingName, const FText& NewLabel)
{
	MappingName = InMappingName;

	Label->SetText(NewLabel);
}

void URageKeybindRow::SetCurrentKey(FKey NewKey)
{
	CurrentKey = NewKey;
	
	KeyText->SetText(NewKey.IsValid() ? NewKey.GetDisplayName() : RAGE_LOC("KeyUnbound"));
}

FName URageKeybindRow::GetMappingName() const
{
	return MappingName;
}

bool URageKeybindRow::IsListeningForInput() const
{
	return bListeningForInput;
}

void URageKeybindRow::NativeConstruct()
{
	Super::NativeConstruct();

	SetIsFocusable(true);
	
	RemapButton->OnClicked.AddDynamic(this, &URageKeybindRow::HandleRemapButtonClicked);
	
	if (IsValid(ResetButton))
	{
		ResetButton->OnClicked.AddDynamic(this, &URageKeybindRow::HandleResetButtonClicked);
	}
}

FReply URageKeybindRow::NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent)
{
	if (bListeningForInput)
	{
		const FKey PressedKey = InKeyEvent.GetKey();
		if (PressedKey == EKeys::Escape)
		{
			EndListening(true);
		}
		else
		{
			TryCommitKey(PressedKey);
		}
		
		return FReply::Handled();
	}
	
	return Super::NativeOnKeyDown(InGeometry, InKeyEvent);
}

FReply URageKeybindRow::NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	if (bListeningForInput)
	{
		TryCommitKey(InMouseEvent.GetEffectingButton());
		
		return FReply::Handled();
	}
	
	return Super::NativeOnMouseButtonDown(InGeometry, InMouseEvent);
}

void URageKeybindRow::NativeOnFocusLost(const FFocusEvent& InFocusEvent)
{
	Super::NativeOnFocusLost(InFocusEvent);
	
	if (bListeningForInput)
	{
		EndListening(true);
	}
}

void URageKeybindRow::HandleRemapButtonClicked()
{
	if (!bListeningForInput)
	{
		BeginListening();
	}
}

void URageKeybindRow::HandleResetButtonClicked()
{
	ResetToDefaultRequestedDelegate.Broadcast(MappingName);
}

void URageKeybindRow::BeginListening()
{
	bListeningForInput = true;

	KeyText->SetText(RAGE_LOC("PressAnyKey"));

	SetButtonsEnabled(false);
	SetKeyboardFocus();
}

void URageKeybindRow::EndListening(bool bCancelled)
{
	bListeningForInput = false;

	SetButtonsEnabled(true);

	if (bCancelled)
	{
		SetCurrentKey(CurrentKey);
	}
}

void URageKeybindRow::SetButtonsEnabled(bool bEnabled)
{
	RemapButton->SetIsEnabled(bEnabled);

	if (IsValid(ResetButton))
	{
		ResetButton->SetIsEnabled(bEnabled);
	}
}

void URageKeybindRow::TryCommitKey(FKey NewKey)
{
	if (NewKey == CurrentKey)
	{
		EndListening(true);
		return;
	}

	EndListening(false);
	SetCurrentKey(NewKey);
	KeyRemappedDelegate.Broadcast(MappingName, NewKey);
}
