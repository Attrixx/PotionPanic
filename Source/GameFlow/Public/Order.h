// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Order.generated.h"

class URecipeAsset;

UENUM()
enum class EOrderState
{
	Pending,
	Placed,
	Cancelled, // Negative outcome
	Completed, // Positive outcome
	SystemDeleted, // Deleted without an outcome
};

USTRUCT(BlueprintType)
struct GAMEFLOW_API FOrder
{
	GENERATED_BODY()

	uint32 OrderId;

	UPROPERTY(BlueprintReadOnly)
	TObjectPtr<URecipeAsset> Recipe;
	
	UPROPERTY(BlueprintReadOnly)
	EOrderState State;
	
	UPROPERTY(BlueprintReadOnly)
	double StartTime;

	UPROPERTY(BlueprintReadOnly)
	double MaxDuration;
};

DECLARE_MULTICAST_DELEGATE_OneParam(FOrderDelegate, const FOrder&);
