// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ActivityConclusion.h"
#include "DeliverOrderActivityConclusion.generated.h"

/**
 * Hands the activity's item over to the round's orders: consumes it when an order was waiting for
 * it, and leaves it untouched otherwise.
 */
UCLASS(DisplayName = "Deliver Order")
class UDeliverOrderActivityConclusion : public UActivityConclusion
{
	GENERATED_BODY()

protected:

	void Conclude_Implementation(const FActivityExecutionState& State) const override;
};
