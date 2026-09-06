// Fill out your copyright notice in the Description page of Project Settings.

#include "ActivityDisplaySettings.h"
#include "ActivityStep.h"

FName UActivityDisplaySettings::GetCategoryName() const
{
	return TEXT("Game");
}

TSoftClassPtr<UActivityStepWidget> UActivityDisplaySettings::GetWidgetClass(TSubclassOf<UActivityStep> StepClass) const
{
	for (UClass* Class = StepClass; Class; Class = Class->GetSuperClass())
	{
		if (const TSoftClassPtr<UActivityStepWidget>* Found = StepWidgets.Find(Class))
		{
			return *Found;
		}
	}

	return nullptr;
}
