#include "ActivitySteps/TimeActivityStep.h"
#include "Misc/DataValidation.h"

DEFINE_LOG_CATEGORY_STATIC(MS_TimeActivityStep, Verbose, All);

#if WITH_EDITOR
EDataValidationResult UTimeActivitySettings::IsDataValid(FDataValidationContext& Context) const
{
	EDataValidationResult Result = Super::IsDataValid(Context);

	if (SecondsToWait < 0.f)
	{
		Context.AddError(FText::Format(FTextFormat::FromString("Field 'SecondsToWait' should be positive (Current value: {0})."), SecondsToWait));
		Result = EDataValidationResult::Invalid;
	}

	return Result;
}
#endif

UActivityStep* UTimeActivitySettings::CreateStep_Implementation(UObject* Outer) const
{
	check(SecondsToWait >= 0.f);

	auto* Step = NewObject<UTimeActivityStep>(Outer);
	Step->SecondsToWait = SecondsToWait;
	return Step;
}

void UTimeActivityStep::StartStep_Implementation(AActor* Performer)
{
	if (UWorld* World = GetOuter()->GetWorld())
	{
		UE_LOG(MS_TimeActivityStep, Verbose, TEXT("Started waiting on Time Activity"));
		World->GetTimerManager().SetTimer(TimerHandle,
			[this]
			{
				UE_LOG(MS_TimeActivityStep, Verbose, TEXT("Stopped waiting on Time Activity"));
				FinishStep(FActivityStepResult{
					.Status = EActivityStepStatus::Success,
					.Score = 0, // TODO: Fill this field
				});
			},
			SecondsToWait,
			false);
	}
}

void UTimeActivityStep::CancelStep_Implementation()
{
	if (UWorld* World = GetOuter()->GetWorld())
	{
		UE_LOG(MS_TimeActivityStep, Verbose, TEXT("Time Activity canceled"));
		World->GetTimerManager().ClearTimer(TimerHandle);
	}
}
