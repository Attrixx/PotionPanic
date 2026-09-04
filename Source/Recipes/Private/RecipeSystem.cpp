// Fill out your copyright notice in the Description page of Project Settings.

#include "RecipeSystem.h"
#include "RecipeAsset.h"
#include "ActivityAsset.h"

DEFINE_LOG_CATEGORY_STATIC(MS_RecipeSystem, Log, All);

void URecipeSystem::AddRecipe(URecipeAsset* Recipe)
{
	if (!Recipe)
		return;

	for (UActivityAsset* Step : Recipe->Steps)
	{
		check(Step);
		Activities.AddUnique(Step);
	}
}

void URecipeSystem::ClearRecipes()
{
	Activities.Empty();
}

UActivityAsset* URecipeSystem::FindActivityByInputTags(const FGameplayTagContainer& Tags) const
{
	for (UActivityAsset* Activity : Activities)
	{
		check(Activity);
		if (Tags.HasAll(Activity->InputTags))
		{
			// Returns the first transformation that matches
			return Activity;
		}
	}
	return nullptr;
}
