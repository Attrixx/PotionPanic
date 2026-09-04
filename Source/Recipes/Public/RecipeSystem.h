// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "GameplayTagContainer.h"
#include "RecipeSystem.generated.h"

class UActivityAsset;
class URecipeAsset;

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

	// NOTE: If this is ever needed, we need to track how many times each activity was added
	// UFUNCTION(BlueprintCallable)
	// void RemoveRecipe(URecipeAsset* Recipe);

	/**
	 * Finds an activity matching the given tags.
	 * @param Tags Item tags and activity tags.
	 * @return The found activity, or nullptr.
	 */
	UActivityAsset* FindActivityByInputTags(const FGameplayTagContainer& Tags) const;

	UFUNCTION(BlueprintCallable)
	const TArray<UActivityAsset*>& GetActivities() const { return Activities; }

private:

	UPROPERTY()
	TArray<UActivityAsset*> Activities;
};
