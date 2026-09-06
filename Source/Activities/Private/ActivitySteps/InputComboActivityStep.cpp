// Fill out your copyright notice in the Description page of Project Settings.

#include "ActivitySteps/InputComboActivityStep.h"
#include "ActivityExecutor.h"
#include "ActivityInputCapture.h"
#include "ActivityStepResult.h"
#include <Misc/DataValidation.h>

DEFINE_LOG_CATEGORY_STATIC(MS_InputComboActivityStep, Verbose, All);

namespace
{
/** The slots a mask can hold, in the order a drawn index walks them. */
constexpr EActivityInputSlot AllSlots[] = {
	EActivityInputSlot::Up,
	EActivityInputSlot::Left,
	EActivityInputSlot::Down,
	EActivityInputSlot::Right,
};

bool MaskHas(uint8 Mask, EActivityInputSlot Slot)
{
	return (Mask & static_cast<uint8>(Slot)) != 0;
}

int32 CountSlots(uint8 Mask)
{
	int32 Count = 0;
	for (EActivityInputSlot Slot : AllSlots)
	{
		Count += MaskHas(Mask, Slot) ? 1 : 0;
	}
	return Count;
}

/** @return The Index-th slot present in Mask, None when it holds fewer than that. */
EActivityInputSlot SlotAt(uint8 Mask, int32 Index)
{
	for (EActivityInputSlot Slot : AllSlots)
	{
		if (MaskHas(Mask, Slot) && Index-- == 0)
		{
			return Slot;
		}
	}
	return EActivityInputSlot::None;
}
}

#if WITH_EDITOR
EDataValidationResult UInputComboActivitySettings::IsDataValid(FDataValidationContext& Context) const
{
	EDataValidationResult Result = Super::IsDataValid(Context);

	if (CountSlots(AllowedSlots) == 0)
	{
		Context.AddError(FText::FromString("Field 'AllowedSlots' must hold at least one input."));
		Result = EDataValidationResult::Invalid;
	}

	if (PressCount < 1)
	{
		Context.AddError(FText::Format(FTextFormat::FromString("Field 'PressCount' should be at least 1 (Current value: {0})."), PressCount));
		Result = EDataValidationResult::Invalid;
	}

	if (MaxFailedPresses < 0)
	{
		Context.AddError(FText::Format(FTextFormat::FromString("Field 'MaxFailedPresses' should be positive (Current value: {0})."), MaxFailedPresses));
		Result = EDataValidationResult::Invalid;
	}

	if (PressTimeoutSeconds <= 0.f)
	{
		Context.AddError(FText::Format(FTextFormat::FromString("Field 'PressTimeoutSeconds' should be greater than zero (Current value: {0})."),
			PressTimeoutSeconds));
		Result = EDataValidationResult::Invalid;
	}

	if (MaxFailedPresses >= PressCount)
	{
		Context.AddWarning(FText::FromString("'MaxFailedPresses' covers every press: the combo cannot be failed."));
	}

	if (!bAllowConsecutiveRepeats && CountSlots(AllowedSlots) == 1)
	{
		Context.AddWarning(FText::FromString("'bAllowConsecutiveRepeats' is off but a single input is allowed: the same one repeats anyway."));
	}

	return Result;
}
#endif

UActivityStep* UInputComboActivitySettings::CreateStep_Implementation(UObject* Outer) const
{
	check(PressCount >= 1 && MaxFailedPresses >= 0 && PressTimeoutSeconds > 0.f);

	auto* Step = NewObject<UInputComboActivityStep>(Outer);
	Step->AllowedSlots = AllowedSlots;
	Step->PressCount = PressCount;
	Step->MaxFailedPresses = MaxFailedPresses;
	Step->PressTimeoutSeconds = PressTimeoutSeconds;
	Step->bAllowConsecutiveRepeats = bAllowConsecutiveRepeats;
	return Step;
}

void UInputComboActivityStep::StartStep_Implementation(AActor* LastInstigator)
{
	CapturedInstigator.Reset();
	CurrentPressIndex = INDEX_NONE;
	SuccessCount = 0;
	FailureCount = 0;
	bWaitingForInstigator = false;
	Sequence.Reset();
	Results.Reset();

	if (CountSlots(AllowedSlots) == 0)
	{
		UE_LOGFMT(MS_InputComboActivityStep, Warning, "No allowed input to draw from, the combo cannot run.");
		Finish(EActivityStepStatus::Fail);
		return;
	}

	if (IsValid(LastInstigator))
	{
		BeginCombo(LastInstigator);
		return;
	}

	UE_LOGFMT(MS_InputComboActivityStep, Verbose, "Waiting for an instigator to take the combo.");
	bWaitingForInstigator = true;
}

