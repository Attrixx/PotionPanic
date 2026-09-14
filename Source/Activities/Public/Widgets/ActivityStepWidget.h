// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "ActivityStepPresentation.h"
#include "ActivityStepWidget.generated.h"

/**
 * Base class for the widgets drawing a running activity step.
 *
 * The same widget runs on every client, including the ones only watching, so the first thing a
 * subclass usually does in OnPresentationUpdated is branch on IsLocalPlayerInstigator(): the player
 * driving the step needs to know which input to press, the others only need to see it happening.
 *
 * Everything it draws from is replicated state, and nothing in here decides anything: the
 * authority has already made every call by the time a presentation lands.
 *
 * The timing helpers read a clock rather than a replicated progress value, so they are worth
 * polling every frame -- from a Tick or a bound property -- not just on OnPresentationUpdated.
 */
UCLASS(Abstract)
class ACTIVITIES_API UActivityStepWidget : public UUserWidget
{
	GENERATED_BODY()

public:

	/**
	 * Pushes a new presentation in and notifies the subclass. Called on creation, then on every
	 * replicated change.
	 * @param InPresentation What the step wants displayed.
	 */
	void SetPresentation(const FActivityStepPresentation& InPresentation);

	/** What the step wants displayed, as of the last replicated change. */
	UFUNCTION(BlueprintPure, Category = "Activity")
	const FActivityStepPresentation& GetPresentation() const { return Presentation; }

	/**
	 * @return True when the local player is the one this step is running for.
	 * @note Reads the first local player: a splitscreen client would need one widget per player.
	 */
	UFUNCTION(BlueprintPure, Category = "Activity")
	bool IsLocalPlayerInstigator() const;
	
	UFUNCTION(BlueprintPure, Category = "Activity")
	double GetElapsedTimeSinceStart() const;	

	UFUNCTION(BlueprintPure, Category = "Activity")
	const TInstancedStruct<FActivityStepPresentationCustomInfo>& GetCustomInfo() { return Presentation.CustomInfo; }
	
protected:

	/**
	 * The presentation changed. Redraw from GetPresentation() and the helpers above.
	 * Fires once on creation too, so there is no separate initial state to handle.
	 */
	UFUNCTION(BlueprintImplementableEvent, Category = "Activity", meta = (DisplayName = "On Presentation Updated"))
	void OnPresentationUpdated();

private:

	/** @return The shared clock the presentation's times are expressed in. */
	double GetServerTimeSeconds() const;

	UPROPERTY()
	FActivityStepPresentation Presentation;
};
