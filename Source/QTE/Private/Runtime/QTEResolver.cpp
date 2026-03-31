#include "Runtime/QTEResolver.h"

#include "Core/QTEDefinitionDataAsset.h"

float FQTEResolver::GetDifficultyMultiplier(const UQTEDefinitionDataAsset* Definition)
{
	return Definition ? FMath::Max(0.1f, Definition->Configuration.DifficultyMultiplier) : 1.f;
}

float FQTEResolver::GetEffectiveGlobalTimeout(const UQTEDefinitionDataAsset* Definition)
{
	if (!Definition || Definition->Configuration.GlobalTimeoutSeconds <= 0.f)
	{
		return 0.f;
	}

	return Definition->Configuration.GlobalTimeoutSeconds / GetDifficultyMultiplier(Definition);
}

float FQTEResolver::GetEffectiveStepTimeout(const UQTEDefinitionDataAsset* Definition, const FQTEStepDefinition& Step)
{
	if (!Step.bUseStepTimeout || Step.StepTimeoutSeconds <= 0.f)
	{
		return 0.f;
	}

	return Step.StepTimeoutSeconds / GetDifficultyMultiplier(Definition);
}

float FQTEResolver::GetEffectiveTolerance(const UQTEDefinitionDataAsset* Definition, const FQTEStepDefinition& Step)
{
	const float BaseTolerance = Step.bOverrideTolerance
		? Step.ToleranceOverrideSeconds
		: Definition
			? Definition->Configuration.InputToleranceSeconds
			: 0.f;

	return BaseTolerance / GetDifficultyMultiplier(Definition);
}

float FQTEResolver::GetEffectiveHoldTime(const UQTEDefinitionDataAsset* Definition, const FQTEStepDefinition& Step)
{
	return Step.StepType == EQTEStepType::Hold
		? Step.RequiredHoldTimeSeconds * GetDifficultyMultiplier(Definition)
		: 0.f;
}

int32 FQTEResolver::GetEffectiveMashTarget(const UQTEDefinitionDataAsset* Definition, const FQTEStepDefinition& Step)
{
	return Step.StepType == EQTEStepType::Mash
		? FMath::Max(1, FMath::CeilToInt(static_cast<float>(Step.RequiredMashCount) * GetDifficultyMultiplier(Definition)))
		: 0;
}

const FQTEOutcomeConfiguration* FQTEResolver::GetOutcomeConfiguration(const UQTEDefinitionDataAsset* Definition, EQTEState FinalState)
{
	if (!Definition)
	{
		return nullptr;
	}

	switch (FinalState)
	{
		case EQTEState::Success:
			return &Definition->SuccessOutcome;
		case EQTEState::Failure:
			return &Definition->FailureOutcome;
		case EQTEState::Timeout:
			return &Definition->TimeoutOutcome;
		case EQTEState::Canceled:
			return &Definition->CanceledOutcome;
		case EQTEState::Interrupted:
			return &Definition->InterruptedOutcome;
		case EQTEState::None:
		case EQTEState::Running:
			return nullptr;
	}

	return nullptr;
}

EQTEGrade FQTEResolver::ResolveGrade(const FQTERuntimeState& RuntimeState, EQTEState FinalState)
{
	if (FinalState == EQTEState::Success)
	{
		return RuntimeState.Mistakes == 0 ? EQTEGrade::Perfect : EQTEGrade::Good;
	}

	if (FinalState == EQTEState::Failure || FinalState == EQTEState::Timeout)
	{
		return EQTEGrade::Fail;
	}

	return EQTEGrade::None;
}

FText FQTEResolver::ResolveOutcomeMessage(const UQTEDefinitionDataAsset* Definition, EQTEState FinalState, const FText& OverrideMessage)
{
	if (!OverrideMessage.IsEmpty())
	{
		return OverrideMessage;
	}

	if (const FQTEOutcomeConfiguration* OutcomeConfiguration = GetOutcomeConfiguration(Definition, FinalState))
	{
		return OutcomeConfiguration->Message;
	}

	return FText();
}

FQTEResult FQTEResolver::BuildResult(
	const UQTEDefinitionDataAsset* Definition,
	UObject* SourceObject,
	AActor* InstigatorActor,
	const FQTERuntimeState& RuntimeState,
	EQTEState FinalState,
	const FText& OverrideMessage)
{
	FQTEResult Result;
	Result.Definition = const_cast<UQTEDefinitionDataAsset*>(Definition);
	Result.SourceObject = SourceObject;
	Result.InstigatorActor = InstigatorActor;
	Result.QTEType = RuntimeState.QTEType;
	Result.Outcome = FinalState;
	Result.Grade = ResolveGrade(RuntimeState, FinalState);
	Result.CompletedStepCount = RuntimeState.CompletedStepCount;
	Result.FailedStepIndex = FinalState == EQTEState::Success ? INDEX_NONE : RuntimeState.CurrentStepIndex;
	Result.Mistakes = RuntimeState.Mistakes;
	Result.ElapsedTime = RuntimeState.ElapsedTime;
	Result.FinalRuntimeState = RuntimeState;
	Result.Message = ResolveOutcomeMessage(Definition, FinalState, OverrideMessage);
	return Result;
}

FQTEAuthorityResult FQTEResolver::BuildAuthorityResult(
	const UQTEDefinitionDataAsset* Definition,
	const FQTERuntimeState& RuntimeState,
	const FQTEResult& LastResult,
	EQTEState FallbackState)
{
	FQTEAuthorityResult AuthorityResult;

	if (RuntimeState.Status != EQTEState::Running && LastResult.Outcome != EQTEState::None)
	{
		AuthorityResult.Outcome = LastResult.Outcome;
		AuthorityResult.Grade = LastResult.Grade;
		AuthorityResult.CompletedStepCount = LastResult.CompletedStepCount;
		AuthorityResult.FailedStepIndex = LastResult.FailedStepIndex;
		AuthorityResult.Mistakes = LastResult.Mistakes;
		AuthorityResult.ElapsedTime = LastResult.ElapsedTime;
		AuthorityResult.Message = LastResult.Message;
		return AuthorityResult;
	}

	AuthorityResult.Outcome = FallbackState;
	AuthorityResult.Grade = ResolveGrade(RuntimeState, FallbackState);
	AuthorityResult.CompletedStepCount = RuntimeState.CompletedStepCount;
	AuthorityResult.FailedStepIndex = FallbackState == EQTEState::Success ? INDEX_NONE : RuntimeState.CurrentStepIndex;
	AuthorityResult.Mistakes = RuntimeState.Mistakes;
	AuthorityResult.ElapsedTime = RuntimeState.ElapsedTime;
	AuthorityResult.Message = ResolveOutcomeMessage(Definition, FallbackState, FText());
	return AuthorityResult;
}
