// Copyright (c) 2026 Abdallah Boutrif
// SPDX-License-Identifier: MIT

#include "RageModalBase.h"

#include "Components/TextBlock.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(RageModalBase)

void URageModalBase::Open()
{
	if (bIsOpen)
	{
		return;
	}

	bIsOpen = true;
	SetVisibility(ESlateVisibility::Visible);
	OnModalOpenStateChanged(true);
}

void URageModalBase::Close()
{
	if (!bIsOpen)
	{
		return;
	}

	bIsOpen = false;
	SetVisibility(ESlateVisibility::Collapsed);
	OnModalOpenStateChanged(false);
}

bool URageModalBase::IsOpen() const
{
	return bIsOpen;
}

void URageModalBase::SetMessage(const FText& NewMessage)
{
	if (IsValid(MessageText))
	{
		MessageText->SetText(NewMessage);
	}
}

void URageModalBase::NativeConstruct()
{
	Super::NativeConstruct();
	
	bIsOpen = false;
	SetVisibility(ESlateVisibility::Collapsed);
}
