// Fill out your copyright notice in the Description page of Project Settings.

#include "ActivitySteps/IFTActivityStep.h"

DEFINE_LOG_CATEGORY_STATIC(MS_ITFActivityStep, Verbose, All);

UActivityStep* UIFTActivitySetting::CreateStep(UObject* Outer) const
{
	check(SecondsBeforeWindow >= 0.f);
	check(WindowLengthSeconds >= 0.f);
	
	auto* Step = NewObject<UIFTActivityStep>(Outer);
	Step->SecondsBeforeWindow = SecondsBeforeWindow;
	Step->WindowLengthSeconds = WindowLengthSeconds;
	return Step;
}

void UIFTActivityStep::StartActivity(const FActivityContext& Context)
{
	ActivityContext = Context;
	if (UWorld* World = GetOuter()->GetWorld())
	{
		FTimerHandle Handle;
		Status = EIFTStatus::WaitingForWindow;
		UE_LOGFMT(MS_ITFActivityStep, Verbose, "Waiting for Window opening");
		
		World->GetTimerManager().SetTimer(Handle, [&, World]
		{
			Status = EIFTStatus::DuringWindow;
			UE_LOGFMT(MS_ITFActivityStep, Verbose, "Window opened, Waiting for User Activity");
			
			World->GetTimerManager().SetTimer(WindowHandle, [&]
			{
				Status = EIFTStatus::PastWindow;
				UE_LOGFMT(MS_ITFActivityStep, Verbose, "Missed Window");				
				ActivityOutput.ActivityResult = EActivityResult::Fail;
				// TODO FranÃ§ois Compute Activity score
				ActivityContext.OnActivityFinished.Broadcast(ActivityOutput);
			}, WindowLengthSeconds, false);
			
		}, SecondsBeforeWindow, false);
	}
}

void UIFTActivityStep::InteractWhileProcess()
{
	Super::InteractWhileProcess();
	
	if (Status == EIFTStatus::DuringWindow)
	{
		UE_LOGFMT(MS_ITFActivityStep, Verbose, "Window Activity triggered successfully");
		GetWorld()->GetTimerManager().ClearTimer(WindowHandle);
		ActivityOutput.ActivityResult = EActivityResult::Success;		
		ActivityContext.OnActivityFinished.Broadcast(ActivityOutput);
	}
}

