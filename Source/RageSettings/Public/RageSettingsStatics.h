// Copyright (c) 2026 Abdallah Boutrif
// SPDX-License-Identifier: MIT

#pragma once

#include "UObject/PropertyPortFlags.h" // PPF_None

namespace RageSettings
{
	template <typename TStructType>
	bool AreEqual(const TStructType& A, const TStructType& B)
	{
		return TStructType::StaticStruct()->CompareScriptStruct(&A, &B, PPF_None);
	}
}
