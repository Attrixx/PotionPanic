// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "RecipeSystem.generated.h"

class UInteractionBase;
class UItemAsset;
class UActivityAsset;
class UHolderComponent;
class RecipeAsset;

USTRUCT()
struct RECIPES_API FInteractionInfo
{
	GENERATED_BODY()
	
	UPROPERTY()
	UInteractionBase* Interaction = nullptr;
	bool bRequiresPlayerInteraction = false;
};

USTRUCT()
struct RECIPES_API FGetRecipeStepResponse
{	
	GENERATED_BODY()
public:
	UPROPERTY()
	TArray<FInteractionInfo> InteractionInfos;
	UPROPERTY()
	TObjectPtr<UItemAsset> OutputItem = nullptr;
};

/**
 * 
 */
UCLASS()
class RECIPES_API URecipeSystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	void OnWorldBeginPlay(UWorld& InWorld) override;
	
	FGetRecipeStepResponse GetRecipeStep(const TObjectPtr<UHolderComponent> StationHolder, const TArray<TObjectPtr<UActivityAsset>>& StationActivities);
	
	UFUNCTION(BlueprintCallable, Category = "Recipes|Shuffling")
	TArray<URecipeAsset*> GetShuffledRecipes(const TArray<URecipeAsset*>& InRecipes);
	
private:
	UPROPERTY()
	TObjectPtr<URecipeAsset> RecipeAsset = nullptr;
};
