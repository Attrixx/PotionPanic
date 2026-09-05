// Fill out your copyright notice in the Description page of Project Settings.

#include "Widgets/ActivityStepWidget.h"
#include <GameFramework/GameStateBase.h>
#include <GameFramework/PlayerController.h>

void UActivityStepWidget::SetPresentation(const FActivityStepPresentation& InPresentation)
{
	Presentation = InPresentation;
	OnPresentationUpdated();
}

bool UActivityStepWidget::IsLocalPlayerInstigator() const
{
	const AActor* Instigator = Presentation.Instigator;
	if (!Instigator)
		return false;

	// A world-space widget has no owning player of its own: it belongs to the station, not to
	// anyone watching it. Fall back to whoever is playing on this client.
	const APlayerController* LocalController = GetOwningPlayer();
	if (!LocalController)
	{
		const UWorld* World = GetWorld();
		LocalController = World ? World->GetFirstPlayerController() : nullptr;
	}

	return LocalController && LocalController->GetPawn() == Instigator;
}

float UActivityStepWidget::GetStartToOpenAlpha() const
{
	const double Duration = Presentation.OpenServerTime - Presentation.StartServerTime;
	if (Duration <= 0.0)
		return 1.f; // nothing to wait for: the opening already happened

	const double Elapsed = GetServerTimeSeconds() - Presentation.StartServerTime;
	return static_cast<float>(FMath::Clamp(Elapsed / Duration, 0.0, 1.0));
}

float UActivityStepWidget::GetOpenToCloseAlpha() const
{
	const double Duration = Presentation.CloseServerTime - Presentation.OpenServerTime;
	if (Duration <= 0.0)
		return 0.f; // never closes: nothing is running out

	// Clamped low as well as high, so this reads 0 for the whole opening delay rather than going
	// negative behind the first bar.
	const double Elapsed = GetServerTimeSeconds() - Presentation.OpenServerTime;
	return static_cast<float>(FMath::Clamp(Elapsed / Duration, 0.0, 1.0));
}

bool UActivityStepWidget::IsOpen() const
{
	return GetServerTimeSeconds() >= Presentation.OpenServerTime;
}

float UActivityStepWidget::GetRemainingSeconds() const
{
	if (Presentation.CloseServerTime <= Presentation.OpenServerTime)
		return 0.f;

	const double Remaining = Presentation.CloseServerTime - GetServerTimeSeconds();
	return static_cast<float>(FMath::Max(Remaining, 0.0));
}

EActivityInputSlot UActivityStepWidget::GetCurrentSlot() const
{
	return Presentation.Sequence.IsValidIndex(Presentation.CurrentPressIndex)
		? Presentation.Sequence[Presentation.CurrentPressIndex]
		: EActivityInputSlot::None;
}

float UActivityStepWidget::GetComboProgressAlpha() const
{
	if (Presentation.Sequence.IsEmpty())
		return 0.f;

	// CurrentPressIndex is how many have been answered: it is the one being asked for, and every
	// index below it already has a result. INDEX_NONE outside a combo, hence the low clamp.
	const int32 Answered = FMath::Max(Presentation.CurrentPressIndex, 0);
	return FMath::Clamp(static_cast<float>(Answered) / Presentation.Sequence.Num(), 0.f, 1.f);
}

float UActivityStepWidget::GetComboErrorAlpha() const
{
	if (Presentation.Sequence.IsEmpty())
		return 0.f;

	// A combo that tolerates nothing is at full stakes from its first press: there is no margin to
	// spend, so it is already one miss away from ending. Also the only division by zero here.
	if (Presentation.MaxFailedPresses <= 0)
		return 1.f;

	int32 Missed = 0;
	for (EActivityPressResult Result : Presentation.Results)
	{
		if (Result == EActivityPressResult::Miss)
		{
			++Missed;
		}
	}

	return FMath::Clamp(static_cast<float>(Missed) / Presentation.MaxFailedPresses, 0.f, 1.f);
}

double UActivityStepWidget::GetServerTimeSeconds() const
{
	const UWorld* World = GetWorld();
	const AGameStateBase* GameState = World ? World->GetGameState() : nullptr;
	return GameState ? GameState->GetServerWorldTimeSeconds() : 0.0;
}
