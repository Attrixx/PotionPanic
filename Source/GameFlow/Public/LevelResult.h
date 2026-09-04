// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "LevelResult.generated.h"

/** Outcome of a whole level, handed to the end screen once the last round is over. */
USTRUCT(BlueprintType)
struct GAMEFLOW_API FLevelResult
{
	GENERATED_BODY()

	/** Points gathered over every round of the level. */
	UPROPERTY(BlueprintReadOnly)
	int64 Score = 0;

	/** Points the level asked for, from UWorldData::ScoreToSucceed. */
	UPROPERTY(BlueprintReadOnly)
	int64 ScoreToSucceed = 0;

	/** True when Score reached ScoreToSucceed. A world asking for zero can only be won. */
	UPROPERTY(BlueprintReadOnly)
	bool bSucceeded = false;

	UPROPERTY(BlueprintReadOnly)
	int32 CompletedOrders = 0;

	/** Orders nobody delivered: expired on their own timer, or still open when the round ended. */
	UPROPERTY(BlueprintReadOnly)
	int32 FailedOrders = 0;
};
