#include "Runtime/QTEChainRuntime.h"

void FQTEChainRuntime::Start()
{
	BeginCurrentStep(false);
}

bool FQTEChainRuntime::HandlePressed(const UInputAction* InputAction)
{
	const FQTEStepDefinition* Step = ResolveInputStep(InputAction, true);
	return Step ? HandleCurrentStepPressedByType(*Step) : false;
}

bool FQTEChainRuntime::HandleReleased(const UInputAction* InputAction)
{
	const FQTEStepDefinition* Step = ResolveInputStep(InputAction, false);
	return Step ? HandleCurrentStepReleasedByType(*Step) : false;
}

bool FQTEChainRuntime::HandleTriggered(const UInputAction* InputAction)
{
	const FQTEStepDefinition* Step = ResolveInputStep(InputAction, false);
	return Step ? HandleCurrentStepTriggeredByType(*Step) : false;
}

void FQTEChainRuntime::RefreshStepProgress(const FQTEStepDefinition& Step)
{
	RefreshCurrentStepByType(Step);
}
