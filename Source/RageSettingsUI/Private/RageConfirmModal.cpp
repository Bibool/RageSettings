// Copyright (c) 2026 Abdallah Boutrif
// SPDX-License-Identifier: MIT

#include "RageConfirmModal.h"

#include "Components/Button.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(RageConfirmModal)

void URageConfirmModal::NativeConstruct()
{
	Super::NativeConstruct();

	ConfirmButton->OnClicked.AddUniqueDynamic(this, &URageConfirmModal::HandleConfirmClicked);
	CancelButton->OnClicked.AddUniqueDynamic(this, &URageConfirmModal::HandleCancelClicked);
}

void URageConfirmModal::HandleConfirmClicked()
{
	ConfirmChosenDelegate.Broadcast();
}

void URageConfirmModal::HandleCancelClicked()
{
	CancelChosenDelegate.Broadcast();
}
