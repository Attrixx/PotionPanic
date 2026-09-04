// Fill out your copyright notice in the Description page of Project Settings.

#include "ActivitySteps/WindowInteractionActivityStep.h"
#include "ActivityStepResult.h"
#include <Misc/DataValidation.h>

DEFINE_LOG_CATEGORY_STATIC(MS_WindowInteractionActivityStep, Verbose, All);

#if WITH_EDITOR
EDataValidationResult UWindowInteractionActivitySettings::IsDataValid(FDataValidationContext& Context) const
{
	EDataValidationResult Result = Super::IsDataValid(Context);

	if (TimeoutSeconds < 0.f)
	{
		Context.AddError(FText::Format(FTextFormat::FromString("Field 'TimeoutSeconds' should be positive (Current value: {0})."), TimeoutSeconds));
		Result = EDataValidationResult::Invalid;
	}

	if (TimeoutSeconds > 0.f && !bAlwaysWaitForInteract)
	{
		Context.AddWarning(FText::FromString("The timeout is nearly unreachable with 'bAlwaysWaitForInteract' off: the step succeeds on start whenever an instigator is already there."));
	}

	return Result;
}
#endif

UActivityStep* UWindowInteractionActivitySettings::CreateStep_Implementation(UObject* Outer) const
{
	check(TimeoutSeconds >= 0.f);

	auto* Step = NewObject<UWindowInteractionActivityStep>(Outer);
	Step->bAlwaysWaitForInteract = bAlwaysWaitForInteract;
	Step->TimeoutSeconds = TimeoutSeconds;
	return Step;
}

void UWindowInteractionActivityStep::StartStep_Implementation(AActor* LastInstigator)
{
	if (!bAlwaysWaitForInteract && IsValid(LastInstigator))
	{
		UE_LOGFMT(MS_WindowInteractionActivityStep, Verbose, "Instigator '{0}' is already there, window skipped.",
			GetNameSafe(LastInstigator));

		FinishStep(FActivityStepResult{
			.Status = EActivityStepStatus::Success,
			.Score = 0, // TODO: Fill this field
		});
		return;
	}

	UE_LOGFMT(MS_WindowInteractionActivityStep, Verbose, "Window opened (timeout: {0}).", TimeoutSeconds);

	StartTimeout();
}

void UWindowInteractionActivityStep::OnInteract_Implementation(AActor* Instigator)
{
	if (!IsValid(Instigator))
	{
		// Interact carries a null instigator when the activity is driven by something other than a
		// player: that is not the interact this window waits for.
		UE_LOGFMT(MS_WindowInteractionActivityStep, Verbose, "Interact without instigator ignored.");
		return;
	}

	UE_LOGFMT(MS_WindowInteractionActivityStep, Verbose, "Instigator '{0}' interacted during the window.",
		GetNameSafe(Instigator));

	ClearTimeout();

	FinishStep(FActivityStepResult{
		.Status = EActivityStepStatus::Success,
		.Score = 0, // TODO: Fill this field
	});
}

void UWindowInteractionActivityStep::CancelStep_Implementation()
{
	ClearTimeout();
}

void UWindowInteractionActivityStep::StartTimeout()
{
	if (TimeoutSeconds <= 0.f)
		return;

	UWorld* World = GetOuter()->GetWorld();
	if (!World)
	{
		// No timer manager to fail us: the window would stay open forever, which is not what the
		// data asks for.
		UE_LOGFMT(MS_WindowInteractionActivityStep, Warning, "No world to arm the timeout on. The window will never close.");
		return;
	}

	World->GetTimerManager().SetTimer(TimeoutHandle,
		[this]
		{
			UE_LOGFMT(MS_WindowInteractionActivityStep, Verbose, "Window missed.");

			FinishStep(FActivityStepResult{
				.Status = EActivityStepStatus::Fail,
				.Score = 0, // TODO: Fill this field
			});
		},
		TimeoutSeconds,
		false);
}

void UWindowInteractionActivityStep::ClearTimeout()
{
	if (UWorld* World = GetOuter()->GetWorld())
	{
		World->GetTimerManager().ClearTimer(TimeoutHandle);
	}
}
