#include "Runtime/QTEMashRuntime.h"

void FQTEMashRuntime::Start()
{
	BeginCurrentStep(false);
}

bool FQTEMashRuntime::HandlePressed(const UInputAction* InputAction)
{
	const FQTEStepDefinition* Step = ResolveInputStep(InputAction, true);
	return Step && Step->StepType == EQTEStepType::Mash
		? AdvanceMashStep()
		: false;
}

bool FQTEMashRuntime::HandleReleased(const UInputAction* InputAction)
{
	ResolveInputStep(InputAction, false);
	return false;
}

bool FQTEMashRuntime::HandleTriggered(const UInputAction* InputAction)
{
	ResolveInputStep(InputAction, false);
	return false;
}

void FQTEMashRuntime::RefreshStepProgress(const FQTEStepDefinition&)
{
	RefreshMashStepProgress();
}
