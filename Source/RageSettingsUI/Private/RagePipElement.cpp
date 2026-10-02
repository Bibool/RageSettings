// Copyright (c) 2026 Abdallah Boutrif
// SPDX-License-Identifier: MIT

#include "RagePipElement.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(RagePipElement)

void URagePipElement::Setup(int32 InPipIndex)
{
	PipIndex = InPipIndex;
}

void URagePipElement::SetPipState(ERagePipState NewState)
{
	if (NewState == PipState)
	{
		return;
	}

	PipState = NewState;
	NativeOnPipStateChanged(PipState);
	OnPipStateChanged(PipState);
}

ERagePipState URagePipElement::GetPipState() const
{
	return PipState;
}

int32 URagePipElement::GetPipIndex() const
{
	return PipIndex;
}

bool URagePipElement::IsFilled() const
{
	return PipState != ERagePipState::Empty;
}

void URagePipElement::NativeConstruct()
{
	Super::NativeConstruct();

	NativeOnPipStateChanged(PipState);
	OnPipStateChanged(PipState);
}

void URagePipElement::AssignSlot(UPanelSlot* BaseSlot)
{
	NativeOnSlotAssigned(BaseSlot);
	OnSlotAssigned(BaseSlot);
}

void URagePipElement::NativeOnSlotAssigned(UPanelSlot* BaseSlot)
{
}

void URagePipElement::NativeOnPipStateChanged(ERagePipState NewState)
{
}
