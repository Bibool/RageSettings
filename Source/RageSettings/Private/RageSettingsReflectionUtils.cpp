// Copyright (c) 2026 Abdallah Boutrif
// SPDX-License-Identifier: MIT

#include "RageSettingsReflectionUtils.h"

#include "UObject/PropertyPortFlags.h"
#include "UObject/UnrealType.h"

bool RageSettings::AreObjectsEqual(const UObject* A, const UObject* B)
{
	check(IsValid(A) && IsValid(B) && A->GetClass() == B->GetClass());

	for (TFieldIterator<FProperty> It(A->GetClass()); It; ++It)
	{
		const FProperty* Property = *It;
		if (!Property->HasAnyPropertyFlags(CPF_Config))
		{
			continue;
		}

		if (!Property->Identical(Property->ContainerPtrToValuePtr<void>(A), Property->ContainerPtrToValuePtr<void>(B), PPF_None))
		{
			return false;
		}
	}

	return true;
}

TArray<FName> RageSettings::CollectChangedProperties(const UObject* A, const UObject* B)
{
	TArray<FName> Changed;

	if (!IsValid(A) || !IsValid(B) || A->GetClass() != B->GetClass())
	{
		return Changed;
	}

	for (TFieldIterator<FProperty> It(A->GetClass()); It; ++It)
	{
		const FProperty* Property = *It;
		if (!Property->HasAnyPropertyFlags(CPF_Config))
		{
			continue;
		}

		if (!Property->Identical(Property->ContainerPtrToValuePtr<void>(A), Property->ContainerPtrToValuePtr<void>(B), PPF_None))
		{
			Changed.Add(Property->GetFName());
		}
	}

	return Changed;
}

void RageSettings::CopyObjectProperties(UObject* Dest, const UObject* Src)
{
	check(IsValid(Dest) && IsValid(Src) && Dest->GetClass() == Src->GetClass());

	for (TFieldIterator<FProperty> It(Dest->GetClass()); It; ++It)
	{
		const FProperty* Property = *It;
		if (!Property->HasAnyPropertyFlags(CPF_Config))
		{
			continue;
		}

		Property->CopyCompleteValue(Property->ContainerPtrToValuePtr<void>(Dest), Property->ContainerPtrToValuePtr<void>(Src));
	}
}

UObject* RageSettings::CreateShadowInstance(UObject* Outer, const UObject* Source, FName Name)
{
	UObject* Instance = NewObject<UObject>(Outer, Source->GetClass(), Name);
	CopyObjectProperties(Instance, Source);
	return Instance;
}
