// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Class.h"
#include "ActivityStepSettings.generated.h"

class UActivityStep;

/**
 * 
 */
UCLASS(Abstract, EditInlineNew, Blueprintable)
class ACTIVITIES_API UActivityStepSettings : public UObject
{
	GENERATED_BODY()

public:

	UFUNCTION(BlueprintNativeEvent)
	UActivityStep* CreateStep(UObject* Outer) const;
	
protected:
	
	virtual UActivityStep* CreateStep_Implementation(UObject* Outer) const PURE_VIRTUAL(UActivityStepSettings::CreateStep, return nullptr;);
};
