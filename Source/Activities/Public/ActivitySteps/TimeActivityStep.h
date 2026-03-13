// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ActivityStep.h"
#include "ActivityStepSettings.h"
#include "TimeActivityStep.generated.h"

UCLASS(DisplayName = "Time")
class ACTIVITIES_API UTimeActivitySettings : public UActivityStepSettings
{
	GENERATED_BODY()

#if WITH_EDITOR
	EDataValidationResult IsDataValid(FDataValidationContext& Context) const override;
#endif

	UActivityStep* CreateStep_Implementation(UObject* Outer) const override;

public:

	UPROPERTY(EditAnywhere, Category="")
	float SecondsToWait = 0.f;
};

UCLASS()
class ACTIVITIES_API UTimeActivityStep : public UActivityStep
{
	GENERATED_BODY()

	void StartStep_Implementation(AActor* Performer) override;

	friend UTimeActivitySettings;

	float SecondsToWait = 0.f;
};
