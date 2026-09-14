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

double UActivityStepWidget::GetElapsedTimeSinceStart() const
{
	return GetServerTimeSeconds() - Presentation.StartServerTime;
}

double UActivityStepWidget::GetServerTimeSeconds() const
{
	const UWorld* World = GetWorld();
	const AGameStateBase* GameState = World ? World->GetGameState() : nullptr;
	return GameState ? GameState->GetServerWorldTimeSeconds() : 0.0;
}
