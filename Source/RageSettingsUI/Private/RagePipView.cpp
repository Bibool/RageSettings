// Copyright (c) 2026 Abdallah Boutrif
// SPDX-License-Identifier: MIT

#include "RagePipView.h"

#include "RageSettingsSharedDebug.h"
#include "Components/PanelWidget.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(RagePipView)

void URagePipView::SetPipCount(int32 NewPipCount)
{
	if (!PipElementClass)
	{
		S_LOG(Warning, "Rage Settings : {view} has no PipElementClass - the pip strip will stay empty.", *GetName());
		return;
	}
	
	if (Pips.IsEmpty())
	{
		PipContainer->ClearChildren();
	}

	const int32 TargetCount = FMath::Max(NewPipCount, 0);

	for (int32 Index = Pips.Num(); Index < TargetCount; ++Index)
	{
		URagePipElement* Pip = CreateWidget<URagePipElement>(this, PipElementClass);
		if (!IsValid(Pip))
		{
			break;
		}

		Pip->Setup(Index);
		Pip->OnSlotAssigned(PipContainer->AddChild(Pip));
		Pips.Add(Pip);
		Pip->PipClickedDelegate.AddDynamic(this, &URagePipView::HandlePipClicked);
	}

	while (Pips.Num() > TargetCount)
	{
		URagePipElement* Pip = Pips.Pop();
		if (IsValid(Pip))
		{
			Pip->RemoveFromParent();
		}
	}

	RefreshPipStates();
}

void URagePipView::SetSelectedIndex(int32 NewIndex)
{
	if (NewIndex == SelectedIndex)
	{
		return;
	}
	
	SelectedIndex = NewIndex;
	RefreshPipStates();
}

void URagePipView::SetPipStyle(ERagePipStyle NewStyle)
{
	if (NewStyle == PipStyle)
	{
		return;
	}

	PipStyle = NewStyle;
	RefreshPipStates();
}

int32 URagePipView::GetPipCount() const
{
	return Pips.Num();
}

int32 URagePipView::GetSelectedIndex() const
{
	return SelectedIndex;
}

ERagePipState URagePipView::ResolvePipState(int32 PipIndex) const
{
	if (PipIndex < 0 || SelectedIndex < 0)
	{
		return ERagePipState::Empty;
	}

	if (PipIndex == SelectedIndex)
	{
		return ERagePipState::Current;
	}

	return PipStyle == ERagePipStyle::Cumulative && PipIndex < SelectedIndex ? ERagePipState::Filled : ERagePipState::Empty;
}

void URagePipView::HandlePipClicked(int32 PipIndex)
{
	PipClickedDelegate.Broadcast(PipIndex);
}

void URagePipView::RefreshPipStates() const
{
	for (int32 Index = 0; Index < Pips.Num(); ++Index)
	{
		if (IsValid(Pips[Index]))
		{
			Pips[Index]->SetPipState(ResolvePipState(Index));
		}
	}
}
