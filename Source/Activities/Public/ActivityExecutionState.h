// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ActivityExecutionState.generated.h"

class UHolderComponent;
class AItemActor;

UENUM(BlueprintType)
enum class EActivityExecutionStatus : uint8
{
	NotStarted = 0,
	Ongoing,
	Success,
	Failed,
	Canceled,
};

USTRUCT(BlueprintType)
struct FActivityExecutionState
{
	GENERATED_BODY()

	/**
	 * Holder where the Activity is executed.
	 */
	UPROPERTY(BlueprintReadOnly)
	TWeakObjectPtr<UHolderComponent> Holder;

	/**
	 * Item present on the Holder. Can be null.
	 */
	UPROPERTY(BlueprintReadOnly)
	TWeakObjectPtr<AItemActor> Item; // = Cast<AItemActor>(Holder->GetCarriable());

	/**
	 * Second ingredient, carried by the instigator, matched against the activity's
	 * SecondaryInputTags. Stays in their hands for the whole activity and is consumed when it
	 * concludes. Null for a single-item activity.
	 */
	UPROPERTY(BlueprintReadOnly)
	TWeakObjectPtr<AItemActor> SecondaryItem;

	/**
	 * Last Instigator received through StartActivity or Interact. Can be null.
	 */
	UPROPERTY(BlueprintReadOnly)
	TWeakObjectPtr<AActor> LastInstigator;

	UPROPERTY(BlueprintReadOnly)
	EActivityExecutionStatus Status = EActivityExecutionStatus::NotStarted;

	/**
	 * Global score of the activity.
	 */
	UPROPERTY(BlueprintReadOnly)
	int32 Score = 0;
};
