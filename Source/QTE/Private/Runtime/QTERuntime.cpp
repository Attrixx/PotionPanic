#include "Runtime/QTERuntime.h"

#include "Components/QTEComponent.h"
#include "Core/QTEDefinitionDataAsset.h"
#include "Runtime/QTERuntimeSettings.h"

TUniquePtr<FQTERuntime> FQTERuntime::Create(UQTEComponent& OwnerComponent, UQTEDefinitionDataAsset& Definition)
{
	Definition.EnsureRuntimeSettings();
	return Definition.RuntimeSettings
		? Definition.RuntimeSettings->CreateRuntime(OwnerComponent, Definition)
		: nullptr;
}

FQTERuntime::FQTERuntime(UQTEComponent& InOwnerComponent, UQTEDefinitionDataAsset& InDefinition)
	: OwnerComponent(InOwnerComponent)
	, Definition(InDefinition)
{
}

FQTERuntime::~FQTERuntime() = default;

void FQTERuntime::RefreshRuntimeState()
{
	const FQTEStepDefinition* CurrentStep = GetCurrentStep();
	FQTERuntimeState& RuntimeState = OwnerComponent.GetMutableRuntimeState();

	if (!CurrentStep)
	{
		RuntimeState.CurrentStepIndex = INDEX_NONE;
		RuntimeState.EffectiveMashTarget = 0;
		RuntimeState.EffectiveHoldTime = 0.f;
		RuntimeState.EffectiveStepTimeout = 0.f;
		RuntimeState.EffectiveTolerance = 0.f;
		RuntimeState.StepTimeRemaining = 0.f;
		const float GlobalTimeout = Definition.GetEffectiveGlobalTimeout();
		RuntimeState.GlobalTimeRemaining = GlobalTimeout > 0.f
			? FMath::Max(GlobalTimeout - RuntimeState.ElapsedTime, 0.f)
			: 0.f;
		RuntimeState.StepProgress = RuntimeState.Status == EQTEState::Success ? 1.f : 0.f;
		RuntimeState.OverallProgress = RuntimeState.TotalSteps > 0
			? FMath::Clamp(static_cast<float>(RuntimeState.CompletedStepCount) / static_cast<float>(RuntimeState.TotalSteps), 0.f, 1.f)
			: 0.f;
		return;
	}

	RuntimeState.EffectiveTolerance = Definition.GetEffectiveTolerance(*CurrentStep);
	RuntimeState.EffectiveHoldTime = Definition.GetEffectiveHoldTime(*CurrentStep);
	RuntimeState.EffectiveMashTarget = Definition.GetEffectiveMashTarget(*CurrentStep);
	RuntimeState.EffectiveStepTimeout = Definition.GetEffectiveStepTimeout(*CurrentStep);
	const float GlobalTimeout = Definition.GetEffectiveGlobalTimeout();
	RuntimeState.GlobalTimeRemaining = GlobalTimeout > 0.f
		? FMath::Max(GlobalTimeout - RuntimeState.ElapsedTime, 0.f)
		: 0.f;
	RuntimeState.StepTimeRemaining = RuntimeState.EffectiveStepTimeout > 0.f
		? FMath::Max(RuntimeState.EffectiveStepTimeout - RuntimeState.StepElapsedTime, 0.f)
		: 0.f;

	RefreshStepProgress(*CurrentStep);

	const float CompletedStepsAsFloat = static_cast<float>(RuntimeState.CompletedStepCount);
	const float TotalStepsAsFloat = static_cast<float>(FMath::Max(RuntimeState.TotalSteps, 1));
	RuntimeState.OverallProgress = FMath::Clamp((CompletedStepsAsFloat + RuntimeState.StepProgress) / TotalStepsAsFloat, 0.f, 1.f);
}

bool FQTERuntime::TryAutoCompleteHoldStep()
{
	const FQTEStepDefinition* CurrentStep = GetCurrentStep();
	FQTERuntimeState& RuntimeState = OwnerComponent.GetMutableRuntimeState();
	if (!CurrentStep || CurrentStep->StepType != EQTEStepType::Hold || !RuntimeState.bInputHeld)
	{
		return false;
	}

	const float HeldDuration = RuntimeState.StepElapsedTime - GetHoldStartTime();
	if (HeldDuration < RuntimeState.EffectiveHoldTime)
	{
		return false;
	}

	RuntimeState.StepProgress = 1.f;
	return AdvanceToNextStep();
}

