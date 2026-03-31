// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "ActivityExecutionState.h"
#include "ActivityEvaluator.generated.h"

struct FActivityStepResult;

UENUM(BlueprintType)
enum class EActivityFlowDecision : uint8
{
	Continue, // Start the next step or Concludes the activity if there is none.
	Fail, // Skip remaining steps and Concludes the activity
	Restart // Restart the activity without concluding.
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
		const FActivityExecutionState& State, const FActivityStepResult& StepResult) const PURE_VIRTUAL(UActivityEvaluator::EvaluateStep, return {};);
};
