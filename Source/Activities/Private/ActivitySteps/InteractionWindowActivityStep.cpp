// Fill out your copyright notice in the Description page of Project Settings.

#include "ActivitySteps/InteractionWindowActivityStep.h"
#include "ActivityExecutor.h"
#include "ActivityStepResult.h"
#include <Misc/DataValidation.h>

DEFINE_LOG_CATEGORY_STATIC(MS_InteractionWindowActivityStep, Verbose, All);

#if WITH_EDITOR
EDataValidationResult UInteractionWindowActivitySettings::IsDataValid(FDataValidationContext& Context) const
{
	EDataValidationResult Result = Super::IsDataValid(Context);

	if (DelayBeforeOpen < 0.f)
	{
		Context.AddError(FText::Format(FTextFormat::FromString("Field 'DelayBeforeOpen' should be positive (Current value: {0})."), DelayBeforeOpen));
		Result = EDataValidationResult::Invalid;
	}

	if (WindowDuration < 0.f)
	{
		Context.AddError(FText::Format(FTextFormat::FromString("Field 'WindowDuration' should be positive (Current value: {0})."), WindowDuration));
		Result = EDataValidationResult::Invalid;
	}

	if (WindowDuration == 0.f)
	{
		Context.AddWarning(FText::FromString("A zero 'WindowDuration' never closes: the step waits for an interact forever and can never fail."));
	}

	return Result;
}
#endif

UActivityStep* UInteractionWindowActivitySettings::CreateStep_Implementation(UObject* Outer) const
{
	check(DelayBeforeOpen >= 0.f && WindowDuration >= 0.f);

	auto* Step = NewObject<UInteractionWindowActivityStep>(Outer);
	Step->DelayBeforeOpen = DelayBeforeOpen;
	Step->WindowDuration = WindowDuration;
	return Step;
}

void UInteractionWindowActivityStep::StartStep_Implementation(AActor* LastInstigator)
{
	UE_LOGFMT(MS_InteractionWindowActivityStep, Verbose, "Started (delay: {0}, window: {1}).",
		DelayBeforeOpen, WindowDuration);

	bWindowOpen = false;

	// Published once, for the whole timeline: the opening is a time the clients already have, so
	// nothing has to go over the wire again when it arrives.
	PublishPresentation();

	if (DelayBeforeOpen <= 0.f)
	{
		OpenWindow();
		return;
	}

	if (UWorld* World = GetOuter()->GetWorld())
	{
		World->GetTimerManager().SetTimer(TimerHandle,
			[this] { OpenWindow(); },
			DelayBeforeOpen,
			false);
	}
	else
	{
		// No timer manager to open it: the step would sit closed forever, refusing every interact.
		UE_LOGFMT(MS_InteractionWindowActivityStep, Warning, "No world to arm the opening delay on. Opening now.");
		OpenWindow();
	}
}

void UInteractionWindowActivityStep::OnInteract_Implementation(AActor* Instigator)
{
	if (!IsValid(Instigator))
	{
		// Interact carries a null instigator when something other than a player drives the
		// activity: that is not the interact this window waits for.
		return;
	}

	if (!bWindowOpen)
	{
		UE_LOGFMT(MS_InteractionWindowActivityStep, Verbose, "'{0}' interacted before the window opened, ignored.",
			GetNameSafe(Instigator));
		return;
	}

	UE_LOGFMT(MS_InteractionWindowActivityStep, Verbose, "'{0}' interacted during the window.", GetNameSafe(Instigator));

	ClearTimer();
	if (UActivityExecutor* Executor = GetExecutor())
	{
		Executor->ClearStepPresentation();
	}

	FinishStep(FActivityStepResult{
		.Status = EActivityStepStatus::Success,
		.Score = 0, // TODO: Fill this field
	});
}

void UInteractionWindowActivityStep::CancelStep_Implementation()
{
	ClearTimer();
	if (UActivityExecutor* Executor = GetExecutor())
	{
		Executor->ClearStepPresentation();
	}
}

void UInteractionWindowActivityStep::OpenWindow()
{
	UE_LOGFMT(MS_InteractionWindowActivityStep, Verbose, "Window opened for {0}s.", WindowDuration);

	// No publish: the clients were told when this would happen back when the step started, and
	// they cross that time on their own.
	bWindowOpen = true;

	if (WindowDuration <= 0.f)
		return;

	if (UWorld* World = GetOuter()->GetWorld())
	{
		World->GetTimerManager().SetTimer(TimerHandle,
			[this]
			{
				UE_LOGFMT(MS_InteractionWindowActivityStep, Verbose, "Window missed.");

				if (UActivityExecutor* Executor = GetExecutor())
				{
					Executor->ClearStepPresentation();
				}

				FinishStep(FActivityStepResult{
					.Status = EActivityStepStatus::Fail,
					.Score = 0, // TODO: Fill this field
				});
			},
			WindowDuration,
			false);
	}
	else
	{
		UE_LOGFMT(MS_InteractionWindowActivityStep, Warning, "No world to arm the closing timer on. The window will never close.");
	}
}

void UInteractionWindowActivityStep::PublishPresentation()
{
	UActivityExecutor* Executor = GetExecutor();
	if (!Executor)
		return;

	const double Now = GetServerTimeSeconds();
	const double OpensAt = Now + DelayBeforeOpen;

	FActivityStepPresentation NewPresentation;
	NewPresentation.Instigator = Executor->GetExecutionState().LastInstigator.Get();
	NewPresentation.StartServerTime = Now;
	NewPresentation.OpenServerTime = OpensAt;

	// Equal to the opening time when the window never closes, which is what says "no deadline".
	NewPresentation.CloseServerTime = WindowDuration > 0.f ? OpensAt + WindowDuration : OpensAt;

	Executor->SetStepPresentation(this, NewPresentation);
}

void UInteractionWindowActivityStep::ClearTimer()
{
	if (UWorld* World = GetOuter()->GetWorld())
	{
		World->GetTimerManager().ClearTimer(TimerHandle);
	}
}
