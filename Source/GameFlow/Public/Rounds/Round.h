// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Round.generated.h"

class UStationsLayoutLayer;
class URecipeAsset;
class UItemAsset;

/** An item customers may ask for during the round, and how often they ask for it. */
USTRUCT(BlueprintType)
struct GAMEFLOW_API FRoundOrderable
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TSoftObjectPtr<UItemAsset> Asset;

	/**
	 * Weight of this item relative to the round's other orderables, not a percentage:
	 * items weighted 4 and 10 are ordered 4 and 10 times per 14 orders.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float BaseProbability = 0.f;

	/**
	 * How long, in seconds, customers wait for this item once its order is placed. It is also what
	 * the order's score is measured against: delivered the instant it is placed it is worth the
	 * round's MaxOrderScore, delivered as this runs out, MinOrderScore.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (ClampMin = 0))
	float TimeToComplete = 30.f;
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

	/** Every recipe available during this round: what the players are able to craft. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TArray<TSoftObjectPtr<URecipeAsset>> Recipes;

	/** Every item this round may order, weighted. Each one should be craftable through Recipes. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TArray<FRoundOrderable> Orderables;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float Duration = 60.f;

	/** How many orders are issued during this round, evenly spread over Duration. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int32 OrderCount = 3;

	/**
	 * Longest stretch, in seconds, this round may leave the players without a placed order.
	 * Completing the last placed order pulls every pending one forward so the next is placed
	 * no later than this. Zero places it at once; a value >= Duration keeps the even spread.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (ClampMin = 0))
	float MaxTimeWithoutPlacedOrder = 3.f;

	/**
	 * Points a completed order is worth when it is delivered the instant it is placed.
	 * An order delivered later is worth less, down to MinOrderScore on the deadline.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Score", meta = (ClampMin = 0))
	int32 MaxOrderScore = 100;

	/** Points a completed order is worth when it is delivered as its timer runs out. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Score", meta = (ClampMin = 0))
	int32 MinOrderScore = 25;

	/** Rounds that may follow this one, as indices in the owning UWorldData. Empty means last round. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TArray<int32> NextRounds;
};
