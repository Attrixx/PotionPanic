#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Recipe.generated.h"

class UStationComponent;
class URecipeStep;
struct FStationStep;
struct FGameplayTag;

USTRUCT(BlueprintType)
struct FRecipe : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere)
	FText Name;

	UPROPERTY(EditAnywhere)
	FText Description;

	UPROPERTY(EditAnywhere)
	TArray<FGameplayTag> Stations;

	UPROPERTY(EditAnywhere)
	TMap<TSubclassOf<AActor>, int32> Ingredients;

	UPROPERTY(EditAnywhere)
	TArray<FStationStep> Steps;

	UPROPERTY(EditAnywhere)
	TSubclassOf<AActor> Product;

};