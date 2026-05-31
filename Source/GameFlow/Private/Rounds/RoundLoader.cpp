// Fill out your copyright notice in the Description page of Project Settings.

#include "Rounds/RoundLoader.h"
#include "Rounds/RoundContent.h"
#include "Engine/AssetManager.h"
#include "StationsLayoutSubsystem.h"
#include "RecipeSystem.h"
#include "StationsLayoutLayer.h"
#include "RecipeAsset.h"

DEFINE_LOG_CATEGORY_STATIC(MS_RoundLoader, Log, All);

URoundLoader* URoundLoader::LoadAndApplyRound(UObject* WorldContextObject, const FRoundContent& RoundContent, FOnRoundAppliedDelegate OnComplete)
{
	if (!IsValid(WorldContextObject))
	{
		UE_LOGFMT(MS_RoundLoader, Error, "Invalid WorldContextObject.");
		return nullptr;
	}

	if (URoundLoader* Loader = NewObject<URoundLoader>())
	{
		Loader->AddToRoot();
		Loader->StartLoading(WorldContextObject, RoundContent, OnComplete);
		return Loader;
	}

	return nullptr;
}

void URoundLoader::Cancel() const
{
	if (StreamableHandle.IsValid())
		StreamableHandle->CancelHandle();
}

void URoundLoader::StartLoading(UObject* WorldContextObject, const FRoundContent& RoundContent, FOnRoundAppliedDelegate OnComplete)
{
	TArray<FSoftObjectPath> Paths;
	Paths.Reserve(RoundContent.NewLayers.Num() + RoundContent.NewRecipes.Num());

	for (const TSoftObjectPtr<UStationsLayoutLayer>& Layer : RoundContent.NewLayers)
	{
		if (!Layer.IsNull())
			Paths.Add(Layer.ToSoftObjectPath());
	}

	for (const TSoftObjectPtr<URecipeAsset>& Recipe : RoundContent.NewRecipes)
	{
		if (!Recipe.IsNull())
			Paths.Add(Recipe.ToSoftObjectPath());
	}

	if (Paths.IsEmpty())
	{
		OnAssetsLoaded(WorldContextObject, RoundContent, OnComplete);
		return;
	}

	FStreamableManager& Streamable = UAssetManager::GetStreamableManager();
	StreamableHandle = Streamable.RequestAsyncLoad(
		MoveTemp(Paths),
		FStreamableDelegate::CreateUObject(this, &ThisClass::OnAssetsLoaded, TWeakObjectPtr(WorldContextObject), RoundContent, OnComplete),
		FStreamableManager::AsyncLoadHighPriority
	);
}

void URoundLoader::OnAssetsLoaded(TWeakObjectPtr<> WeakContext, FRoundContent RoundContent, FOnRoundAppliedDelegate OnComplete)
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
		UE_LOGFMT(MS_RoundLoader, Log, "Applying {0} layers.", RoundContent.NewLayers.Num());
		for (auto& LayerPtr : RoundContent.NewLayers)
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
		UE_LOGFMT(MS_RoundLoader, Log, "Adding {0} recipes.", RoundContent.NewRecipes.Num());
		for (auto& RecipePtr : RoundContent.NewRecipes)
		{
			if (URecipeAsset* Recipe = RecipePtr.Get())
			{
				RecipeSystem->AddRecipe(Recipe);
			}
			else
			{
				UE_LOGFMT(MS_RoundLoader, Warning, "Failed to resolve recipe: {0}", RecipePtr.ToString());
			}
		}
	}

	OnComplete.ExecuteIfBound();
}
