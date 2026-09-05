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

	/**
	 * First half of the timeline: the wait before input is accepted.
	 * @return 0 at the start, 1 at the opening. Already 1 when there was nothing to wait for.
	 */
	UFUNCTION(BlueprintPure, Category = "Activity")
	float GetStartToOpenAlpha() const;

	/**
	 * Second half: the window that is open, or the press that is on the clock. Only starts moving
	 * once GetStartToOpenAlpha() has reached 1.
	 * @return 0 at the opening, 1 at the closing. Stays 0 when it never closes.
	 */
	UFUNCTION(BlueprintPure, Category = "Activity")
	float GetOpenToCloseAlpha() const;

	/** @return True once input is being accepted, i.e. the opening time has passed. */
	UFUNCTION(BlueprintPure, Category = "Activity")
	bool IsOpen() const;

	/** @return Seconds left before the closing, 0 once it is past or when it never closes. */
	UFUNCTION(BlueprintPure, Category = "Activity")
	float GetRemainingSeconds() const;

	/** @return The input being asked for right now. None outside a combo. */
	UFUNCTION(BlueprintPure, Category = "Activity")
	EActivityInputSlot GetCurrentSlot() const;

	/**
	 * How far through the combo the player is, counting presses answered rather than time.
	 * @return Answered presses over total presses. 0 outside a combo.
	 */
	UFUNCTION(BlueprintPure, Category = "Activity")
	float GetComboProgressAlpha() const;

	/**
	 * How much of the combo's tolerance for mistakes has been spent.
	 * @return 0 with nothing missed yet, up to 1 once one more miss would end the step. 0 outside
	 *         a combo. Already 1 on a combo that tolerates nothing, where the very first miss is
	 *         the fatal one.
	 * @note 1 is the last chance, not the failure: the step is still alive there, and dies on the
	 *       miss after.
	 */
	UFUNCTION(BlueprintPure, Category = "Activity")
	float GetComboErrorAlpha() const;

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
