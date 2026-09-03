// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "ActivityConclusion.generated.h"

struct FActivityExecutionState;
class UItemAsset;

/**
 * Abstract base class defining the final outcome of an Activity.
 */
UCLASS(Abstract, EditInlineNew, Blueprintable)
class ACTIVITIES_API UActivityConclusion : public UObject
{
	GENERATED_BODY()

public:

	/**
	 * Executes the final logic of the activity based on its final execution state.
	 * @param State The final context and evaluation data of the completed activity.
	 * @warning Child classes (both C++ and Blueprint) MUST override this method. Failure to do so will trigger a fatal error.
	 */
	UFUNCTION(BlueprintNativeEvent)
	void Conclude(const FActivityExecutionState& State) const;

#if WITH_EDITOR
	/**
	 * Collects every item this conclusion may leave in the world when the activity succeeds.
	 * Used to validate that the items a round orders can actually be crafted during it.
	 * Blueprint conclusions cannot report anything here, so an empty result is not proof that
	 * nothing is produced.
	 */
	virtual void GatherItemsProducedOnSuccess(TSet<const UItemAsset*>& OutItems) const {}
#endif

protected:

	virtual void Conclude_Implementation(const FActivityExecutionState& State) const PURE_VIRTUAL(UActivityConclusion::Conclude);
};
