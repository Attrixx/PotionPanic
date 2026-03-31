// Fill out your copyright notice in the Description page of Project Settings.

#include "ActivitySteps/IFTActivityStep.h"
#include "ActivityStepResult.h"
#include <Misc/DataValidation.h>

DEFINE_LOG_CATEGORY_STATIC(MS_ITFActivityStep, Verbose, All);

#if WITH_EDITOR
EDataValidationResult UIFTActivitySetting::IsDataValid(FDataValidationContext& Context) const
{
	EDataValidationResult Result = Super::IsDataValid(Context);

	if (SecondsBeforeWindow < 0.f)
	{
		Context.AddError(FText::Format(FTextFormat::FromString("Field 'SecondsBeforeWindow' should be positive (Current value: {0})."), SecondsBeforeWindow));
		Result = EDataValidationResult::Invalid;
	}

	if (WindowLengthSeconds < 0.f)
	{
		Context.AddError(FText::Format(FTextFormat::FromString("Field 'WindowLengthSeconds' should be positive (Current value: {0})."), WindowLengthSeconds));
		Result = EDataValidationResult::Invalid;
	}

	return Result;
}
#endif

UActivityStep* UIFTActivitySetting::CreateStep_Implementation(UObject* Outer) const
{
	check(SecondsBeforeWindow >= 0.f);
	check(WindowLengthSeconds >= 0.f);

	auto* Step = NewObject<UIFTActivityStep>(Outer);
	Step->SecondsBeforeWindow = SecondsBeforeWindow;
	Step->WindowLengthSeconds = WindowLengthSeconds;
	return Step;
}

void UIFTActivityStep::StartStep_Implementation(AActor* LastInstigator)
{
	if (UWorld* World = GetOuter()->GetWorld())
	{
		Status = EIFTStatus::WaitingForWindow;
		UE_LOGFMT(MS_ITFActivityStep, Verbose, "Waiting for Window opening");

		World->GetTimerManager().SetTimer(WindowHandle,
			[&, World]
			{
				Status = EIFTStatus::DuringWindow;
				UE_LOGFMT(MS_ITFActivityStep, Verbose, "Window opened, Waiting for User Activity");

				World->GetTimerManager().SetTimer(WindowHandle,
					[&]
					{
						Status = EIFTStatus::PastWindow;
						UE_LOGFMT(MS_ITFActivityStep, Verbose, "Missed Window");

						FinishStep(FActivityStepResult{
							.Status = EActivityStepStatus::Fail,
							.Score = 0, // TODO: Fill this field
						});
					},
					WindowLengthSeconds,
					false);
			},
			SecondsBeforeWindow,
			false);
	}
}

void UIFTActivityStep::OnInteract_Implementation(AActor* Instigator)
{
	if (Status == EIFTStatus::DuringWindow)
	{
		UE_LOGFMT(MS_ITFActivityStep, Verbose, "Window Activity triggered successfully");

		if (UWorld* World = GetOuter()->GetWorld())
		{
			World->GetTimerManager().ClearTimer(WindowHandle);
		}

		FinishStep(FActivityStepResult{
			.Status = EActivityStepStatus::Success,
			.Score = 0, // TODO: Fill this field
		});
	}
}

void UIFTActivityStep::CancelStep_Implementation()
{
	if (UWorld* World = GetOuter()->GetWorld())
	{
		World->GetTimerManager().ClearTimer(WindowHandle);
	}
}
