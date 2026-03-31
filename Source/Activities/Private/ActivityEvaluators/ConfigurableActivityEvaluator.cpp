// Fill out your copyright notice in the Description page of Project Settings.


#include "ActivityEvaluators/ConfigurableActivityEvaluator.h"
#include "ActivityExecutionState.h"
#include "ActivityStepResult.h"

FActivityEvaluationResult UConfigurableActivityEvaluator::EvaluateStep_Implementation(
	const FActivityExecutionState& State, const FActivityStepResult& StepResult) const
{
	FActivityEvaluationResult Result;
	EScoreManagementMethod ScoreManagementMethod = EScoreManagementMethod::Discard;
	switch (StepResult.Status)
	{
	case EActivityStepStatus::Success:
		Result.FlowDecision = FlowDecisionOnSuccess;
		ScoreManagementMethod = ScoreManagementMethodOnSuccess;
		break;

	case EActivityStepStatus::Fail:
		Result.FlowDecision = FlowDecisionOnFail;
		ScoreManagementMethod = ScoreManagementMethodOnFail;
		break;

	case EActivityStepStatus::CriticalFail:
		Result.FlowDecision = FlowDecisionOnCriticalFail;
		ScoreManagementMethod = ScoreManagementMethodOnCriticalFail;
		break;

	default:
		checkNoEntry();
	}
	
	switch (ScoreManagementMethod)
	{
	case EScoreManagementMethod::Discard: break;
	case EScoreManagementMethod::AddToScore: Result.Score = State.Score + StepResult.Score; break;
	case EScoreManagementMethod::RemoveFromScore: Result.Score = State.Score - StepResult.Score; break;
	case EScoreManagementMethod::MultiplyScore: Result.Score = State.Score * StepResult.Score; break;
	case EScoreManagementMethod::DivideScore: Result.Score = State.Score / StepResult.Score; break;
	default:
		checkNoEntry();
	}

	return Result;
}
