// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Class.h"
#include "ActivityStepSettings.generated.h"

class UActivityStep;

/**
 * Base configuration class for defining and instantiating an Activity Step.
 * Designed to configure step properties at edit time, and act as a factory at runtime.
 */
UCLASS(Abstract, EditInlineNew, Blueprintable)
class ACTIVITIES_API UActivityStepSettings : public UObject
{
	GENERATED_BODY()

public:

	/**
	 * Factory method that allocates and initializes the runtime step based on these settings.
	 * @param Outer The object that will own the instantiated step.
	 * @return The newly created UActivityStep instance, or nullptr if the instantiation failed.
	 * @warning Child classes (both C++ and Blueprint) MUST override this method.
	 */
	UFUNCTION(BlueprintNativeEvent)
	UActivityStep* CreateStep(UObject* Outer) const;
	
protected:
	
	virtual UActivityStep* CreateStep_Implementation(UObject* Outer) const PURE_VIRTUAL(UActivityStepSettings::CreateStep, return nullptr;);
};
