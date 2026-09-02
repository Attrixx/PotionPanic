// Fill out your copyright notice in the Description page of Project Settings.

#include "Rounds/RoundLoader.h"
#include "Rounds/Round.h"
#include "Engine/AssetManager.h"
#include "StationsLayoutSubsystem.h"
#include "RecipeSystem.h"
#include "StationsLayoutLayer.h"
#include "RecipeAsset.h"

DEFINE_LOG_CATEGORY_STATIC(MS_RoundLoader, Log, All);

URoundLoader* URoundLoader::LoadAndApplyRound(UObject* WorldContextObject, const FRound& Round, FOnRoundAppliedDelegate OnComplete)
{
	if (!IsValid(WorldContextObject))
	{
		UE_LOGFMT(MS_RoundLoader, Error, "Invalid WorldContextObject.");
		return nullptr;
	}

	if (URoundLoader* Loader = NewObject<URoundLoader>())
	{
		Loader->AddToRoot();
		Loader->StartLoading(WorldContextObject, Round, OnComplete);
		return Loader;
	}

	return nullptr;
}

void URoundLoader::Cancel() const
{
	if (StreamableHandle.IsValid())
		StreamableHandle->CancelHandle();
}

void URoundLoader::StartLoading(UObject* WorldContextObject, const FRound& Round, FOnRoundAppliedDelegate OnComplete)
{
	TArray<FSoftObjectPath> Paths;
	Paths.Reserve(Round.Layers.Num() + Round.Recipes.Num() + Round.Orderables.Num());

	for (const TSoftObjectPtr<UStationsLayoutLayer>& Layer : Round.Layers)
	{
		if (!Layer.IsNull())
			Paths.Add(Layer.ToSoftObjectPath());
	}

	for (const FRoundRecipe& RoundRecipe : Round.Recipes)
	{
		if (!RoundRecipe.Asset.IsNull())
			Paths.Add(RoundRecipe.Asset.ToSoftObjectPath());
	}

	// Orders resolve their item straight from the round data, so it has to be loaded by then.
	for (const FRoundOrderable& Orderable : Round.Orderables)
	{
		if (!Orderable.Asset.IsNull())
			Paths.Add(Orderable.Asset.ToSoftObjectPath());
	}

	if (Paths.IsEmpty())
	{
		OnAssetsLoaded(WorldContextObject, Round, OnComplete);
		return;
	}

	FStreamableManager& Streamable = UAssetManager::GetStreamableManager();
	StreamableHandle = Streamable.RequestAsyncLoad(
		MoveTemp(Paths),
		FStreamableDelegate::CreateUObject(this, &ThisClass::OnAssetsLoaded, TWeakObjectPtr(WorldContextObject), Round, OnComplete),
		FStreamableManager::AsyncLoadHighPriority
	);
}

void URoundLoader::OnAssetsLoaded(TWeakObjectPtr<> WeakContext, FRound Round, FOnRoundAppliedDelegate OnComplete)
{
	ON_SCOPE_EXIT
	{
		StreamableHandle.Reset();
		RemoveFromRoot();
	};

	if (!WeakContext.IsValid())
	{
		UE_LOGFMT(MS_RoundLoader, Warning, "WorldContextObject was destroyed before load completion.");
		return;
	}

	UWorld* World = WeakContext->GetWorld();
	if (!World)
	{
		UE_LOGFMT(MS_RoundLoader, Error, "WorldContextObject could not yield a valid UWorld.");
		return;
	}

	if (UStationsLayoutSubsystem* LayoutSubsystem = World->GetSubsystem<UStationsLayoutSubsystem>())
	{
		UE_LOGFMT(MS_RoundLoader, Log, "Applying {0} layers.", Round.Layers.Num());
		LayoutSubsystem->ResetToDefaultLayout();

		for (auto& LayerPtr : Round.Layers)
		{
			if (UStationsLayoutLayer* Layer = LayerPtr.Get())
			{
				LayoutSubsystem->ApplyLayer(Layer);
			}
			else
			{
				UE_LOGFMT(MS_RoundLoader, Warning, "Failed to resolve layer: {0}", LayerPtr.ToString());
			}
		}
	}

	if (URecipeSystem* RecipeSystem = World->GetSubsystem<URecipeSystem>())
	{
		UE_LOGFMT(MS_RoundLoader, Log, "Adding {0} recipes.", Round.Recipes.Num());
		RecipeSystem->ClearRecipes();

		for (auto& RoundRecipe : Round.Recipes)
		{
			if (URecipeAsset* Recipe = RoundRecipe.Asset.Get())
			{
				RecipeSystem->AddRecipe(Recipe);
			}
			else
			{
				UE_LOGFMT(MS_RoundLoader, Warning, "Failed to resolve recipe: {0}", RoundRecipe.Asset.ToString());
			}
		}
	}

	OnComplete.ExecuteIfBound();
}
