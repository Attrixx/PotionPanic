// Fill out your copyright notice in the Description page of Project Settings.

#include "WorldData.h"

#if WITH_EDITOR
#include "Misc/DataValidation.h"
#include "RecipeAsset.h"
#include "ActivityAsset.h"
#include "ActivityConclusion.h"
#include "ItemAsset.h"

namespace
{
/**
 * Collects every item the round's recipes can leave in the players' hands.
 * Intermediate items count: the walk cannot tell a recipe's final output from its middle steps.
 */
TSet<const UItemAsset*> GatherCraftableItems(const FRound& Round)
{
	TSet<const UItemAsset*> CraftableItems;

	for (const TSoftObjectPtr<URecipeAsset>& RoundRecipe : Round.Recipes)
	{
		const URecipeAsset* Recipe = RoundRecipe.LoadSynchronous();
		if (!Recipe)
			continue;

		for (const UActivityAsset* Step : Recipe->Steps)
		{
			if (IsValid(Step) && IsValid(Step->Conclusion))
				Step->Conclusion->GatherItemsProducedOnSuccess(CraftableItems);
		}
	}

	return CraftableItems;
}

enum class EVisitState : uint8
{
	White, // Not processed
	Gray, // Processing
	Black // Fully processed
};

/**
 * Depth-first walk of the progression graph, marking every round it can reach.
 * @param OutLoopEdges Filled with each (from, to) transition that closes a loop.
 */
void WalkRounds(int32 RoundIdx, const TArray<FRound>& Rounds, TArray<EVisitState>& States, TArray<TPair<int32, int32>>& OutLoopEdges)
{
	States[RoundIdx] = EVisitState::Gray;

	for (const int32 NextIdx : Rounds[RoundIdx].NextRounds)
	{
		if (!Rounds.IsValidIndex(NextIdx))
			continue; // Already reported as an out-of-bounds reference

		switch (States[NextIdx])
		{
		case EVisitState::White:
			WalkRounds(NextIdx, Rounds, States, OutLoopEdges);
			break;

		case EVisitState::Gray:
			// Still being processed, so this transition leads back into the current path
			OutLoopEdges.Emplace(RoundIdx, NextIdx);
			break;

		case EVisitState::Black:
			// The graph is diamond-shaped: two paths lead to the same round, which is fine
			break;
		}
	}

	States[RoundIdx] = EVisitState::Black;
}
} // namespace

EDataValidationResult UWorldData::IsDataValid(FDataValidationContext& Context) const
{
	EDataValidationResult Result = Super::IsDataValid(Context);

	if (Rounds.IsEmpty())
	{
		Context.AddError(FText::FromString("At least one round is expected."));
		return EDataValidationResult::Invalid;
	}

	auto Error = [&](int32 RoundIdx, const TCHAR* Message)
	{
		Context.AddError(FText::FromString(FString::Format(TEXT("Round [{0}]: {1}"), {RoundIdx, Message})));
		Result = EDataValidationResult::Invalid;
	};

	for (int32 i = 0; i < Rounds.Num(); ++i)
	{
		const FRound& Round = Rounds[i];

		if (Round.Duration <= 0.f)
			Error(i, TEXT("Duration must be greater than zero."));

		if (Round.OrderCount <= 0)
			Error(i, TEXT("OrderCount must be greater than zero."));

		for (const TSoftObjectPtr<UStationsLayoutLayer>& Layer : Round.Layers)
		{
			if (Layer.IsNull())
			{
				Error(i, TEXT("Contains a null layer."));
				break;
			}
		}

		if (Round.Recipes.IsEmpty())
		{
			Error(i, TEXT("Has no recipe: nothing can be crafted."));
		}

		for (const TSoftObjectPtr<URecipeAsset>& Recipe : Round.Recipes)
		{
			if (Recipe.IsNull())
			{
				Error(i, TEXT("Contains a null recipe."));
				break;
			}
		}

		if (Round.Orderables.IsEmpty())
		{
			Error(i, TEXT("Has no orderable item: orders cannot be generated."));
		}

		for (const FRoundOrderable& Orderable : Round.Orderables)
		{
			if (Orderable.Asset.IsNull())
			{
				Error(i, TEXT("Contains a null orderable item."));
				break;
			}
		}

		for (const FRoundOrderable& Orderable : Round.Orderables)
		{
			// An order with no time to run is placed and expired in the same update, and scores
			// nothing: ScoreForOrder returns 0 on a non-positive duration.
			if (Orderable.TimeToComplete <= 0.f)
			{
				Error(i, TEXT("Contains an orderable item whose TimeToComplete is not greater than zero."));
				break;
			}
		}

		if (!Round.Orderables.ContainsByPredicate([](const FRoundOrderable& O) { return O.BaseProbability > 0.f; }))
		{
			Error(i, TEXT("Every orderable item has a zero probability."));
		}

		const TSet<const UItemAsset*> CraftableItems = GatherCraftableItems(Round);
		for (const FRoundOrderable& Orderable : Round.Orderables)
		{
			const UItemAsset* Item = Orderable.Asset.LoadSynchronous();
			if (Item && !CraftableItems.Contains(Item))
			{
				// A warning, not an error: Blueprint conclusions cannot report what they produce,
				// so a round relying on one would be flagged despite being valid.
				Context.AddWarning(FText::FromString(FString::Printf(
					TEXT("Round [%d]: orderable item '%s' is not produced by any of its recipes."),
					i, *Item->GetName())));
			}
		}

		if (Round.MinOrderScore > Round.MaxOrderScore)
			Error(i, TEXT("MinOrderScore is above MaxOrderScore: a late delivery would beat an early one."));

		for (const int32 NextIndex : Round.NextRounds)
		{
			if (!Rounds.IsValidIndex(NextIndex))
			{
				Error(i, *FString::Printf(TEXT("Refers to an out of bounds next round [%d]."), NextIndex));
			}
		}
	}

	// Summed over every round rather than over one path through them: a target above even that
	// cannot be reached whichever branches the run takes.
	int64 MaxReachableScore = 0;
	for (const FRound& Round : Rounds)
		MaxReachableScore += static_cast<int64>(Round.OrderCount) * Round.MaxOrderScore;

	if (ScoreToSucceed > MaxReachableScore)
	{
		Context.AddError(FText::FromString(FString::Printf(
			TEXT("ScoreToSucceed (%lld) is above what every round together can award (%lld): this world cannot be won."),
			ScoreToSucceed, MaxReachableScore)));
		Result = EDataValidationResult::Invalid;
	}

	// Walk the progression graph from the first round: any round left White is unreachable.
	TArray<EVisitState> States;
	States.Init(EVisitState::White, Rounds.Num());

	TArray<TPair<int32, int32>> LoopEdges;
	WalkRounds(0, Rounds, States, LoopEdges);

	for (const TPair<int32, int32>& LoopEdge : LoopEdges)
	{
		Error(LoopEdge.Key, *FString::Printf(TEXT("Loops back to round [%d]."), LoopEdge.Value));
	}

	for (int32 i = 1; i < States.Num(); ++i)
	{
		if (States[i] == EVisitState::White)
		{
			Context.AddWarning(FText::FromString(FString::Format(
				TEXT("Round [{0}] cannot be reached from the first round."),
				{i})));
		}
	}

	return Result;
}
#endif
