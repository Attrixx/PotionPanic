// Fill out your copyright notice in the Description page of Project Settings.


#include "ActivityEvaluators/ConfigurableActivityEvaluator.h"

#include "ActivityExecutor.h"
#include "ActivityStep.h"

FActivityEvaluationResult UConfigurableActivityEvaluator::EvaluateStep_Implementation(
	const FActivityExecutionState& State, const FActivityStepResult& StepResult) const
{
	auto ScoreManagementMethod = [](EScoreManagementMethod Method, int32 ActivityScore, int32 StepScore) -> int32
	{
		int32 NewScore = ActivityScore;
		switch (Method)
		{
		case EScoreManagementMethod::Discard: NewScore = ActivityScore; break;
		case EScoreManagementMethod::AddToScore: NewScore = ActivityScore + StepScore; break;
		case EScoreManagementMethod::RemoveFromScore: NewScore = ActivityScore - StepScore; break;
		case EScoreManagementMethod::MultiplyScore: NewScore = ActivityScore * StepScore; break;
		case EScoreManagementMethod::DivideScore: NewScore = ActivityScore / StepScore; break;
		default:
			checkNoEntry();
		}
		return NewScore;
	};

	FActivityEvaluationResult Result;
	switch (StepResult.Status)
	{
	case EActivityStepStatus::Success:
		Result.FlowDecision = FlowDecisionOnSuccess;
		Result.Score = ScoreManagementMethod(ScoreManagementMethodOnSuccess, State.Score, StepResult.Score);
		break;

	case EActivityStepStatus::Fail:
		Result.FlowDecision = FlowDecisionOnFail;
		Result.Score = ScoreManagementMethod(ScoreManagementMethodOnFail, State.Score, StepResult.Score);
		break;

	case EActivityStepStatus::CriticalFail:
		Result.FlowDecision = FlowDecisionOnCriticalFail;
		Result.Score = ScoreManagementMethod(ScoreManagementMethodOnCriticalFail, State.Score, StepResult.Score);
		break;

	default:
		checkNoEntry();
	}

	return Result;
}
