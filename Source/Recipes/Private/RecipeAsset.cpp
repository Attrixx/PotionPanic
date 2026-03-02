#include "RecipeAsset.h"
#include "ItemTransformation.h"
#include "Misc/DataValidation.h"

bool URecipeDataAsset::IsRecipeDefinitionValid(FText& OutFailureReason) const
{
	OutFailureReason = FText::GetEmpty();

	if (FailureOutputItem == nullptr && FailureOutputQuantity != 1)
	{
		OutFailureReason = FText::FromString(TEXT("FailureOutputQuantity must stay at 1 when FailureOutputItem is not configured."));
		return false;
	}

	if (FailureOutputItem != nullptr && FailureOutputQuantity < 1)
	{
		OutFailureReason = FText::FromString(TEXT("FailureOutputQuantity must be >= 1."));
		return false;
	}

	if (Steps.Num() == 0)
	{
		OutFailureReason = FText::FromString(TEXT("Recipe has no steps."));
		return false;
	}

	for (int32 StepIndex = 0; StepIndex < Steps.Num(); ++StepIndex)
	{
		const UItemTransformation* Step = Steps[StepIndex];
		if (Step == nullptr)
		{
			OutFailureReason = FText::Format(
				FText::FromString(TEXT("Recipe step {0} is null.")),
				StepIndex);
			return false;
		}

		FText StepFailureReason;
		if (!Step->IsStepDefinitionValid(StepFailureReason))
		{
			OutFailureReason = FText::Format(
				FText::FromString(TEXT("Recipe step {0} is invalid: {1}")),
				StepIndex,
				StepFailureReason);
			return false;
		}
	}

	return true;
}

#if WITH_EDITOR
EDataValidationResult URecipeDataAsset::IsDataValid(FDataValidationContext& Context) const
{
	EDataValidationResult Result = EDataValidationResult::Valid;
	const EDataValidationResult ParentResult = Super::IsDataValid(Context);

	FText FailureReason;
	if (!IsRecipeDefinitionValid(FailureReason))
	{
		Context.AddError(FailureReason);
		Result = EDataValidationResult::Invalid;
	}

	return CombineDataValidationResults(ParentResult, Result);
}
#endif
