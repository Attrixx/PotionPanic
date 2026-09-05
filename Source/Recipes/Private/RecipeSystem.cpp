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

UActivityAsset* URecipeSystem::FindActivity(const FGameplayTagContainer& StationItemTags,
                                            const FGameplayTagContainer& ActivityTags,
                                            const FGameplayTagContainer& InstigatorItemTags) const
{
	for (UActivityAsset* Activity : Activities)
	{
		check(Activity);

		// HasAll against an empty container is true, so an activity leaves an axis unconstrained
		// by leaving its container empty.
		if (StationItemTags.HasAll(Activity->StationItemTags)
			&& ActivityTags.HasAll(Activity->ActivityTags)
			&& InstigatorItemTags.HasAll(Activity->InstigatorItemTags))
		{
			// Returns the first transformation that matches
			return Activity;
		}
	}
	return nullptr;
}
