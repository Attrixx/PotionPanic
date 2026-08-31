#include "Runtime/QTEPressRuntime.h"

void FQTEPressRuntime::Start()
{
	BeginCurrentStep(false);
}

bool FQTEPressRuntime::HandlePressed(const UInputAction* InputAction)
{
	const FQTEStepDefinition* Step = ResolveInputStep(InputAction, true);
	return Step && Step->StepType == EQTEStepType::Press
		? CompletePressStep()
		: false;
}

bool FQTEPressRuntime::HandleReleased(const UInputAction* InputAction)
{
	ResolveInputStep(InputAction, false);
	return false;
}

bool FQTEPressRuntime::HandleTriggered(const UInputAction* InputAction)
{
	ResolveInputStep(InputAction, false);
	return false;
}

void FQTEPressRuntime::RefreshStepProgress(const FQTEStepDefinition&)
{
	RefreshPressStepProgress();
}
