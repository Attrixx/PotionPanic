// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "ActivityInputCapture.generated.h"

class UActivityExecutor;

UINTERFACE(BlueprintType)
class ACTIVITIES_API UActivityInputCapture : public UInterface
{
	GENERATED_BODY()
};

/**
 * Implemented by actors that can hand their inputs over to an activity step -- in practice, the
 * player pawn.
 *
 * Both calls run on the authority, where the steps live. The implementation is expected to forward
 * them to the owning client, which is the only side that has inputs to give in the first place.
 */
class ACTIVITIES_API IActivityInputCapture
{
	GENERATED_BODY()

public:

	/**
	 * Takes the actor's gameplay inputs over for the duration of a step: it must stop moving and
	 * interacting with the world, and route its presses back through
	 * UActivityExecutor::ReceiveActivityInput on Executor.
	 * @param Executor The executor running the step that wants the inputs.
	 */
	UFUNCTION(BlueprintNativeEvent, Category = "Activity")
	void BeginActivityInputCapture(UActivityExecutor* Executor);

	/** Gives the inputs back. Always called, cancellation included. */
	UFUNCTION(BlueprintNativeEvent, Category = "Activity")
	void EndActivityInputCapture();
};
