#include "ItemTransformation.h"
#include "Misc/DataValidation.h"

bool UItemTransformation::HasInputConstraints() const
{
	return InputItem != nullptr
		|| RequiredItemDataTags.Num() > 0
		|| RequiredTransformationFlags.Num() > 0
		|| bRequireProcessedIngredient
		|| RequiredIngredientStateFlags.Num() > 0
		|| RequiredIngredientStateTags.Num() > 0;
}

bool UItemTransformation::IsStepDefinitionValid(FText& OutFailureReason) const
{
	OutFailureReason = FText::GetEmpty();

	if (InputQuantity < 1)
	{
		OutFailureReason = FText::FromString(TEXT("Step input quantity must be >= 1."));
		return false;
	}

	if (OutputQuantity < 1)
	{
		OutFailureReason = FText::FromString(TEXT("Step output quantity must be >= 1."));
		return false;
	}

	if (!HasInputConstraints())
	{
		OutFailureReason = FText::FromString(TEXT("Step has no input constraints."));
		return false;
	}

	if (InputItem == nullptr)
	{
		OutFailureReason = FText::FromString(TEXT("Step has no required input item."));
		return false;
	}

	if (Activity == nullptr)
	{
		OutFailureReason = FText::FromString(TEXT("Step has no activity."));
		return false;
	}

	if (InteractionDefinition == nullptr)
	{
		OutFailureReason = FText::FromString(TEXT("Step has no interaction definition (QTE/IFT)."));
		return false;
	}

	if (OutputItem == nullptr)
	{
		OutFailureReason = FText::FromString(TEXT("Step has no output item."));
		return false;
	}

	return true;
}

#if WITH_EDITOR
EDataValidationResult UItemTransformation::IsDataValid(FDataValidationContext& Context) const
{
	EDataValidationResult Result = EDataValidationResult::Valid;
	const EDataValidationResult ParentResult = Super::IsDataValid(Context);

	FText FailureReason;
	if (!IsStepDefinitionValid(FailureReason))
	{
		Context.AddError(FailureReason);
		Result = EDataValidationResult::Invalid;
	}

	return CombineDataValidationResults(ParentResult, Result);
}
#endif
