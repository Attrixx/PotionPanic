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

void UTimeActivityStep::StartActivity_Implementation(AActor* Instigator)
{
	if (UWorld* World = GetOuter()->GetWorld())
	{
		FTimerHandle Handle;
		UE_LOG(MS_TimeActivityStep, Verbose, TEXT("Started waiting on Time Activity"));
		World->GetTimerManager().SetTimer(Handle,
			[this]
			{
				FActivityOutput Output;
				UE_LOG(MS_TimeActivityStep, Verbose, TEXT("Stopped waiting on Time Activity"));
				Output.ActivityResult = EActivityResult::Success;
				FinishActivity(Output);
			},
			SecondsToWait,
			false);
	}
}
