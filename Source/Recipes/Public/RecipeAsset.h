// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Misc/DataValidation.h"
#include "RecipeAsset.generated.h"

class UItemTransformation;
class UItemAsset;

UENUM(BlueprintType)
enum class ERecipeInteractionScoreMode : uint8
{
	Additive UMETA(DisplayName = "Additive"),
	Average UMETA(DisplayName = "Average"),
	BestStep UMETA(DisplayName = "BestStep"),
	WorstStep UMETA(DisplayName = "WorstStep")
};

/** Data definition for a linear recipe sequence. */
UCLASS(BlueprintType)
class RECIPES_API URecipeDataAsset : public UPrimaryDataAsset
{
	GENERATED_BODY()
	
public:
	UFUNCTION(BlueprintCallable, Category = "Recipe|Validation")
	bool IsRecipeDefinitionValid(FText& OutFailureReason) const;

#if WITH_EDITOR
	virtual EDataValidationResult IsDataValid(FDataValidationContext& Context) const override;
#endif

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Recipe")
	FText RecipeName;

	/** Ordered list of operators/transformations. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Recipe")
	TArray<TObjectPtr<UItemTransformation>> Steps;

	/** Bonus applied when recipe is fully completed. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Scoring")
	int32 CompletionBonusScore = 250;

	/** Strategy used to aggregate interaction score contributions. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Scoring")
	ERecipeInteractionScoreMode InteractionScoreMode = ERecipeInteractionScoreMode::Additive;

	/** If true, inputs must match each step in exact order/count. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Validation")
	bool bStrictLinearOrder = true;

	/** If true, inputs matched before a failure are considered consumed. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Failure")
	bool bConsumeMatchedInputsOnFailure = true;

	/** Optional output item produced when the recipe fails. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Failure")
	TObjectPtr<UItemAsset> FailureOutputItem = nullptr;
	/** Station-level overrides can replace this fallback in RecipeStationIntegrationSubsystem. */

	/** Quantity of failure output generated when a failure output item is configured. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Failure", meta = (ClampMin = "1", EditCondition = "FailureOutputItem != nullptr"))
	int32 FailureOutputQuantity = 1;

	// TODO (Nath): Extend recipe definition with optional/branching paths.
	// TODO (Nath): Consider an Unreal Editor tool (Editor Utility Widget/custom details panel)
	// to author recipe steps faster and validate constraints before save.
};
