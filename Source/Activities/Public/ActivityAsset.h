// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GameplayTagContainer.h"
#include "ActivityAsset.generated.h"

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
	 * is what bCanTakeItemFromInstigator requires, since the item it describes is about to become
	 * the station's own.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta=(Categories="Item"))
	FGameplayTagContainer InstigatorItemTags;

	/**
	 * If true the activity can start by taking the instigator's item onto the station's holder, as
	 * long as that item satisfies StationItemTags. Mutually exclusive with InstigatorItemTags: the
	 * stolen item is matched against the station's requirements, not the instigator's.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	bool bCanTakeItemFromInstigator = true;

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
