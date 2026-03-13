// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "ActivityEvaluator.generated.h"

struct FActivityExecutionState;
struct FActivityStepResult;

UENUM()
enum class EActivityFlowDecision
{
	Continue, // Start the next step or Concludes the activity if there is none.
	Fail, // Skip remaining steps and Concludes the activity
	Restart // Restart the activity without concluding.
};

USTRUCT(blueprintType)
struct FActivityEvaluationResult
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite)
	EActivityFlowDecision FlowDecision;

	UPROPERTY(BlueprintReadWrite)
	int32 Score;
};

/**
 * 
 */
UCLASS(Abstract, EditInlineNew, Blueprintable)
class ACTIVITIES_API UActivityEvaluator : public UObject
{
	GENERATED_BODY()

public:

	/**
	 *
	 */
	UFUNCTION(BlueprintNativeEvent)
	FActivityEvaluationResult EvaluateStep(const FActivityExecutionState& State, const FActivityStepResult& StepResult) const;

protected:

	virtual FActivityEvaluationResult EvaluateStep_Implementation(
		const FActivityExecutionState& State, const FActivityStepResult& StepResult) const PURE_VIRTUAL(UActivityEvaluator::EvaluateStep, return {};)
};
