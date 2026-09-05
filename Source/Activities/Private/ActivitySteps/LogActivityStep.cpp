// Fill out your copyright notice in the Description page of Project Settings.

#include "ActivitySteps/LogActivityStep.h"
#include "ActivityStepResult.h"
#include <Misc/DataValidation.h>

DEFINE_LOG_CATEGORY_STATIC(MS_LogActivityStep, Log, All);

#if WITH_EDITOR
EDataValidationResult ULogActivitySettings::IsDataValid(FDataValidationContext& Context) const
{
	EDataValidationResult Result = Super::IsDataValid(Context);

	if (Message.IsEmpty())
	{
		Context.AddWarning(FText::FromString("Field 'Message' is empty: the step will log nothing useful."));
	}

	return Result;
}
#endif

UActivityStep* ULogActivitySettings::CreateStep_Implementation(UObject* Outer) const
{
	auto* Step = NewObject<ULogActivityStep>(Outer);
	Step->Message = Message;
	return Step;
}

void ULogActivityStep::StartStep_Implementation(AActor* LastInstigator)
{
	UE_LOGFMT(MS_LogActivityStep, Log, "{0} (instigator '{1}', outer '{2}')",
		Message, GetNameSafe(LastInstigator), GetNameSafe(GetOuter()));

	FinishStep(FActivityStepResult{
		.Status = EActivityStepStatus::Success,
		.Score = 0,
	});
}
