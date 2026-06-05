#include "Core/QTEDefinitionDataAsset.h"
#include "InputAction.h"
#include "Misc/DataValidation.h"

namespace
{
TSubclassOf<UQTERuntimeSettings> GetRuntimeSettingsClass(EQTEType QTEType)
{
	switch (QTEType)
	{
	case EQTEType::Hold:
		return UQTEHoldRuntimeSettings::StaticClass();
	case EQTEType::Mash:
		return UQTEMashRuntimeSettings::StaticClass();
	case EQTEType::Sequence:
		return UQTESequenceRuntimeSettings::StaticClass();
	case EQTEType::Chain:
		return UQTEChainRuntimeSettings::StaticClass();
	case EQTEType::Press:
	default:
		return UQTEPressRuntimeSettings::StaticClass();
	}
}
}

void UQTEDefinitionDataAsset::PostLoad()
{
	Super::PostLoad();
	EnsureRuntimeSettings();
}

void UQTEDefinitionDataAsset::EnsureRuntimeSettings()
{
	if (RuntimeSettings)
	{
		return;
	}

	RuntimeSettings = NewObject<UQTERuntimeSettings>(this, GetRuntimeSettingsClass(QTEType), NAME_None, RF_Transactional);
}

FText UQTEDefinitionDataAsset::GetResolvedDisplayName() const
{
	if (!DisplayName.IsEmpty())
	{
		return DisplayName;
	}

	if (!Presentation.Title.IsEmpty())
	{
		return Presentation.Title;
	}

	return FText::FromString(TEXT("QTE"));
}

FText UQTEDefinitionDataAsset::GetResolvedStepLabel(int32 StepIndex) const
{
	if (!Steps.IsValidIndex(StepIndex))
	{
		return FText::GetEmpty();
	}

	const FQTEStepDefinition& Step = Steps[StepIndex];
	if (!Step.Presentation.Title.IsEmpty())
	{
		return Step.Presentation.Title;
	}

	if (!Step.Feedback.PromptText.IsEmpty())
	{
		return Step.Feedback.PromptText;
	}

	if (Step.InputAction)
	{
		FString ActionName = Step.InputAction->GetName();
		ActionName.RemoveFromStart(TEXT("IA_"));
		ActionName.ReplaceInline(TEXT("_"), TEXT(" "));
		return FText::FromString(ActionName);
	}

	return FText::Format(FText::FromString(TEXT("Step {0}")), FText::AsNumber(StepIndex + 1));
}

EQTEType UQTEDefinitionDataAsset::GetQTEType() const
{
	return RuntimeSettings ? RuntimeSettings->GetQTEType() : QTEType;
}

