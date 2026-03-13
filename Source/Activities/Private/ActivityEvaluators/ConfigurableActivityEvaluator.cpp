// Fill out your copyright notice in the Description page of Project Settings.


#include "ActivityEvaluators/ConfigurableActivityEvaluator.h"

#include "ActivityExecutor.h"
#include "ActivityStep.h"

FActivityEvaluationResult UConfigurableActivityEvaluator::EvaluateStep_Implementation(
	const FActivityExecutionState& State, const FActivityStepResult& StepResult) const
{
	auto ScoreManagementMethod = [](EScoreManagementMethod Method, int32 ActivityScore, int32 StepScore) -> int32
	{
		switch (Method)
		{
		case EScoreManagementMethod::Discard: return ActivityScore;
		case EScoreManagementMethod::AddToScore: return ActivityScore + StepScore;
		case EScoreManagementMethod::RemoveFromScore: return ActivityScore - StepScore;
		case EScoreManagementMethod::MultiplyScore: return ActivityScore * StepScore;
		case EScoreManagementMethod::DivideScore: return ActivityScore / StepScore;
		default:
			checkNoEntry();
			return ActivityScore;
		}
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
