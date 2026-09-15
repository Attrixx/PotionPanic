// Fill out your copyright notice in the Description page of Project Settings.

#include "RecipeSystem.h"
#include "RecipeAsset.h"
#include "ActivityAsset.h"

DEFINE_LOG_CATEGORY_STATIC(MS_RecipeSystem, Log, All);

namespace
{
/**
 * How specific an activity is; higher wins. An activity that states what the instigator must carry
 * always beats one that ignores its hands: holding an item is a deliberate use of it (adding an
 * ingredient to a cauldron must not start cooking what is already inside). Past that, the total
 * count of required tags decides, since more tags match fewer situations.
 */
int32 GetSpecificity(const UActivityAsset& Activity)
{
	const int32 TagCount = Activity.StationItemTags.Num() + Activity.ActivityTags.Num() + Activity.InstigatorItemTags.Num();
	return (Activity.InstigatorItemTags.IsEmpty() ? 0 : 1 << 16) + TagCount;
}
}

void URecipeSystem::AddRecipe(URecipeAsset* Recipe)
{
	if (!Recipe)
		return;

	for (UActivityAsset* Step : Recipe->Steps)
	{
		check(Step);
		if (Activities.Contains(Step))
		{
			continue;
		}

		// Keep Activities sorted by decreasing specificity so FindActivity's first match is the most
		// specific one. Inserting after every activity of equal specificity keeps ties in insertion order.
		const int32 Specificity = GetSpecificity(*Step);
		const int32 InsertIndex = Activities.IndexOfByPredicate([Specificity](const UActivityAsset* Activity)
		{
			return GetSpecificity(*Activity) < Specificity;
		});
		Activities.Insert(Step, InsertIndex == INDEX_NONE ? Activities.Num() : InsertIndex);
	}
}

void URecipeSystem::ClearRecipes()
{
	Activities.Empty();
}

UActivityAsset* URecipeSystem::FindActivity(const FGameplayTagContainer& StationItemTags,
                                            const FGameplayTagContainer& ActivityTags,
                                            const FGameplayTagContainer& InstigatorItemTags,
                                            bool bSwapItemRoles) const
{
	for (UActivityAsset* Activity : Activities)
	{
		check(Activity);

		if (bSwapItemRoles && Activity->TakeFromInstigator != EActivityTakeFromInstigator::Swappable)
		{
			continue;
		}

		const FGameplayTagContainer& ExpectedOnStation = bSwapItemRoles ? Activity->InstigatorItemTags : Activity->StationItemTags;
		const FGameplayTagContainer& ExpectedInHands = bSwapItemRoles ? Activity->StationItemTags : Activity->InstigatorItemTags;

		// HasAll against an empty container is true, so an activity leaves an axis unconstrained
		// by leaving its container empty.
		if (StationItemTags.HasAll(ExpectedOnStation)
			&& ActivityTags.HasAll(Activity->ActivityTags)
			&& InstigatorItemTags.HasAll(ExpectedInHands))
		{
			// Activities are sorted by specificity: the first match is the most specific one
			return Activity;
		}
	}
	return nullptr;
}
