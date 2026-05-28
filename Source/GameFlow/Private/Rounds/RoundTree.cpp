// Fill out your copyright notice in the Description page of Project Settings.

#include "Rounds/RoundTree.h"

void URoundTree::PostInitProperties()
{
	Super::PostInitProperties();
	EnsureRootExists();
}

void URoundTree::PostLoad()
{
	Super::PostLoad();
	EnsureRootExists();
}

#if WITH_EDITOR
#include "Misc/DataValidation.h"

namespace
{
enum class EVisitState : uint8
{
	White, // Not Processed
	Gray, // Processing
	Black // Fully processed
};

bool HasCycle(int32 NodeIndex, const TArray<FRoundNode>& Nodes, TArray<EVisitState>& States)
{
	States[NodeIndex] = EVisitState::Gray;

	for (const int32 ChildIdx : Nodes[NodeIndex].ChildIndices)
	{
		if (!Nodes.IsValidIndex(ChildIdx))
			continue;

		if (States[ChildIdx] == EVisitState::Gray)
		{
			// Already processing, this means there is a cycle
			return true;
		}

		if (States[ChildIdx] == EVisitState::White)
		{
			if (HasCycle(ChildIdx, Nodes, States))
				return true;
		}

		// else (black) means the tree is diamond-shaped: two paths lead to the same node
	}

	States[NodeIndex] = EVisitState::Black;
	return false;
}
} // namespace

EDataValidationResult URoundTree::IsDataValid(FDataValidationContext& Context) const
{
	EDataValidationResult Result = Super::IsDataValid(Context);

	if (Nodes.IsEmpty())
	{
		Context.AddError(FText::FromString("At least one (root) node is expected."));
		return EDataValidationResult::Invalid;
	}

	for (int32 i = 0; i < Nodes.Num(); ++i)
	{
		for (int32 ChildIdx : Nodes[i].ChildIndices)
		{
			if (!Nodes.IsValidIndex(ChildIdx))
			{
				Context.AddError(FText::FromString(FString::Format(
					TEXT("Node [{0}] has invalid child index [{1}]."),
					{i, ChildIdx})));
				Result = EDataValidationResult::Invalid;
			}
		}
	}

	TArray<EVisitState> States;
	States.Init(EVisitState::White, Nodes.Num());

	if (HasCycle(0, Nodes, States))
	{
		Context.AddError(FText::FromString("Cycle detected."));
		Result = EDataValidationResult::Invalid;
	}

	for (int32 i = 1; i < States.Num(); ++i)
	{
		if (States[i] == EVisitState::White)
		{
			Context.AddWarning(FText::FromString(FString::Format(
				TEXT("Node [{0}] cannot be reached."),
				{i})));
		}
	}

	return Result;
}

void URoundTree::PostEditChangeProperty(FPropertyChangedEvent& Event)
{
	Super::PostEditChangeProperty(Event);
	
	if (Event.GetPropertyName() == GET_MEMBER_NAME_CHECKED(URoundTree, Nodes))
	{
		EnsureRootExists();
	}
}
#endif

void URoundTree::EnsureRootExists()
{
	if (Nodes.IsEmpty())
	{
		Nodes.Emplace();
	}
}
