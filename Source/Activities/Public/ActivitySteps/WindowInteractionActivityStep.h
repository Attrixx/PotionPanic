// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ActivityStep.h"
#include "ActivityStepSettings.h"
#include "WindowInteractionActivityStep.generated.h"

UCLASS(DisplayName = "Window Interaction")
class ACTIVITIES_API UWindowInteractionActivitySettings : public UActivityStepSettings
{
	GENERATED_BODY()

#if WITH_EDITOR
	EDataValidationResult IsDataValid(FDataValidationContext& Context) const override;
#endif

	UActivityStep* CreateStep_Implementation(UObject* Outer) const override;

public:

	/**
	 * Whether an interact is required even when a valid instigator is already there on start.
	 * True -- the default -- is what makes this a window: the player must press during it. False
	 * lets the step pass through immediately when someone is already standing there, which turns
	 * the timeout into a formality.
	 */
	UPROPERTY(EditAnywhere, Category="")
	bool bAlwaysWaitForInteract = true;

	/** How long the window stays open before the step fails. Zero -- the default -- never closes it. */
	UPROPERTY(EditAnywhere, Category="", meta=(ClampMin=0))
	float TimeoutSeconds = 0.f;
};

/**
 * Waits for an interact from a valid instigator, optionally within TimeoutSeconds.
 *
 * This is the second half of an IFT: put a "Time" step in front of it for the delay before the
 * window opens, and this one -- with the timeout set -- for the window itself. Without a
 * timeout it degenerates into a plain "wait for the instigator", which is a legitimate use.
 */
UCLASS()
class ACTIVITIES_API UWindowInteractionActivityStep : public UActivityStep
{
	GENERATED_BODY()

	void StartStep_Implementation(AActor* LastInstigator) override;
	void OnInteract_Implementation(AActor* Instigator) override;
	void CancelStep_Implementation() override;

	/** Arms the timeout when TimeoutSeconds is positive. Does nothing otherwise. */
	void StartTimeout();

	void ClearTimeout();

	friend UWindowInteractionActivitySettings;

	bool bAlwaysWaitForInteract = true;
	float TimeoutSeconds = 0.f;

	FTimerHandle TimeoutHandle;
};