void FQTERuntime::BeginCurrentStep(bool bBroadcastStepChanged)
{
	const FQTEStepDefinition* CurrentStep = GetCurrentStep();
	if (!CurrentStep)
	{
		OwnerComponent.FinishQTE(EQTEState::Failure);
		return;
	}

	FQTERuntimeState& RuntimeState = OwnerComponent.GetMutableRuntimeState();
	RuntimeState.Status = EQTEState::Running;
	RuntimeState.CompletedStepCount = FMath::Clamp(RuntimeState.CurrentStepIndex, 0, RuntimeState.TotalSteps);
	RuntimeState.StepElapsedTime = 0.f;
	RuntimeState.CurrentMashCount = 0;
	RuntimeState.bInputHeld = false;
	RuntimeState.FeedbackText = !CurrentStep->Feedback.PromptText.IsEmpty()
		? CurrentStep->Feedback.PromptText
		: CurrentStep->Presentation.Title;
	OwnerComponent.GetMutableHoldStartTime() = 0.f;
	RefreshRuntimeState();

	if (bBroadcastStepChanged)
	{
		OwnerComponent.BroadcastQTEStepChanged();
		OwnerComponent.BroadcastQTEUpdated();
	}
}

bool FQTERuntime::AdvanceToNextStep()
{
	FQTERuntimeState& RuntimeState = OwnerComponent.GetMutableRuntimeState();
	RuntimeState.CompletedStepCount = RuntimeState.CurrentStepIndex + 1;
	++RuntimeState.CurrentStepIndex;

	if (RuntimeState.CurrentStepIndex >= Definition.Steps.Num())
	{
		OwnerComponent.FinishQTE(EQTEState::Success);
		return true;
	}

	BeginCurrentStep(true);
	return true;
}

const FQTEStepDefinition* FQTERuntime::ResolveInputStep(const UInputAction* InputAction, bool bTreatUnexpectedPressedAsMistake)
{
	if (!InputAction)
	{
		return nullptr;
	}

	const FQTEStepDefinition* CurrentStep = GetCurrentStep();
	if (!CurrentStep)
	{
		return nullptr;
	}

	if (CurrentStep->InputAction != InputAction)
	{
		if (bTreatUnexpectedPressedAsMistake)
		{
			HandleMistake(CurrentStep->Feedback.FailureText);
		}

		return nullptr;
	}

	return CurrentStep;
}

void FQTERuntime::RefreshCurrentStepByType(const FQTEStepDefinition& Step)
{
	switch (Step.StepType)
	{
	case EQTEStepType::Hold:
		RefreshHoldStepProgress();
		break;
	case EQTEStepType::Mash:
		RefreshMashStepProgress();
		break;
	case EQTEStepType::Press:
	default:
		RefreshPressStepProgress();
		break;
	}
}

bool FQTERuntime::HandleCurrentStepPressedByType(const FQTEStepDefinition& Step)
{
	switch (Step.StepType)
	{
	case EQTEStepType::Hold:
		return StartHoldStep();
	case EQTEStepType::Mash:
		return AdvanceMashStep();
	case EQTEStepType::Press:
	default:
		return CompletePressStep();
	}
}

bool FQTERuntime::HandleCurrentStepReleasedByType(const FQTEStepDefinition& Step)
{
	return Step.StepType == EQTEStepType::Hold
		? ReleaseHoldStep(Step)
		: false;
}

bool FQTERuntime::HandleCurrentStepTriggeredByType(const FQTEStepDefinition& Step)
{
	return Step.StepType == EQTEStepType::Hold
		? UpdateHoldStep()
		: false;
}

void FQTERuntime::RefreshPressStepProgress()
{
	GetMutableRuntimeState().StepProgress = 0.f;
}

bool FQTERuntime::CompletePressStep()
{
	GetMutableRuntimeState().StepProgress = 1.f;
	return AdvanceToNextStep();
}

void FQTERuntime::RefreshHoldStepProgress()
{
	FQTERuntimeState& RuntimeState = GetMutableRuntimeState();
	if (!RuntimeState.bInputHeld)
	{
		RuntimeState.StepProgress = 0.f;
		return;
	}

	const float HeldDuration = RuntimeState.StepElapsedTime - GetHoldStartTime();
	RuntimeState.StepProgress = RuntimeState.EffectiveHoldTime > 0.f
		? FMath::Clamp(HeldDuration / RuntimeState.EffectiveHoldTime, 0.f, 1.f)
		: 1.f;
}