#if WITH_EDITOR
EDataValidationResult UQTEDefinitionDataAsset::IsDataValid(FDataValidationContext& Context) const
{
	EDataValidationResult Result = Super::IsDataValid(Context);

	if (!RuntimeSettings)
	{
		Context.AddError(FText::FromString("QTE definitions must have RuntimeSettings."));
		Result = EDataValidationResult::Invalid;
	}

	if (Steps.IsEmpty())
	{
		Context.AddError(FText::FromString("QTE definitions must contain at least one step."));
		Result = EDataValidationResult::Invalid;
	}

	if (Configuration.GlobalTimeoutSeconds < 0.f)
	{
		Context.AddError(FText::FromString("GlobalTimeoutSeconds must be positive or zero."));
		Result = EDataValidationResult::Invalid;
	}

	if (Configuration.InputToleranceSeconds < 0.f)
	{
		Context.AddError(FText::FromString("InputToleranceSeconds must be positive or zero."));
		Result = EDataValidationResult::Invalid;
	}

	if (Configuration.MaxMistakes < 0)
	{
		Context.AddError(FText::FromString("MaxMistakes must be positive or zero."));
		Result = EDataValidationResult::Invalid;
	}

	if (Configuration.DifficultyMultiplier < 0.1f)
	{
		Context.AddError(FText::FromString("DifficultyMultiplier must be greater than or equal to 0.1."));
		Result = EDataValidationResult::Invalid;
	}

	if (RuntimeSettings)
	{
		switch (RuntimeSettings->GetQTEType())
		{
		case EQTEType::Press:
		case EQTEType::Hold:
		case EQTEType::Mash:
		{
			if (Steps.Num() != 1)
			{
				Context.AddError(FText::FromString("Press, Hold, and Mash QTE definitions must contain exactly one step."));
				Result = EDataValidationResult::Invalid;
			}
			break;
		}
		case EQTEType::Sequence:
		case EQTEType::Chain:
		{
			if (Steps.Num() < 2)
			{
				Context.AddError(FText::FromString("Sequence and Chain QTE definitions must contain at least two steps."));
				Result = EDataValidationResult::Invalid;
			}
			break;
		}
		}
	}

	for (int32 StepIndex = 0; StepIndex < Steps.Num(); ++StepIndex)
	{
		const FQTEStepDefinition& Step = Steps[StepIndex];

		if (!Step.InputAction)
		{
			Context.AddError(FText::Format(FText::FromString("Step {0} is missing an InputAction."), FText::AsNumber(StepIndex)));
			Result = EDataValidationResult::Invalid;
		}

		if (Step.bUseStepTimeout && Step.StepTimeoutSeconds < 0.f)
		{
			Context.AddError(FText::Format(FText::FromString("Step {0} StepTimeoutSeconds must be positive or zero."), FText::AsNumber(StepIndex)));
			Result = EDataValidationResult::Invalid;
		}

		if (Step.RequiredHoldTimeSeconds < 0.f)
		{
			Context.AddError(FText::Format(FText::FromString("Step {0} RequiredHoldTimeSeconds must be positive or zero."), FText::AsNumber(StepIndex)));
			Result = EDataValidationResult::Invalid;
		}

		if (Step.StepType == EQTEStepType::Hold && Step.RequiredHoldTimeSeconds <= 0.f)
		{
			Context.AddError(FText::Format(FText::FromString("Step {0} is a Hold step and must have RequiredHoldTimeSeconds greater than zero."),
				FText::AsNumber(StepIndex)));
			Result = EDataValidationResult::Invalid;
		}

		if (Step.RequiredMashCount < 1)
		{
			Context.AddError(FText::Format(FText::FromString("Step {0} RequiredMashCount must be greater than zero."), FText::AsNumber(StepIndex)));
			Result = EDataValidationResult::Invalid;
		}

		if (Step.StepType == EQTEStepType::Mash && Step.RequiredMashCount < 2)
		{
			Context.AddWarning(FText::Format(
				FText::FromString("Step {0} is a Mash step with a very low RequiredMashCount. Consider using a Press step instead."),
				FText::AsNumber(StepIndex)));
		}

		if (Step.bOverrideTolerance && Step.ToleranceOverrideSeconds < 0.f)
		{
			Context.AddError(FText::Format(FText::FromString("Step {0} ToleranceOverrideSeconds must be positive or zero."), FText::AsNumber(StepIndex)));
			Result = EDataValidationResult::Invalid;
		}

		if (Step.StepType == EQTEStepType::Hold || Step.StepType == EQTEStepType::Mash)
		{
			const bool bHasGlobalTimeout = Configuration.GlobalTimeoutSeconds > 0.f;
			const bool bHasStepTimeout = Step.bUseStepTimeout && Step.StepTimeoutSeconds > 0.f;
			if (!bHasGlobalTimeout && !bHasStepTimeout)
			{
				Context.AddWarning(FText::Format(
					FText::FromString("Step {0} has no timeout configured. This QTE can run indefinitely if the player never completes it."),
					FText::AsNumber(StepIndex)));
			}
		}
	}

	if (Steps.Num() == 1)
	{
		const EQTEStepType OnlyStepType = Steps[0].StepType;
		const bool bMatchesType =
			(GetQTEType() == EQTEType::Press && OnlyStepType == EQTEStepType::Press) ||
			(GetQTEType() == EQTEType::Hold && OnlyStepType == EQTEStepType::Hold) ||
			(GetQTEType() == EQTEType::Mash && OnlyStepType == EQTEStepType::Mash) ||
			(GetQTEType() == EQTEType::Sequence) ||
			(GetQTEType() == EQTEType::Chain);

		if (!bMatchesType)
		{
			Context.AddError(FText::FromString("Single-step runtime settings must match the first step type."));
			Result = EDataValidationResult::Invalid;
		}
	}

	if (DisplayName.IsEmpty() && Presentation.Title.IsEmpty())
	{
		Context.AddWarning(FText::FromString("QTE asset has no DisplayName or Presentation.Title. UI will fall back to a generic label."));
	}

	return Result;
}
#endif

