#include "ActivityAsset.h"
#include "ActivityConclusion.h"
#include "ActivityEvaluator.h"
#include "ActivityStepSettings.h"
#include "ActivityTags.h"
#include "ItemTags.h"
#include <Misc/DataValidation.h>

#if WITH_EDITOR
EDataValidationResult UActivityAsset::IsDataValid(FDataValidationContext& Context) const
{
	EDataValidationResult Result = Super::IsDataValid(Context);

	if (!InputTags.HasTag(GameTags::Activity))
	{
		Context.AddError(FText::FromString("Input tags must have at least one activity tag."));
		Result = EDataValidationResult::Invalid;
	}

	if (!InputTags.HasTag(GameTags::Item))
	{
		Context.AddError(FText::FromString(FString::Format(
			TEXT("Input tags must have at least one item tag. Use {0} if you wish to not take any item as input."),
			{GameTags::Item_None.GetTag().ToString()})));
		Result = EDataValidationResult::Invalid;
	}

	if (!SecondaryInputTags.IsEmpty())
	{
		if (SecondaryInputTags.HasTag(GameTags::Activity))
		{
			Context.AddError(FText::FromString(
				"Secondary input tags describe an item carried by the instigator, so they cannot hold an activity tag."));
			Result = EDataValidationResult::Invalid;
		}

		if (!SecondaryInputTags.HasTag(GameTags::Item))
		{
			Context.AddError(FText::FromString("Secondary input tags must have at least one item tag."));
			Result = EDataValidationResult::Invalid;
		}

		if (SecondaryInputTags.HasTag(GameTags::Item_None))
		{
			Context.AddError(FText::FromString(FString::Format(
				TEXT("{0} means no item at all, which cannot describe a carried ingredient. Leave SecondaryInputTags empty for a single-item activity."),
				{GameTags::Item_None.GetTag().ToString()})));
			Result = EDataValidationResult::Invalid;
		}
	}

	for (const UActivityStepSettings* Settings : ActivitySteps)
	{
		if (IsValid(Settings))
		{
			Result = CombineDataValidationResults(Settings->IsDataValid(Context), Result);
		}
		else
		{
			Context.AddError(FText::FromString("Invalid Activity Step."));
			Result = EDataValidationResult::Invalid;
		}
	}

	if (ActivitySteps.Num() > 0)
	{
		if (IsValid(Evaluator))
		{
			const auto* EvaluatorPtr = Evaluator.Get();
			Result = CombineDataValidationResults(EvaluatorPtr->IsDataValid(Context), Result);
		}
		else
		{
			Context.AddError(FText::FromString("Invalid Activity Evaluator."));
			Result = EDataValidationResult::Invalid;
		}
	}

	if (IsValid(Conclusion))
	{
		const auto* ConclusionPtr = Conclusion.Get();
		Result = CombineDataValidationResults(ConclusionPtr->IsDataValid(Context), Result);
	}
	else
	{
		Context.AddError(FText::FromString("Invalid Activity Conclusion."));
		Result = EDataValidationResult::Invalid;
	}

	return Result;
}
#endif
