// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ActivityEvaluationResult.generated.h"

UENUM(BlueprintType)
enum class EActivityFlowDecision : uint8
{
	Continue, // Start the next step, or conclude the activity with success if none remain.
	Fail, // Skip remaining steps and instantly conclude the activity with a failure state.
	Restart // Reset and restart the activity from the beginning without concluding.
};

USTRUCT(BlueprintType)
struct FActivityEvaluationResult
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite)
	EActivityFlowDecision FlowDecision = EActivityFlowDecision::Continue;

	UPROPERTY(BlueprintReadWrite)
	int32 Score = 0;
};
