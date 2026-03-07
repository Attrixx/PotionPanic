// Fill out your copyright notice in the Description page of Project Settings.


#include "RecipeSystem.h"
#include "CoreGameplay/Public/HolderComponent.h"
#include "CoreGameplay/Public/ActivityAsset.h"
#include <algorithm>
#include <random>

#include "RecipeAsset.h"
#include "ActivityStep.h"
#include "ItemTransformation.h"
#include "ActivityStepSettings.h"
#include "Items/Public/ItemActor.h"

DEFINE_LOG_CATEGORY_STATIC(MS_RecipeSystem, Log, All);

void URecipeSystem::SetRecipes(URecipeAsset* NewRecipeAsset)
{
	RecipeAsset = NewRecipeAsset;
}

FGetRecipeStepResponse URecipeSystem::GetRecipeStep(const TObjectPtr<UHolderComponent> StationHolder,
                                                    const TArray<TObjectPtr<UActivityAsset>>& StationActivities)
{
	FGetRecipeStepResponse Output;

	if (!StationHolder)
	{
		return Output;
	}

	if (!RecipeAsset)
	{
		return Output;
	}
	
	TObjectPtr<UItemAsset> StationItem = nullptr;
	if (UObject* Carriable = StationHolder->GetCarriable())
	{
		AItemActor* ItemActor = Cast<AItemActor>(Carriable);
		if (!ItemActor) // Carriable inside Holder is not of type Item
			return Output;
		
		StationItem = ItemActor->GetItemAsset();
	}
	
	// When StationItem == nullptr, we still want to go through this loop (Spawner case)
	for (UItemTransformation* Step : RecipeAsset->Steps)
	{
		if (!Step)
		{
			continue;
		}

		if (StationItem == Step->InputItem && StationActivities.Contains(Step->Activity))
		{
			Output.ActivitySteps.Reserve(Step->ActivitySteps.Num());
			for (UActivityStepSettings* Setting : Step->ActivitySteps)
			{
				if (!Setting)
				{
					UE_LOGFMT(MS_RecipeSystem, Warning, "Encountered null StepSettings while creating steps.");
					continue;
				}
				
				if (UActivityStep* ActivityStep = Setting->CreateStep(this))
				{					
					Output.ActivitySteps.Emplace(ActivityStep);
				}
				else
				{
					UE_LOGFMT(MS_RecipeSystem, Warning, "{0} created a null step.", Setting->GetName());
				}
			}
			
			Output.OutputItem = Step->OutputItem;
			// Suppose there is only one possible InputItem + Activity association in RecipeAsset
			break;
		}
	}
	
	return Output;
}

TArray<URecipeAsset*> URecipeSystem::GetShuffledRecipes(const TArray<URecipeAsset*>& InRecipes)
{
	// TODO Ref this, kept in code for now cause might be useful
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
