#include "ActivitySteps/TimeActivityStep.h"

DEFINE_LOG_CATEGORY_STATIC(MS_TimeActivityStep, Verbose, All);

UActivityStep* UTimeActivitySettings::CreateStep(UObject* Outer) const
{
	check(SecondsToWait >= 0.f);
	
	auto* Step = NewObject<UTimeActivityStep>(Outer);
	Step->SecondsToWait = SecondsToWait;
	return Step;
}

void UTimeActivityStep::StartActivity(const FActivityContext& Context)
{
	if (UWorld* World = GetOuter()->GetWorld())
	{
		FTimerHandle Handle;
		UE_LOG(MS_TimeActivityStep, Verbose, TEXT("Started waiting on Time Activity"));
		World->GetTimerManager().SetTimer(Handle, [Context]
		{
			FActivityOutput Output;
			UE_LOG(MS_TimeActivityStep, Verbose, TEXT("Stopped waiting on Time Activity"));			
			Output.ActivityResult = EActivityResult::Success;	
			Context.OnActivityFinished.Broadcast(Output);			
		}, SecondsToWait, false);
	}
}

