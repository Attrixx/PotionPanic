// Fill out your copyright notice in the Description page of Project Settings.


#include "RecipeSystem.h"
#include <algorithm>
#include <random>

TArray<URecipeAsset*> URecipeSystem::GetShuffledRecipes(const TArray<URecipeAsset*>& InRecipes)
{
	TArray<URecipeAsset*> ShuffledRecipes = InRecipes;

	if (ShuffledRecipes.Num() <= 1)
	{
		return ShuffledRecipes;
	}

	std::random_device RandomDevice;
	std::default_random_engine RandomGenerator(RandomDevice());

	std::shuffle(ShuffledRecipes.GetData(), ShuffledRecipes.GetData() + ShuffledRecipes.Num(), RandomGenerator);

	return ShuffledRecipes;
}
