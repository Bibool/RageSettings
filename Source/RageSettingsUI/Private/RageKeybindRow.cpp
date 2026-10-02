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

	SetRowId(InMappingName);
	SetLabel(NewLabel);
}

void URageKeybindRow::SetCurrentKey(FKey NewKey)
{
	CurrentKey = NewKey;

	ShowKeyText(GetValueText());

	/* A remap is saved the moment it is made, outside the pending and apply cycle, so whatever key the row
	 * shows is already the applied one and the row never has anything waiting on Apply. */
	CaptureBaseline();
}

FKey URageKeybindRow::GetCurrentKey() const
{
	return CurrentKey;
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

	if (IsValid(RemapButton))
	{
		RemapButton->OnClicked.AddUniqueDynamic(this, &URageKeybindRow::HandleRemapButtonClicked);
	}

	if (IsValid(ResetButton))
	{
		ResetButton->OnClicked.AddUniqueDynamic(this, &URageKeybindRow::HandleResetButtonClicked);
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

FReply URageKeybindRow::NativeOnPreviewMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	/* The preview pass reaches the row before any button inside it, so a key button that is still clickable
	 * cannot take the very click the row is waiting for. */
	if (bListeningForInput)
	{
		TryCommitKey(InMouseEvent.GetEffectingButton());

		return FReply::Handled();
	}

	return Super::NativeOnPreviewMouseButtonDown(InGeometry, InMouseEvent);
}

void URageKeybindRow::NativeOnFocusLost(const FFocusEvent& InFocusEvent)
{
	Super::NativeOnFocusLost(InFocusEvent);

	if (bListeningForInput)
	{
		EndListening(true);
	}
}

FString URageKeybindRow::GetValueKey() const
{
	/* An unbound key names itself None rather than nothing, so an unbound row still counts as a setting. */
	return CurrentKey.GetFName().ToString();
}

FText URageKeybindRow::GetValueText() const
{
	return CurrentKey.IsValid() ? CurrentKey.GetDisplayName() : RAGE_LOC("KeyUnbound");
}

void URageKeybindRow::NativeOnListeningChanged(bool bListening)
{
}

void URageKeybindRow::NativeOnKeyTextChanged(const FText& NewKeyText)
{
}

void URageKeybindRow::HandleRemapButtonClicked()
{
	if (!bListeningForInput)
	{
		BeginListening();
	}
}

void URageKeybindRow::RequestResetToDefault()
{
	ResetToDefaultRequestedDelegate.Broadcast(MappingName);
}

void URageKeybindRow::HandleResetButtonClicked()
{
	RequestResetToDefault();
}

void URageKeybindRow::BeginListening()
{
	if (bListeningForInput)
	{
		return;
	}

	bListeningForInput = true;

	ShowKeyText(RAGE_LOC("PressAnyKey"));

	SetButtonsEnabled(false);
	SetKeyboardFocus();

	NativeOnListeningChanged(true);
}

void URageKeybindRow::EndListening(bool bCancelled)
{
	bListeningForInput = false;

	SetButtonsEnabled(true);

	if (bCancelled)
	{
		SetCurrentKey(CurrentKey);
	}

	NativeOnListeningChanged(false);
}

void URageKeybindRow::SetButtonsEnabled(bool bEnabled)
{
	if (IsValid(RemapButton))
	{
		RemapButton->SetIsEnabled(bEnabled);
	}

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

void URageKeybindRow::ShowKeyText(const FText& NewKeyText)
{
	if (IsValid(KeyText))
	{
		KeyText->SetText(NewKeyText);
	}

	OnKeyTextSet(NewKeyText);
	NativeOnKeyTextChanged(NewKeyText);
}
