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
	 * True when Item was taken out of the instigator's hands to start the activity, rather than
	 * already sitting on the Holder. Tells a conclusion where the item belongs.
	 */
	UPROPERTY(BlueprintReadOnly)
	bool bItemTakenFromInstigator = false;

	/**
	 * Last Instigator received through StartActivity or Interact. Can be null.
	 */
	UPROPERTY(BlueprintReadOnly)
	TWeakObjectPtr<AActor> LastInstigator;

	/**
	 * Holder of LastInstigator. Null when there is no instigator, or it carries no holder.
	 * Refreshed on every interact, so it never outlives the instigator it was read from.
	 */
	UPROPERTY(BlueprintReadOnly)
	TWeakObjectPtr<UHolderComponent> InstigatorHolder;

	/**
	 * Item carried by InstigatorHolder. Can be null.
	 * Refreshed on every interact, so it never points at an item that was consumed or handed over.
	 */
	UPROPERTY(BlueprintReadOnly)
	TWeakObjectPtr<AItemActor> InstigatorItem;

	/**
	 * True once a different instigator took the activity over. Where the item came from no longer
	 * describes where it should go back to, so origin-based targets degrade to the station.
	 */
	UPROPERTY(BlueprintReadOnly)
	bool bInstigatorChanged = false;

	UPROPERTY(BlueprintReadOnly)
	EActivityExecutionStatus Status = EActivityExecutionStatus::NotStarted;

	/**
	 * Global score of the activity.
	 */
	UPROPERTY(BlueprintReadOnly)
	int32 Score = 0;
};
