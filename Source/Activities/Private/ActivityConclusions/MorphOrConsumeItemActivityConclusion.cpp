// Fill out your copyright notice in the Description page of Project Settings.

#include "ActivityConclusions/MorphOrConsumeItemActivityConclusion.h"
#include "ActivityExecutor.h"
#include "Items/Public/ItemActor.h"

DEFINE_LOG_CATEGORY_STATIC(MS_CMorphOrConsumeItem, Log, All);

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
		UE_LOGFMT(MS_CMorphOrConsumeItem, Error, "Cannot morph Null item.");
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
