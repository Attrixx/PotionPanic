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
