// Fill out your copyright notice in the Description page of Project Settings.


#include "RecipeSystem.h"
#include "CoreGameplay/Public/HolderComponent.h"
#include "CoreGameplay/Public/ActivityAsset.h"
#include "PotionPanicWorldSettings.h"
#include <algorithm>
#include <random>

#include "CarriableComponent.h"
#include "InteractionBase.h"
#include "ItemTransformation.h"
#include "Items/Public/ItemActor.h"

void URecipeSystem::OnWorldBeginPlay(UWorld& InWorld)
{
	Super::OnWorldBeginPlay(InWorld);
	
	auto Settings = Cast<APotionPanicWorldSettings>(InWorld.GetWorldSettings());
	check(Settings);
	
	if (Settings->RecipeAsset)
		RecipeAsset = Settings->RecipeAsset;
}

FGetRecipeStepResponse URecipeSystem::GetRecipeStep(const TObjectPtr<UHolderComponent> StationHolder,
                                                    const TArray<TObjectPtr<UActivityAsset>>& StationActivities)
{
	FGetRecipeStepResponse Output;
	
	TObjectPtr<UItemAsset> StationItem = nullptr;
	if (UCarriableComponent* Carriable = StationHolder->GetCarriable())
	{
		AItemActor* ItemActor = Cast<AItemActor>(Carriable->GetOwner());
		if (!ItemActor) // Carriable inside Holder is not of type Item
			return Output;
		
		StationItem = ItemActor->GetItemAsset();
	}
	
	for (auto Step : RecipeAsset->Steps)
	{		
		// Suppose there is only one possible InputItem + Activity association in RecipeAsset
		if (StationItem == Step->InputItem && StationActivities.Contains(Step->Activity))
		{
			for (auto Setting : Step->InteractionSettings)
			{
				Output.Interactions.Add(UInteractionBase::CreateInteraction(this, Setting));
			}
			
			Output.OutputItem = Step->OutputItem;
		}
	}
	
	return Output;
}

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
