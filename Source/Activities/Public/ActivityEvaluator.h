// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "ActivityEvaluationResult.h"
#include "ActivityEvaluator.generated.h"

struct FActivityExecutionState;
struct FActivityStepResult;

/**
 * @brief Abstract base class responsible for evaluating the outcome of an activity step.
 */
UCLASS(Abstract, EditInlineNew, Blueprintable)
class ACTIVITIES_API UActivityEvaluator : public UObject
{
	GENERATED_BODY()

public:

	/**
	 * Evaluates the current state and the result of the last executed step to determine the next flow decision.
	 * @param State The current execution context of the activity.
	 * @param StepResult The output data of the step that just finished.
	 * @return The evaluation result dictating whether the activity continues, fails, or restarts.
	 * @warning Child classes (both C++ and Blueprint) MUST override this method.
	 */
	UFUNCTION(BlueprintNativeEvent)
	FActivityEvaluationResult EvaluateStep(const FActivityExecutionState& State, const FActivityStepResult& StepResult) const;

protected:

	virtual FActivityEvaluationResult EvaluateStep_Implementation(
		const FActivityExecutionState& State, const FActivityStepResult& StepResult) const PURE_VIRTUAL(UActivityEvaluator::EvaluateStep, return {};);
};
