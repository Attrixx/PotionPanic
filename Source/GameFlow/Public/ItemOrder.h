// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ItemOrder.generated.h"

class UItemAsset;

UENUM(BlueprintType)
enum class EOrderState : uint8
{
	Pending,
	Placed,
	Cancelled, // Negative outcome
	Completed, // Positive outcome
	SystemDeleted, // Deleted without an outcome
};

USTRUCT(BlueprintType)
struct GAMEFLOW_API FItemOrder
{
	GENERATED_BODY()

	/**
	 * Identifies the order for its whole life, including across the network.
	 * The UPROPERTY is what puts it on the wire: an unreflected member does not replicate, and
	 * every order would reach the clients sharing the same id.
	 */
	UPROPERTY(BlueprintReadOnly)
	int32 OrderId = 0;

	/** The item that has to be delivered to complete this order. */
	UPROPERTY(BlueprintReadOnly)
	TObjectPtr<UItemAsset> Item;
	
	UPROPERTY(BlueprintReadOnly)
	EOrderState State;
	
	UPROPERTY(BlueprintReadOnly)
	double StartTime;

	UPROPERTY(BlueprintReadOnly)
	double MaxDuration;

	/** Points this order awarded, set when it is completed and left at zero otherwise. */
	UPROPERTY(BlueprintReadOnly)
	int32 Score = 0;
};

DECLARE_MULTICAST_DELEGATE_OneParam(FOrderDelegate, const FItemOrder&);
