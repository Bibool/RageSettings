// Copyright (c) 2026 Abdallah Boutrif
// SPDX-License-Identifier: MIT

#include "RageUnsavedChangesModal.h"

#include "Components/Button.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(RageUnsavedChangesModal)

void URageUnsavedChangesModal::NativeConstruct()
{
	Super::NativeConstruct();

	ApplyAndCloseButton->OnClicked.AddUniqueDynamic(this, &URageUnsavedChangesModal::HandleApplyAndCloseClicked);
	DiscardAndCloseButton->OnClicked.AddUniqueDynamic(this, &URageUnsavedChangesModal::HandleDiscardAndCloseClicked);
	CancelButton->OnClicked.AddUniqueDynamic(this, &URageUnsavedChangesModal::HandleCancelClicked);
}

void URageUnsavedChangesModal::HandleApplyAndCloseClicked()
{
	ApplyAndCloseChosenDelegate.Broadcast();
}

void URageUnsavedChangesModal::HandleDiscardAndCloseClicked()
{
	DiscardAndCloseChosenDelegate.Broadcast();
}

void URageUnsavedChangesModal::HandleCancelClicked()
{
	CancelChosenDelegate.Broadcast();
}
