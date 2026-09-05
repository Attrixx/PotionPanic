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

	if (!StationItemTags.HasTag(GameTags::Item))
	{
		Context.AddError(FText::FromString(FString::Format(
			TEXT("Station item tags must have at least one item tag. Use {0} if the activity runs on an empty station."),
			{GameTags::Item_None.GetTag().ToString()})));
		Result = EDataValidationResult::Invalid;
	}

	if (!InstigatorItemTags.IsEmpty() && !InstigatorItemTags.HasTag(GameTags::Item))
	{
		Context.AddError(FText::FromString("Instigator item tags is not empty but holds no item tag."));
		Result = EDataValidationResult::Invalid;
	}

	if (TakeFromInstigator != EActivityTakeFromInstigator::Never && !InstigatorItemTags.IsEmpty())
	{
		// A taken item is matched against StationItemTags: it is the station's item by the time the
		// activity runs, and the instigator is left empty-handed.
		Context.AddError(FText::FromString(
			"Taking the instigator's item requires an empty InstigatorItemTags: the taken item is matched against StationItemTags."));
		Result = EDataValidationResult::Invalid;
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
