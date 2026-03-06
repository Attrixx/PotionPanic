// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "ActivityStepSettings.generated.h"

class UActivityStep;

/**
 * 
 */
UCLASS(Abstract, EditInlineNew)
class ACTIVITIES_API UActivityStepSettings : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:

	UFUNCTION(BlueprintCallable)
	virtual UActivityStep* CreateStep(UObject* Outer) const PURE_VIRTUAL(UActivityStepSettings::CreateStep, return nullptr;)
};