float UQTEDefinitionDataAsset::GetDifficultyMultiplier()
{
	return FMath::Max(0.1f, Configuration.DifficultyMultiplier);
}

float UQTEDefinitionDataAsset::GetEffectiveGlobalTimeout()
{
	if (Configuration.GlobalTimeoutSeconds <= 0.f)
	{
		return 0.f;
	}

	return Configuration.GlobalTimeoutSeconds / GetDifficultyMultiplier();
}

float UQTEDefinitionDataAsset::GetEffectiveStepTimeout(const FQTEStepDefinition& Step)
{
	if (!Step.bUseStepTimeout || Step.StepTimeoutSeconds <= 0.f)
	{
		return 0.f;
	}

	return Step.StepTimeoutSeconds / GetDifficultyMultiplier();
}

float UQTEDefinitionDataAsset::GetEffectiveTolerance(const FQTEStepDefinition& Step)
{
	const float BaseTolerance = Step.bOverrideTolerance
		? Step.ToleranceOverrideSeconds
		: Configuration.InputToleranceSeconds;

	return BaseTolerance / GetDifficultyMultiplier();
}

float UQTEDefinitionDataAsset::GetEffectiveHoldTime(const FQTEStepDefinition& Step)
{
	return Step.StepType == EQTEStepType::Hold
		? Step.RequiredHoldTimeSeconds * GetDifficultyMultiplier()
		: 0.f;
}

int32 UQTEDefinitionDataAsset::GetEffectiveMashTarget(const FQTEStepDefinition& Step)
{
	return Step.StepType == EQTEStepType::Mash
		? FMath::Max(1, FMath::CeilToInt(static_cast<float>(Step.RequiredMashCount) * GetDifficultyMultiplier()))
		: 0;
}

const FQTEOutcomeConfiguration* UQTEDefinitionDataAsset::GetOutcomeConfiguration(EQTEState FinalState)
{
	switch (FinalState)
	{
	case EQTEState::Success:
		return &SuccessOutcome;
	case EQTEState::Failure:
		return &FailureOutcome;
	case EQTEState::Timeout:
		return &TimeoutOutcome;
	case EQTEState::Canceled:
		return &CanceledOutcome;
	case EQTEState::Interrupted:
		return &InterruptedOutcome;
	case EQTEState::None:
	case EQTEState::Running:
		return nullptr;
	}

	return nullptr;
}

EQTEGrade UQTEDefinitionDataAsset::ResolveGrade(const FQTERuntimeState& RuntimeState, EQTEState FinalState)
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

FText UQTEDefinitionDataAsset::ResolveOutcomeMessage(EQTEState FinalState, const FText& OverrideMessage)
{
	if (!OverrideMessage.IsEmpty())
	{
		return OverrideMessage;
	}

	if (const FQTEOutcomeConfiguration* OutcomeConfiguration = GetOutcomeConfiguration(FinalState))
	{
		return OutcomeConfiguration->Message;
	}

	return FText();
}

FQTEResult UQTEDefinitionDataAsset::BuildResult(

	UObject* SourceObject,
	AActor* InstigatorActor,
	const FQTERuntimeState& RuntimeState,
	EQTEState FinalState,
	const FText& OverrideMessage)
{
	FQTEResult Result;
	Result.Definition = this;
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
	Result.Message = ResolveOutcomeMessage(FinalState, OverrideMessage);
	return Result;
}

FQTEAuthorityResult UQTEDefinitionDataAsset::BuildAuthorityResult(

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
	AuthorityResult.Message = ResolveOutcomeMessage(FallbackState, FText());
	return AuthorityResult;
}
