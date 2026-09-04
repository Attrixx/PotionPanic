// Fill out your copyright notice in the Description page of Project Settings.


#include "ActivityEvaluators/ConfigurableActivityEvaluator.h"
#include "ActivityAsset.h"
#include "ActivityExecutionState.h"
#include "ActivityStepResult.h"
#include <Misc/DataValidation.h>

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
	// A failed step commonly reports a score of 0 (UQTEActivityStep hands back its completed step
	// count, which is 0 when the first step fails), and an integer division by zero is a hardware
	// fault, not a NaN: it takes the game down. Leave the score untouched instead.
	case EScoreManagementMethod::DivideScore:
		Result.Score = StepResult.Score != 0 ? State.Score / StepResult.Score : State.Score;
		break;
	default:
		checkNoEntry();
	}

	return Result;
}

#if WITH_EDITOR
EDataValidationResult UConfigurableActivityEvaluator::IsDataValid(FDataValidationContext& Context) const
{
	EDataValidationResult Result = Super::IsDataValid(Context);

	// Continue means "start the next step, or conclude the activity with success if none remain".
	// On a single-step activity there is never a next step, so a failed step concludes as Success
	// and the conclusion applies its success outcome: the player fails the QTE and is rewarded
	// anyway. Continue on a failure only makes sense while later steps remain.
	const UActivityAsset* OwningActivity = GetTypedOuter<UActivityAsset>();
	if (!OwningActivity || OwningActivity->ActivitySteps.Num() != 1)
	{
		return Result;
	}

	if (FlowDecisionOnFail == EActivityFlowDecision::Continue)
	{
		Context.AddWarning(FText::FromString(
			"FlowDecisionOnFail is Continue on a single-step activity: a failed step leaves no next step, so the activity concludes with Success and the conclusion applies its success outcome. Set it to Fail unless failing really should reward the player."));
	}

	if (FlowDecisionOnCriticalFail == EActivityFlowDecision::Continue)
	{
		Context.AddWarning(FText::FromString(
			"FlowDecisionOnCriticalFail is Continue on a single-step activity: the activity would conclude with Success. Set it to Fail."));
	}

	return Result;
}
#endif
