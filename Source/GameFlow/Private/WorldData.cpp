// Fill out your copyright notice in the Description page of Project Settings.

#include "WorldData.h"

#if WITH_EDITOR
#include "Misc/DataValidation.h"

namespace
{
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
			Error(i, TEXT("Has no recipe: orders cannot be generated."));
		}

		for (const FRoundRecipe& Recipe : Round.Recipes)
		{
			if (Recipe.Asset.IsNull())
			{
				Error(i, TEXT("Contains a null recipe."));
				break;
			}
		}

		if (!Round.Recipes.ContainsByPredicate([](const FRoundRecipe& R) { return R.BaseProbability > 0.f; }))
		{
			Error(i, TEXT("Every recipe has a zero probability."));
		}

		for (const int32 NextIndex : Round.NextRounds)
		{
			if (!Rounds.IsValidIndex(NextIndex))
			{
				Error(i, *FString::Printf(TEXT("Refers to an out of bounds next round [%d]."), NextIndex));
			}
		}
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
