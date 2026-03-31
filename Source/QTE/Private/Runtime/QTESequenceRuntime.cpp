#include "Runtime/QTESequenceRuntime.h"

void FQTESequenceRuntime::Start()
{
	BeginCurrentStep(false);
}

bool FQTESequenceRuntime::HandlePressed(const UInputAction* InputAction)
{
	const FQTEStepDefinition* Step = ResolveInputStep(InputAction, true);
	return Step ? HandleCurrentStepPressedByType(*Step) : false;
}

bool FQTESequenceRuntime::HandleReleased(const UInputAction* InputAction)
{
	const FQTEStepDefinition* Step = ResolveInputStep(InputAction, false);
	return Step ? HandleCurrentStepReleasedByType(*Step) : false;
}

bool FQTESequenceRuntime::HandleTriggered(const UInputAction* InputAction)
{
	const FQTEStepDefinition* Step = ResolveInputStep(InputAction, false);
	return Step ? HandleCurrentStepTriggeredByType(*Step) : false;
}

void FQTESequenceRuntime::RefreshStepProgress(const FQTEStepDefinition& Step)
{
	RefreshCurrentStepByType(Step);
}