bool FQTERuntime::StartHoldStep()
{
	FQTERuntimeState& RuntimeState = GetMutableRuntimeState();
	if (!RuntimeState.bInputHeld)
	{
		RuntimeState.bInputHeld = true;
		GetHoldStartTime() = RuntimeState.StepElapsedTime;
		RuntimeState.StepProgress = 0.f;
		BroadcastUpdated();
	}

	return true;
}

bool FQTERuntime::ReleaseHoldStep(const FQTEStepDefinition& Step)
{
	FQTERuntimeState& RuntimeState = GetMutableRuntimeState();
	if (!RuntimeState.bInputHeld)
	{
		return true;
	}

	RuntimeState.bInputHeld = false;

	const float HeldDuration = RuntimeState.StepElapsedTime - GetHoldStartTime();
	if (HeldDuration + RuntimeState.EffectiveTolerance >= RuntimeState.EffectiveHoldTime)
	{
		RuntimeState.StepProgress = 1.f;
		return AdvanceToNextStep();
	}

	RuntimeState.StepProgress = RuntimeState.EffectiveHoldTime > 0.f
		? FMath::Clamp(HeldDuration / RuntimeState.EffectiveHoldTime, 0.f, 1.f)
		: 0.f;
	HandleMistake(Step.Feedback.FailureText);
	return true;
}

bool FQTERuntime::UpdateHoldStep()
{
	FQTERuntimeState& RuntimeState = GetMutableRuntimeState();
	if (!RuntimeState.bInputHeld)
	{
		return true;
	}

	const float HeldDuration = RuntimeState.StepElapsedTime - GetHoldStartTime();
	RuntimeState.StepProgress = RuntimeState.EffectiveHoldTime > 0.f
		? FMath::Clamp(HeldDuration / RuntimeState.EffectiveHoldTime, 0.f, 1.f)
		: 1.f;

	if (RuntimeState.StepProgress >= 1.f)
	{
		return AdvanceToNextStep();
	}

	BroadcastUpdated();
	return true;
}

void FQTERuntime::RefreshMashStepProgress()
{
	FQTERuntimeState& RuntimeState = GetMutableRuntimeState();
	RuntimeState.StepProgress = RuntimeState.EffectiveMashTarget > 0
		? FMath::Clamp(static_cast<float>(RuntimeState.CurrentMashCount) / static_cast<float>(RuntimeState.EffectiveMashTarget), 0.f, 1.f)
		: 1.f;
}

bool FQTERuntime::AdvanceMashStep()
{
	FQTERuntimeState& RuntimeState = GetMutableRuntimeState();
	++RuntimeState.CurrentMashCount;
	RuntimeState.StepProgress = RuntimeState.EffectiveMashTarget > 0
		? FMath::Clamp(static_cast<float>(RuntimeState.CurrentMashCount) / static_cast<float>(RuntimeState.EffectiveMashTarget), 0.f, 1.f)
		: 1.f;

	if (RuntimeState.CurrentMashCount >= RuntimeState.EffectiveMashTarget)
	{
		return AdvanceToNextStep();
	}

	BroadcastUpdated();
	return true;
}

const FQTEStepDefinition* FQTERuntime::GetCurrentStep() const
{
	return Definition.Steps.IsValidIndex(OwnerComponent.GetRuntimeState().CurrentStepIndex)
		? &Definition.Steps[OwnerComponent.GetRuntimeState().CurrentStepIndex]
		: nullptr;
}

void FQTERuntime::HandleMistake(const FText& FailureText)
{
	FQTERuntimeState& RuntimeState = OwnerComponent.GetMutableRuntimeState();
	++RuntimeState.Mistakes;
	RuntimeState.FeedbackText = FailureText;
	OwnerComponent.BroadcastQTEUpdated();

	if (Definition.Configuration.bFailOnAnyMistake || RuntimeState.Mistakes > Definition.Configuration.MaxMistakes)
	{
		OwnerComponent.FinishQTE(EQTEState::Failure, FailureText);
	}
}

FQTERuntimeState& FQTERuntime::GetMutableRuntimeState()
{
	return OwnerComponent.GetMutableRuntimeState();
}

const FQTERuntimeState& FQTERuntime::GetRuntimeState() const
{
	return OwnerComponent.GetRuntimeState();
}

float& FQTERuntime::GetHoldStartTime()
{
	return OwnerComponent.GetMutableHoldStartTime();
}

void FQTERuntime::BroadcastUpdated()
{
	OwnerComponent.BroadcastQTEUpdated();
}
