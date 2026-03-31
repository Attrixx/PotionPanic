// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "ActivityExecutionState.h"
#include "ActivityConclusion.generated.h"

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

protected:

	virtual void Conclude_Implementation(const FActivityExecutionState& State) const PURE_VIRTUAL(UActivityConclusion::Conclude);
};
