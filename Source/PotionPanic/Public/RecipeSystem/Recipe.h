#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Recipe.generated.h"

class UStationComponent;
class UItemComponent;
class URecipeStep;

UCLASS()
class URecipe : public UDataAsset
{
	GENERATED_BODY()

public:

	// The stations where this recipe can be performed
	UPROPERTY(EditAnywhere)
	TArray<TSubclassOf<UStationComponent>> Stations;

	// Ingredients needed to perform the recipe
	UPROPERTY(EditAnywhere)
	TArray<TSubclassOf<UItemComponent>> Ingredients;

	// Steps to be followed to perform the recipe
	UPROPERTY(EditAnywhere)
	TArray<TSubclassOf<URecipeStep>> Steps;

	// Products given out after performing the recipe
	UPROPERTY(EditAnywhere)
	TArray<TSubclassOf<UItemComponent>> Products;
};
