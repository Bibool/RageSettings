// Copyright (c) 2026 Abdallah Boutrif
// SPDX-License-Identifier: MIT

#include "RageUnsavedChangesModal.h"

#include "Components/Button.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(RageUnsavedChangesModal)

void URageUnsavedChangesModal::ChooseApplyAndClose()
{
	ApplyAndCloseChosenDelegate.Broadcast();
}

void URageUnsavedChangesModal::ChooseDiscardAndClose()
{
	DiscardAndCloseChosenDelegate.Broadcast();
}

void URageUnsavedChangesModal::ChooseCancel()
{
	CancelChosenDelegate.Broadcast();
}

void URageUnsavedChangesModal::NativeConstruct()
{
	Super::NativeConstruct();

	if (IsValid(ApplyAndCloseButton))
	{
		ApplyAndCloseButton->OnClicked.AddUniqueDynamic(this, &URageUnsavedChangesModal::HandleApplyAndCloseClicked);
	}

	if (IsValid(DiscardAndCloseButton))
	{
		DiscardAndCloseButton->OnClicked.AddUniqueDynamic(this, &URageUnsavedChangesModal::HandleDiscardAndCloseClicked);
	}

	if (IsValid(CancelButton))
	{
		CancelButton->OnClicked.AddUniqueDynamic(this, &URageUnsavedChangesModal::HandleCancelClicked);
	}
}

void URageUnsavedChangesModal::HandleApplyAndCloseClicked()
{
	ChooseApplyAndClose();
}

void URageUnsavedChangesModal::HandleDiscardAndCloseClicked()
{
	ChooseDiscardAndClose();
}

void URageUnsavedChangesModal::HandleCancelClicked()
{
	ChooseCancel();
}
