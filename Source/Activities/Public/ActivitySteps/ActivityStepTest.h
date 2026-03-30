// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ActivityStep.h"
#include "ActivityStepSettings.h"
#include "ActivityStepTest.generated.h"

UCLASS()
class ACTIVITIES_API UTestActivitySetting : public UActivityStepSettings
{
	GENERATED_BODY()
	
	UActivityStep* CreateStep(UObject* Outer) const override;
};

/**
 * 
 */
UCLASS(BlueprintType)
class ACTIVITIES_API UActivityStepTest : public UActivityStep
{
	GENERATED_BODY()
	
public:	

	void StartActivity(const FActivityContext& Context) override;
	bool RequiresPlayerInteraction() const override { return false; }
};

