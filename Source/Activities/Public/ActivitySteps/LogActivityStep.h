// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ActivityStep.h"
#include "ActivityStepSettings.h"
#include "LogActivityStep.generated.h"

UCLASS(DisplayName = "Log")
class ACTIVITIES_API ULogActivitySettings : public UActivityStepSettings
{
	GENERATED_BODY()

#if WITH_EDITOR
	EDataValidationResult IsDataValid(FDataValidationContext& Context) const override;
#endif

	UActivityStep* CreateStep_Implementation(UObject* Outer) const override;

public:

	/** Text written to the log when the step runs. Debug aid only, nothing reads it back. */
	UPROPERTY(EditAnywhere, Category="", meta=(MultiLine))
	FString Message;
};

/**
 * Debug marker: writes Message to the log, then succeeds at once. Drop it between two steps to see
 * an activity's progression, it changes nothing else.
 */
UCLASS()
class ACTIVITIES_API ULogActivityStep : public UActivityStep
{
	GENERATED_BODY()

	void StartStep_Implementation(AActor* LastInstigator) override;

	friend ULogActivitySettings;

	FString Message;
};
