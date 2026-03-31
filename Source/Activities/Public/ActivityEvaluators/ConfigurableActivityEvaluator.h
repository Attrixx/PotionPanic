// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ActivityEvaluator.h"
#include "ConfigurableActivityEvaluator.generated.h"

UENUM()
enum class EScoreManagementMethod : uint8
{
	Discard = 0,
	AddToScore,
	RemoveFromScore,
	MultiplyScore,
	DivideScore,
};

/**
 * 
 */
UCLASS(DisplayName = "Configurable")
class ACTIVITIES_API UConfigurableActivityEvaluator : public UActivityEvaluator
{
	GENERATED_BODY()
	
protected:
	
	FActivityEvaluationResult EvaluateStep_Implementation(const FActivityExecutionState& State, const FActivityStepResult& StepResult) const override;
	
	UPROPERTY(EditAnywhere, Category="")
	EActivityFlowDecision FlowDecisionOnSuccess = EActivityFlowDecision::Continue;
	
	UPROPERTY(EditAnywhere, Category="")
	EActivityFlowDecision FlowDecisionOnFail = EActivityFlowDecision::Continue;
	
	UPROPERTY(EditAnywhere, Category="")
	EActivityFlowDecision FlowDecisionOnCriticalFail = EActivityFlowDecision::Fail;
	
	UPROPERTY(EditAnywhere, Category="")
	EScoreManagementMethod ScoreManagementMethodOnSuccess = EScoreManagementMethod::AddToScore;
	
	UPROPERTY(EditAnywhere, Category="")
	EScoreManagementMethod ScoreManagementMethodOnFail = EScoreManagementMethod::AddToScore;
	
	UPROPERTY(EditAnywhere, Category="")
	EScoreManagementMethod ScoreManagementMethodOnCriticalFail = EScoreManagementMethod::AddToScore;
};
