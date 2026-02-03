// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "RecipeSystem.generated.h"

/**
 * 
 */
UCLASS()
class RECIPES_API URecipeSystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:

	UFUNCTION(BlueprintCallable, Category = "Recipes|Shuffling")
	TArray<URecipeAsset*> GetShuffledRecipes(const TArray<URecipeAsset*>& InRecipes);
};
