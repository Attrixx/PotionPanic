// Fill out your copyright notice in the Description page of Project Settings.


#include "ActivitySteps/ActivityStepTest.h"

DEFINE_LOG_CATEGORY_STATIC(MS_ActivityStepTest, Verbose, All);

UActivityStep* UTestActivitySetting::CreateStep(UObject* Outer) const
{
	return NewObject<UActivityStepTest>(Outer);
}

void UActivityStepTest::StartActivity(const FActivityContext& Context)
{
	UE_LOGFMT(MS_ActivityStepTest, Verbose, "Activity test, started!");
	
	FActivityOutput Output;
	Output.ActivityResult = EActivityResult::Success;	
	Context.OnActivityFinished.Broadcast(Output);
}

