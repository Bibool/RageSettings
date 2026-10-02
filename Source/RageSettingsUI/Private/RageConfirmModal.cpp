// Copyright (c) 2026 Abdallah Boutrif
// SPDX-License-Identifier: MIT

#include "RageConfirmModal.h"

#include "Components/Button.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(RageConfirmModal)

void URageConfirmModal::ChooseConfirm()
{
	ConfirmChosenDelegate.Broadcast();
}

void URageConfirmModal::ChooseAlternate()
{
	if (bAlternateChoiceOffered)
	{
		AlternateChosenDelegate.Broadcast();
	}
}

void URageConfirmModal::ChooseCancel()
{
	CancelChosenDelegate.Broadcast();
}

void URageConfirmModal::SetAlternateChoiceOffered(bool bOffered)
{
	bAlternateChoiceOffered = bOffered;

	if (IsValid(AlternateButton))
	{
		AlternateButton->SetVisibility(bOffered ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
	}

	NativeOnAlternateChoiceOfferedChanged(bOffered);
}

bool URageConfirmModal::OffersAlternateChoice() const
{
	return bAlternateChoiceOffered;
}

void URageConfirmModal::SetChoiceLabels(const FText& NewConfirmLabel, const FText& NewAlternateLabel)
{
	ConfirmLabel = NewConfirmLabel;
	AlternateLabel = NewAlternateLabel;

	OnChoiceLabelsSet(ConfirmLabel, AlternateLabel);
	NativeOnChoiceLabelsChanged();
}

const FText& URageConfirmModal::GetConfirmLabel() const
{
	return ConfirmLabel;
}

const FText& URageConfirmModal::GetAlternateLabel() const
{
	return AlternateLabel;
}

void URageConfirmModal::NativeConstruct()
{
	Super::NativeConstruct();

	if (IsValid(ConfirmButton))
	{
		ConfirmButton->OnClicked.AddUniqueDynamic(this, &URageConfirmModal::HandleConfirmClicked);
	}

	if (IsValid(AlternateButton))
	{
		AlternateButton->OnClicked.AddUniqueDynamic(this, &URageConfirmModal::HandleAlternateClicked);
	}

	if (IsValid(CancelButton))
	{
		CancelButton->OnClicked.AddUniqueDynamic(this, &URageConfirmModal::HandleCancelClicked);
	}

	SetAlternateChoiceOffered(bAlternateChoiceOffered);
}

void URageConfirmModal::NativeOnAlternateChoiceOfferedChanged(bool bOffered)
{
}

void URageConfirmModal::NativeOnChoiceLabelsChanged()
{
}

void URageConfirmModal::HandleConfirmClicked()
{
	ChooseConfirm();
}

void URageConfirmModal::HandleAlternateClicked()
{
	ChooseAlternate();
}

void URageConfirmModal::HandleCancelClicked()
{
	ChooseCancel();
}
