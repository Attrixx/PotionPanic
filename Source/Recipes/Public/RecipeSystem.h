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
	 * Finds the first activity whose three input containers are all satisfied. An empty container
	 * on the activity accepts anything, which is how an activity declares it needs no station or
	 * does not care what the instigator carries.
	 * @param StationItemTags Tags of the item on the station's holder, or Item.None for an empty one.
	 * @param ActivityTags Activities the station implements.
	 * @param InstigatorItemTags Tags of the item the instigator carries, or Item.None for empty hands.
	 * @return The found activity, or nullptr.
	 */
	UActivityAsset* FindActivity(const FGameplayTagContainer& StationItemTags,
	                             const FGameplayTagContainer& ActivityTags,
	                             const FGameplayTagContainer& InstigatorItemTags) const;

	UFUNCTION(BlueprintCallable)
	const TArray<UActivityAsset*>& GetActivities() const { return Activities; }

private:

	UPROPERTY()
	TArray<UActivityAsset*> Activities;
};
