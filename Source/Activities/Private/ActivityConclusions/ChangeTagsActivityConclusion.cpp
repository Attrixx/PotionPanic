// Fill out your copyright notice in the Description page of Project Settings.

#include "ActivityConclusions/ChangeTagsActivityConclusion.h"
#include "ActivityExecutionState.h"
#include "ItemActor.h"

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

	switch (Method)
	{
	case EChangeTagsMethod::DoNothing:
		break;

	case EChangeTagsMethod::SetTags:
		if (ActivityState.Item.IsValid())
			ActivityState.Item->SetItemTags(*Container);
		break;
		
	case EChangeTagsMethod::AddTags:
		if (ActivityState.Item.IsValid())
			ActivityState.Item->AppendItemTags(*Container);
		break;
		
	case EChangeTagsMethod::RemoveTags:
		if (ActivityState.Item.IsValid())
			ActivityState.Item->RemoveItemTag(*Container);
		break;
	}
}
