// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Round.generated.h"

class UStationsLayoutLayer;
class URecipeAsset;

USTRUCT(BlueprintType)
struct GAMEFLOW_API FRoundRecipe
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TSoftObjectPtr<URecipeAsset> Asset;

	// Probability that this recipe will be picked as an order
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float BaseProbability = 0.f;
};

USTRUCT(BlueprintType)
struct GAMEFLOW_API FRound
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FText RoundName;

	/**
	 * Layout layers, applied in order on top of the level's default layout.
	 * Order matters: last write wins on a given slot.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TArray<TSoftObjectPtr<UStationsLayoutLayer>> Layers;

	/** Every recipe available during this round. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TArray<FRoundRecipe> Recipes;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float Duration = 60.f;

	/** How many orders are issued during this round, evenly spread over Duration. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int32 OrderCount = 3;

	/** Rounds that may follow this one, as indices in the owning UWorldData. Empty means last round. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TArray<int32> NextRounds;
};
