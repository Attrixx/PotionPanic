// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "ActivityExecutor.h"
#include "ActivityConclusion.generated.h"

struct FActivityExecutionState;

/**
 * 
 */
UCLASS(Abstract, EditInlineNew, Blueprintable)
class ACTIVITIES_API UActivityConclusion : public UObject
{
	GENERATED_BODY()

public:

	/**
	 * Concludes the activity. Often by spawning an item or modifying the input item.
	 * @param State Output of the activity.
	 * @return Transformed Item. Can be equal or not to the InputItem.
	 */
	UFUNCTION(BlueprintNativeEvent)
	void Conclude(const FActivityExecutionState& State) const;

protected:

	virtual void Conclude_Implementation(const FActivityExecutionState& State) const PURE_VIRTUAL(UActivityConclusion::Conclude)
};
