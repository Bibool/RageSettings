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

	OnPipStateChanged(PipState);
}
