// Fill out your copyright notice in the Description page of Project Settings.

#include "ActivityConclusions/MorphOrConsumeItemActivityConclusion.h"
#include "ActivityExecutionState.h"
#include "ItemActor.h"
#include "ItemAsset.h"

DEFINE_LOG_CATEGORY_STATIC(MS_MorphOrConsumeItem, Log, All);

#if WITH_EDITOR
void UMorphOrConsumeItemActivityConclusion::GatherItemsProducedOnSuccess(TSet<const UItemAsset*>& OutItems) const
{
	// Without a morph the item is consumed, so nothing is left behind.
	if (bMorphOnSuccess && OnActivitySuccess)
		OutItems.Add(OnActivitySuccess);
}
#endif

void UMorphOrConsumeItemActivityConclusion::Conclude_Implementation(const FActivityExecutionState& ActivityState) const
{
	UItemAsset* ItemAsset = nullptr;
	switch (ActivityState.Status)
	{
	case EActivityExecutionStatus::Success:
		if (!bMorphOnSuccess)
			return;
		ItemAsset = OnActivitySuccess;
		break;

	case EActivityExecutionStatus::Failed:
		if (!bMorphOnFail)
			return;
		ItemAsset = OnActivityFail;
		break;

	default:
		checkNoEntry();
		return;
	}

	if (!ActivityState.Item.IsValid())
	{
		UE_LOGFMT(MS_MorphOrConsumeItem, Error, "Cannot morph Null item.");
		return;
	}

	if (ItemAsset)
	{
		ActivityState.Item->SetItemAsset(ItemAsset);
	}
	else
	{
		ActivityState.Item->Destroy();
	}
}
