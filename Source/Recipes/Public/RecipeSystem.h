// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "GameplayTagContainer.h"
#include "RecipeSystem.generated.h"

class UActivityStep;
class URecipeAsset;
class UItemAsset;
class UItemTransformation;

USTRUCT()
struct FInstruction
{
	GENERATED_BODY()

	UPROPERTY()
	TArray<TObjectPtr<UActivityStep>> Steps;

	UPROPERTY()
	TObjectPtr<UItemAsset> OutputItem;
};

/**
 * 
 */
UCLASS()
class RECIPES_API URecipeSystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:

	UFUNCTION(BlueprintCallable)
	void AddRecipe(URecipeAsset* Recipe);

	UFUNCTION(BlueprintCallable)
	void ClearRecipes();

	// NOTE: If this is ever needed, we need to track how many times each transformation was added
	// UFUNCTION(BlueprintCallable)
	// void RemoveRecipe(URecipeAsset* Recipe);

	/**
	 * Finds a transformation matching the given tags and creates an instruction from it.
	 * @param Tags Item tags and activity tags.
	 * @return Instruction, or NullOpt if no transformation matches the tags.
	 */
	TOptional<FInstruction> CreateInstruction(const FGameplayTagContainer& Tags);

	UFUNCTION(BlueprintCallable)
	const TArray<UItemTransformation*>& GetTransformations() const { return Transformations; }

private:

	FInstruction CreateInstruction(UItemTransformation& Transformation);

	UPROPERTY()
	TArray<UItemTransformation*> Transformations;
};
