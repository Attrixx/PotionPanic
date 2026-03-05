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
#include "InteractionSettingBase.h"
#include "Items/Public/ItemActor.h"

void URecipeSystem::OnWorldBeginPlay(UWorld& InWorld)
{
	Super::OnWorldBeginPlay(InWorld);
	
	// TODO: remove dependency to world settings (going to be moved into different module)
	// Should probably expose Init(RecipeAsset) method instead to be called from game mode 
	auto Settings = Cast<APotionPanicWorldSettings>(InWorld.GetWorldSettings());
	check(Settings);

	if (Settings->RecipeAsset)
		RecipeAsset = Settings->RecipeAsset;
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
	if (UCarriableComponent* Carriable = StationHolder->GetCarriable())
	{
		AItemActor* ItemActor = Cast<AItemActor>(Carriable->GetOwner());
		if (!ItemActor) // Carriable inside Holder is not of type Item
			return Output;
		
		StationItem = ItemActor->GetItemAsset();
	}
	
	// When StationItem == nullptr, we still want to go through this loop (Spawner case)
	for (auto Step : RecipeAsset->Steps)
	{
		if (!Step)
		{
			continue;
		}

		if (StationItem == Step->InputItem && StationActivities.Contains(Step->Activity))
		{
			for (auto Setting : Step->InteractionSettings)
			{
				if (UInteractionBase* Interaction = UInteractionBase::CreateInteraction(this, Setting))
				{					
					FInteractionInfo& Info = Output.InteractionInfos.Emplace_GetRef();
					Info.Interaction = Interaction;
					Info.bRequiresPlayerInteraction = Setting->bRequiresPlayerInteraction;
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