void UInputComboActivityStep::OnInteract_Implementation(AActor* Instigator)
{
	if (!bWaitingForInstigator || !IsValid(Instigator))
		return;

	BeginCombo(Instigator);
}

void UInputComboActivityStep::CancelStep_Implementation()
{
	// No FinishStep here: the executor cancelled us, it is not waiting for a result.
	ClearPressTimeout();
	ReleaseCapture();
	CurrentPressIndex = INDEX_NONE;
	bWaitingForInstigator = false;
}

void UInputComboActivityStep::OnActivityInput_Implementation(AActor* Instigator, EActivityInputSlot Slot)
{
	if (CurrentPressIndex == INDEX_NONE || Instigator != CapturedInstigator.Get())
	{
		// Another player's pawn, or a press that crossed the wire after the combo ended.
		return;
	}

	const bool bHit = Slot == Sequence[CurrentPressIndex];

	UE_LOGFMT(MS_InputComboActivityStep,
		Verbose,
		"Press {0}/{1}: got slot {2}, wanted {3} -- {4}.",
		CurrentPressIndex + 1,
		Sequence.Num(),
		static_cast<int32>(Slot),
		static_cast<int32>(Sequence[CurrentPressIndex]),
		bHit ? TEXT("hit") : TEXT("miss"));

	RecordPress(bHit ? EActivityPressResult::Hit : EActivityPressResult::Miss);
}

void UInputComboActivityStep::OnCancelRequested_Implementation(AActor* Instigator)
{
	if (CurrentPressIndex == INDEX_NONE || Instigator != CapturedInstigator.Get())
		return;

	UE_LOGFMT(MS_InputComboActivityStep, Verbose, "'{0}' gave up on the combo.", GetNameSafe(Instigator));

	ClearPressTimeout();
	Finish(EActivityStepStatus::Fail);
}

void UInputComboActivityStep::BeginCombo(AActor* Instigator)
{
	check(IsValid(Instigator) && CountSlots(AllowedSlots) > 0);

	bWaitingForInstigator = false;
	CapturedInstigator = Instigator;

	Sequence.Reset(PressCount);
	EActivityInputSlot Previous = EActivityInputSlot::None;
	for (int32 Index = 0; Index < PressCount; ++Index)
	{
		Previous = DrawSlot(Previous);
		Sequence.Emplace(Previous);
	}

	Results.Init(EActivityPressResult::Pending, PressCount);
	CurrentPressIndex = 0;
	SuccessCount = 0;
	FailureCount = 0;

	UE_LOGFMT(MS_InputComboActivityStep,
		Verbose,
		"'{0}' starts a {1} press combo ({2} miss allowed, {3}s each).",
		GetNameSafe(Instigator),
		PressCount,
		MaxFailedPresses,
		PressTimeoutSeconds);

	if (Instigator->Implements<UActivityInputCapture>())
	{
		IActivityInputCapture::Execute_BeginActivityInputCapture(Instigator, GetExecutor());
	}
	else
	{
		// The combo still runs and still times out, it just cannot be played. Worth shouting about:
		// this is a pawn that was never set up for activities.
		UE_LOGFMT(MS_InputComboActivityStep,
			Warning,
			"'{0}' does not implement IActivityInputCapture: it cannot press anything.",
			GetNameSafe(Instigator));
	}

	ArmPressTimeout();
	PublishPresentation();
}

EActivityInputSlot UInputComboActivityStep::DrawSlot(EActivityInputSlot Previous) const
{
	uint8 Candidates = AllowedSlots;

	// Taking the last drawn one out of the pool is all "no repeats" means -- except when it would
	// empty the pool, where there is nothing else to draw and it comes up again regardless.
	if (!bAllowConsecutiveRepeats && Previous != EActivityInputSlot::None && CountSlots(Candidates) > 1)
	{
		// Narrowed back explicitly: ~ promotes to int, and the compound form would convert on the
		// way in under a build that takes warnings as errors.
		Candidates = static_cast<uint8>(Candidates & ~static_cast<uint8>(Previous));
	}

	const int32 Count = CountSlots(Candidates);
	check(Count > 0);

	return SlotAt(Candidates, FMath::RandRange(0, Count - 1));
}

