// Copyright (c) 2026 Abdallah Boutrif
// SPDX-License-Identifier: MIT

#include "RageSettingsReflectionUtils.h"

#include "Misc/ConfigCacheIni.h"
#include "UObject/PropertyPortFlags.h"
#include "UObject/UnrealType.h"

namespace
{
	const FString LayersFilename = TEXT("RageSettingsDefaults.ini");
}

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

UObject* RageSettings::CreateDefaultsInstance(UObject* Outer, UClass* Class)
{
	/* Mimics UClass CDO construction since NewObject would copy from the loaded config vals as opposed to initialized members.*/
	UObject* Instance = StaticAllocateObject(Class, Outer, NAME_None, RF_NoFlags);
	const EObjectInitializerOptions InitOptions = Class->HasAnyClassFlags(CLASS_Native | CLASS_Intrinsic)
		? EObjectInitializerOptions::None : EObjectInitializerOptions::InitializeProperties;
	(*Class->ClassConstructor)(FObjectInitializer(Instance, nullptr, InitOptions));

	const FConfigBranch* Branch = GConfig->FindBranch(Class->ClassConfigName, Class->GetConfigName());
	if (!Branch)
	{
		return Instance;
	}
	
	GConfig->Add(LayersFilename, Branch->FinalCombinedLayers);
	
	TArray<UClass*> ConfigClasses;
	for (UClass* ConfigClass = Class; ConfigClass && ConfigClass->HasAnyClassFlags(CLASS_Config); ConfigClass = ConfigClass->GetSuperClass())
	{
		ConfigClasses.Insert(ConfigClass, 0);
	}

	for (UClass* ConfigClass : ConfigClasses)
	{
		Instance->LoadConfig(ConfigClass, *LayersFilename);
	}

	GConfig->Remove(LayersFilename);

	return Instance;
}
