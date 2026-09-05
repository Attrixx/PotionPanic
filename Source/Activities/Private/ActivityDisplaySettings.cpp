// Fill out your copyright notice in the Description page of Project Settings.

#include "ActivityDisplaySettings.h"
#include "ActivityStep.h"

FName UActivityDisplaySettings::GetCategoryName() const
{
	return TEXT("Game");
}

TSoftClassPtr<UActivityStepWidget> UActivityDisplaySettings::GetWidgetClass(TSubclassOf<UActivityStep> StepClass) const
{
	// Walking up rather than looking the exact class up: a Blueprint step derived from a C++ one
	// inherits its widget, and a row on a base class is the default for everything under it.
	for (UClass* Class = StepClass; Class; Class = Class->GetSuperClass())
	{
		if (const TSoftClassPtr<UActivityStepWidget>* Found = StepWidgets.Find(Class))
		{
			return *Found;
		}
	}

	return nullptr;
}
