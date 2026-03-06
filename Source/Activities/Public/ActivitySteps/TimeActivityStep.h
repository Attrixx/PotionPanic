// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ActivityStep.h"
#include "ActivityStepSettings.h"
#include "TimeActivityStep.generated.h"

UCLASS()
class ACTIVITIES_API UTimeActivitySettings : public UActivityStepSettings
{
	GENERATED_BODY()

	UActivityStep* CreateStep(UObject* Outer) const override;

public:

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="")
	float SecondsToWait = 0.f;
};

UCLASS()
class ACTIVITIES_API UTimeActivityStep : public UActivityStep
{
	GENERATED_BODY()

public:

	void StartActivity(const FActivityContext& Context) override;
	bool RequiresPlayerInteraction() const override { return true; }

private:

	friend UTimeActivitySettings;

	float SecondsToWait = 0.f;
};
