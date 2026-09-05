// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ActivityStep.h"
#include "ActivityStepSettings.h"
#include "InteractionWindowActivityStep.generated.h"

UCLASS(DisplayName = "Interaction Window")
class ACTIVITIES_API UInteractionWindowActivitySettings : public UActivityStepSettings
{
	GENERATED_BODY()

#if WITH_EDITOR
	EDataValidationResult IsDataValid(FDataValidationContext& Context) const override;
#endif

	UActivityStep* CreateStep_Implementation(UObject* Outer) const override;

public:

	/** How long the step waits before the window opens. Zero opens it immediately. */
	UPROPERTY(EditAnywhere, Category = "", meta = (ClampMin = 0))
	float DelayBeforeOpen = 0.f;

	/**
	 * How long the window stays open before the step fails. Zero leaves it open forever, which
	 * turns the step into a plain "wait for anyone to interact".
	 */
	UPROPERTY(EditAnywhere, Category = "", meta = (ClampMin = 0))
	float WindowDuration = 0.f;
};

/**
 * Waits out a delay, then accepts an interact from anyone for as long as the window stays open.
 *
 * This is the cooking step: DelayBeforeOpen is how long the pot needs, WindowDuration is how long
 * you have to take it off the fire before it burns. The whole table sees it -- the presentation is
 * replicated -- and any player can be the one to press, not just whoever started the activity.
 *
 * Interacting before the window opens does nothing at all: pressing early is not a mistake, it is
 * just early.
 */
UCLASS()
class ACTIVITIES_API UInteractionWindowActivityStep : public UActivityStep
{
	GENERATED_BODY()

	void StartStep_Implementation(AActor* LastInstigator) override;
	void OnInteract_Implementation(AActor* Instigator) override;
	void CancelStep_Implementation() override;

	/** Opens the window and arms the closing timer when WindowDuration is positive. */
	void OpenWindow();

	/** Publishes the current phase so clients can draw it. */
	void PublishPresentation();

	void ClearTimer();

	friend UInteractionWindowActivitySettings;

	float DelayBeforeOpen = 0.f;
	float WindowDuration = 0.f;

	bool bWindowOpen = false;
	FTimerHandle TimerHandle;
};
