// Fill out your copyright notice in the Description page of Project Settings.

#include "ActivityConclusions/ChangeTagsActivityConclusion.h"
#include "ActivityExecutionState.h"
#include "ActivityHolderTarget.h"
#include "ItemActor.h"

DEFINE_LOG_CATEGORY_STATIC(MS_ChangeTags, Log, All);

void UChangeTagsActivityConclusion::Conclude_Implementation(const FActivityExecutionState& ActivityState) const
{
	EChangeTagsMethod Method = EChangeTagsMethod::DoNothing;
	const FGameplayTagContainer* Container = nullptr;
	switch (ActivityState.Status)
	{
	case EActivityExecutionStatus::Success:
		Method = OnSuccessMethod;
		Container = &OnSuccessTags;
		break;

	case EActivityExecutionStatus::Failed:
		Method = OnFailedMethod;
		Container = &OnFailedTags;
		break;

	default:
		checkNoEntry();
		return;
	}

	// Nothing to touch: do not demand a holder carrying an item for a no-op.
	if (Method == EChangeTagsMethod::DoNothing)
		return;

	const FActivityTargetHolders Targets(ActivityState, Target);
	AItemActor* TargetItem = Targets.FindCarriedItem();
	if (!TargetItem)
	{
		UE_LOGFMT(MS_ChangeTags, Error, "Cannot change the tags of a Null item.");
		return;
	}

	switch (Method)
	{
	case EChangeTagsMethod::SetTags:
		TargetItem->SetItemTags(*Container);
		break;

	case EChangeTagsMethod::AddTags:
		TargetItem->AppendItemTags(*Container);
		break;

	case EChangeTagsMethod::RemoveTags:
		TargetItem->RemoveItemTag(*Container);
		break;

	default:
		checkNoEntry();
		break;
	}
}
