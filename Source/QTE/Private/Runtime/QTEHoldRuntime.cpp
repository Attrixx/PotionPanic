#include "Runtime/QTEHoldRuntime.h"

void FQTEHoldRuntime::Start()
{
	BeginCurrentStep(false);
}

bool FQTEHoldRuntime::HandlePressed(const UInputAction* InputAction)
{
	const FQTEStepDefinition* Step = ResolveInputStep(InputAction, true);
	return Step && Step->StepType == EQTEStepType::Hold
		? StartHoldStep()
		: false;
}

bool FQTEHoldRuntime::HandleReleased(const UInputAction* InputAction)
{
	const FQTEStepDefinition* Step = ResolveInputStep(InputAction, false);
	return Step && Step->StepType == EQTEStepType::Hold
		? ReleaseHoldStep(*Step)
		: false;
}

bool FQTEHoldRuntime::HandleTriggered(const UInputAction* InputAction)
{
	const FQTEStepDefinition* Step = ResolveInputStep(InputAction, false);
	return Step && Step->StepType == EQTEStepType::Hold
		? UpdateHoldStep()
		: false;
}

void FQTEHoldRuntime::RefreshStepProgress(const FQTEStepDefinition&)
{
	RefreshHoldStepProgress();
}
