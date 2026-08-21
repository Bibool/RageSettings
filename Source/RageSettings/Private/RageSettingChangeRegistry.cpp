// Copyright (c) 2026 Abdallah Boutrif
// SPDX-License-Identifier: MIT

#include "RageSettingChangeRegistry.h"

void FRageSettingChangeRegistry::Broadcast(ERageSettingsCategory Category, const TArray<FName>& ChangedProperties)
{
	for (const FName PropertyName : ChangedProperties)
	{
		if (const FRageSettingChangedNative* Delegate = PerProperty.Find(PropertyName))
		{
			Delegate->Broadcast(Category, PropertyName);
		}

		AnyProperty.Broadcast(Category, PropertyName);
	}
}
