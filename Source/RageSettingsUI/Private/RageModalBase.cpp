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
	NativeOnOpenStateChanged(true);
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
	NativeOnOpenStateChanged(false);
	OnModalOpenStateChanged(false);
}

bool URageModalBase::IsOpen() const
{
	return bIsOpen;
}

void URageModalBase::SetTitle(const FText& NewTitle)
{
	Title = NewTitle;

	if (IsValid(TitleText))
	{
		TitleText->SetText(NewTitle);
	}

	OnTitleSet(NewTitle);
	NativeOnContentChanged();
}

void URageModalBase::SetMessage(const FText& NewMessage)
{
	Message = NewMessage;

	if (IsValid(MessageText))
	{
		MessageText->SetText(NewMessage);
	}

	OnMessageSet(NewMessage);
	NativeOnContentChanged();
}

void URageModalBase::SetChanges(const TArray<FRageModalChange>& NewChanges)
{
	Changes = NewChanges;

	OnChangesSet(Changes);
	NativeOnContentChanged();
}

const FText& URageModalBase::GetTitle() const
{
	return Title;
}

const FText& URageModalBase::GetMessage() const
{
	return Message;
}

const TArray<FRageModalChange>& URageModalBase::GetChanges() const
{
	return Changes;
}

void URageModalBase::NativeConstruct()
{
	Super::NativeConstruct();

	bIsOpen = false;
	SetVisibility(ESlateVisibility::Collapsed);
}

void URageModalBase::NativeOnContentChanged()
{
}

void URageModalBase::NativeOnOpenStateChanged(bool bIsModalOpen)
{
}
