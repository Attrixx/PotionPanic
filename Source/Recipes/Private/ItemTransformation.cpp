#include "ItemTransformation.h"
#include "ActivityStepSettings.h"
#include "Misc/DataValidation.h"

#if WITH_EDITOR
EDataValidationResult UItemTransformation::IsDataValid(class FDataValidationContext& Context) const
{
	EDataValidationResult Result = Super::IsDataValid(Context);

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

	return Result;
}
#endif