void UInputComboActivityStep::RecordPress(EActivityPressResult Result)
{
	check(Sequence.IsValidIndex(CurrentPressIndex));

	ClearPressTimeout();

	Results[CurrentPressIndex] = Result;
	++(Result == EActivityPressResult::Hit ? SuccessCount : FailureCount);
	++CurrentPressIndex;

	if (FailureCount > MaxFailedPresses)
	{
		// One miss too many. The presses that remain cannot bring the combo back, so making the
		// player enter them would only be a formality.
		UE_LOGFMT(MS_InputComboActivityStep,
			Verbose,
			"Failed at press {0}/{1}: {2} misses for {3} allowed.",
			CurrentPressIndex,
			Sequence.Num(),
			FailureCount,
			MaxFailedPresses);

		Finish(EActivityStepStatus::Fail);
		return;
	}

	if (CurrentPressIndex >= Sequence.Num())
	{
		UE_LOGFMT(MS_InputComboActivityStep, Verbose, "Combo cleared: {0} hits, {1} misses.", SuccessCount, FailureCount);
		Finish(EActivityStepStatus::Success);
		return;
	}

	ArmPressTimeout();
	PublishPresentation();
}

void UInputComboActivityStep::Finish(EActivityStepStatus Status)
{
	const int32 Score = SuccessCount;

	// Before FinishStep: the executor starts the next step from inside it, and that step publishes
	// a presentation of its own that we would otherwise wipe right after.
	ReleaseCapture();
	CurrentPressIndex = INDEX_NONE;

	FinishStep(FActivityStepResult{
		.Status = Status,
		.Score = Score,
	});
}

void UInputComboActivityStep::ReleaseCapture()
{
	if (AActor* Instigator = CapturedInstigator.Get())
	{
		if (Instigator->Implements<UActivityInputCapture>())
		{
			IActivityInputCapture::Execute_EndActivityInputCapture(Instigator);
		}
	}

	CapturedInstigator.Reset();

	if (UActivityExecutor* Executor = GetExecutor())
	{
		Executor->ClearStepPresentation();
	}
}

void UInputComboActivityStep::ArmPressTimeout()
{
	UWorld* World = GetOuter()->GetWorld();
	if (!World)
	{
		// No timer manager to miss on: the combo would wait on a press that may never come.
		UE_LOGFMT(MS_InputComboActivityStep, Warning, "No world to arm the press timeout on.");
		return;
	}

	World->GetTimerManager().SetTimer(PressTimeoutHandle,
		[this] { RecordPress(EActivityPressResult::Miss); },
		PressTimeoutSeconds,
		false);
}

void UInputComboActivityStep::ClearPressTimeout()
{
	if (UWorld* World = GetOuter()->GetWorld())
	{
		World->GetTimerManager().ClearTimer(PressTimeoutHandle);
	}
}

void UInputComboActivityStep::PublishPresentation()
{
	if (UActivityExecutor* Executor = GetExecutor())
	{
		FInputComboActivityPresentation NewPresentation
		{
			.Sequence = Sequence,
			.Results = Results,
			.CurrentPressIndex = CurrentPressIndex,
			.MaxFailedPresses = MaxFailedPresses,
		};
		Executor->SetStepPresentationCustomInfo(NewPresentation);
	}
}

EActivityInputSlot UInputComboActivityStep::GetCurrentSlot(const FInputComboActivityPresentation& Data)
{
	return Data.Sequence.IsValidIndex(Data.CurrentPressIndex)
		? Data.Sequence[Data.CurrentPressIndex]
		: EActivityInputSlot::None;
}

float UInputComboActivityStep::GetComboProgressAlpha(const FInputComboActivityPresentation& Data)
{
	if (Data.Sequence.IsEmpty())
		return 0.f;

	const int32 Answered = FMath::Max(Data.CurrentPressIndex, 0);
	return FMath::Clamp(float(Answered) / Data.Sequence.Num(), 0.f, 1.f);
}

float UInputComboActivityStep::GetComboErrorAlpha(const FInputComboActivityPresentation& Data)
{
	if (Data.Sequence.IsEmpty())
		return 0.f;

	if (Data.MaxFailedPresses <= 0)
		return 1.f;

	int32 Missed = 0;
	for (EActivityPressResult Result : Data.Results)
	{
		if (Result == EActivityPressResult::Miss)
		{
			++Missed;
		}
	}

	return FMath::Clamp(float(Missed) / Data.MaxFailedPresses, 0.f, 1.f);
}
