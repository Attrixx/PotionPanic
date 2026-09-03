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

	// Tags that must be present on the item or the station's implemented activities.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta=(Categories="Activity,Item"))
	FGameplayTagContainer InputTags;

	/**
	 * Tags a SECOND item, carried by the instigator, must have for this activity to start.
	 *
	 * Leave empty for a single-item activity. When filled, the item already on the station is the
	 * subject the conclusion transforms, and the carried one is the ingredient handed to it: it is
	 * consumed once the activity concludes, on success as well as on failure. The station holder
	 * only ever holds one item, which is why the second one stays in the instigator's hands for
	 * the whole activity.
	 *
	 * So the flow is: drop the subject on the station, come back carrying the ingredient, interact.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta=(Categories="Item"))
	FGameplayTagContainer SecondaryInputTags;
	
	// If true the activity can be started by taking the input item from the instigator's hands.
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	bool bCanTakeItemFromInstigator;

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
