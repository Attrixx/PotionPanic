// Fill out your copyright notice in the Description page of Project Settings.

#include "ActivityConclusions/CompositeActivityConclusion.h"
#include <Misc/DataValidation.h>

EDataValidationResult UCompositeActivityConclusion::IsDataValid(FDataValidationContext& Context) const
{
	EDataValidationResult Result = Super::IsDataValid(Context);
	
	for (const UActivityConclusion* Conclusion : Conclusions)
	{
		if (!IsValid(Conclusion))
		{
			Context.AddError(FText::FromString("Invalid Composite Conclusion"));
			Result = EDataValidationResult::Invalid;
		}
		else
		{
			Result = CombineDataValidationResults(Conclusion->IsDataValid(Context), Result);
		}
	}
	
	return Result;
}

void UCompositeActivityConclusion::Conclude_Implementation(const FActivityExecutionState& ActivityState) const
{
	for (UActivityConclusion* Conclusion : Conclusions)
	{
		check(Conclusion);
		Conclusion->Conclude(ActivityState);
	}
}
