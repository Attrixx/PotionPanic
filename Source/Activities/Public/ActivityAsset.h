// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GameplayTagContainer.h"
#include "ActivityAsset.generated.h"

/** What an activity is allowed to do with the item the instigator carries. */
UENUM(BlueprintType)
enum class EActivityTakeFromInstigator : uint8
{
	/** The instigator keeps its item: this activity only ever runs on what the station already holds. */
	Never,

	/** The item moves onto the station's holder to start the activity, and stays the station's. */
	Take,

	/**
	 * Same, but whatever the station still holds when the activity ends goes back to the instigator.
	 * A conclusion that consumed the item leaves nothing to return, and a morphed item comes back
	 * as what it became.
	 */
	TakeAndReturn,
};

class UActivityStepSettings;
class UActivityEvaluator;
class UActivityConclusion;

UCLASS()
class ACTIVITIES_API UActivityAsset : public UPrimaryDataAsset
{
	GENERATED_BODY()

#if WITH_EDITOR
	EDataValidationResult IsDataValid(class FDataValidationContext& Context) const override;
#endif

public:

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FText ActivityName;

	/**
	 * Tags the item on the station's holder must carry. Never empty: an activity that runs on an
	 * empty station says so explicitly with Item.None.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta=(Categories="Item"))
	FGameplayTagContainer StationItemTags;

	/**
	 * Tags the station must implement for this activity to be available on it. Leave EMPTY for an
	 * activity that needs no station in particular.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta=(Categories="Activity"))
	FGameplayTagContainer ActivityTags;

	/**
	 * Tags the item the instigator carries must have. Leave EMPTY to ignore what it holds -- which
	 * is what taking that item requires, since the item it describes is about to become the
	 * station's own.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta=(Categories="Item"))
	FGameplayTagContainer InstigatorItemTags;

	/**
	 * Whether the activity may start by taking the instigator's item onto the station's holder, and
	 * whether it hands it back at the end. Anything but Never is mutually exclusive with
	 * InstigatorItemTags: a taken item is matched against StationItemTags, not the instigator's.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	EActivityTakeFromInstigator TakeFromInstigator = EActivityTakeFromInstigator::Take;

	// Steps to execute after starting the activity.
	// If there is none, starting this activity instantly concludes it with success.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Instanced)
	TArray<TObjectPtr<UActivityStepSettings>> ActivitySteps;

	// Decides if the activity continues after each step and merges the step's score into the activity score.
	// Can be null if there is no steps
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Instanced)
	TObjectPtr<UActivityEvaluator> Evaluator;

	// Concludes the activity on success or failure.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Instanced)
	TObjectPtr<UActivityConclusion> Conclusion;
};
