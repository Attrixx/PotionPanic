// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Templates/SubclassOf.h"
#include "StructUtils/InstancedStruct.h"
#include "ActivityStepPresentation.generated.h"

class UActivityStep;


USTRUCT(Blueprintable)
struct ACTIVITIES_API FActivityStepPresentationCustomInfo
{
	GENERATED_BODY()
};

/**
 * Everything a client needs to draw the running step. Replicated to everyone.
 */
USTRUCT(BlueprintType)
struct ACTIVITIES_API FActivityStepPresentation
{
	GENERATED_BODY()

	/**
	 * The step being drawn, which is also what picks the widget: UActivityDisplaySettings maps step
	 * classes to widget classes, so a new step only has to add a row there. Null when nothing is
	 * running, and stamped by the executor rather than by the steps themselves.
	 */
	UPROPERTY(BlueprintReadOnly)
	TSubclassOf<UActivityStep> StepClass = nullptr;

	/**
	 * Actor the step is running for, null when it has no owner yet.
	 */
	UPROPERTY(BlueprintReadOnly)
	TObjectPtr<AActor> Instigator = nullptr;

	/** Server time this became the thing being drawn. */
	UPROPERTY(BlueprintReadOnly)
	double StartServerTime = 0.0;
	
	UPROPERTY(BlueprintReadOnly)
	TInstancedStruct<FActivityStepPresentationCustomInfo> CustomInfo;
	
	UPROPERTY(BlueprintReadOnly)
	uint8 Revision = 0;
};
