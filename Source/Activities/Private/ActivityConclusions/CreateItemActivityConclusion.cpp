// Fill out your copyright notice in the Description page of Project Settings.


#include "ActivityConclusions/CreateItemActivityConclusion.h"
#include "HolderComponent.h"
#include "ItemAsset.h"
#include "Items/Public/ItemActor.h"
#include "Misc/DataValidation.h"

DEFINE_LOG_CATEGORY_STATIC(MS_CreateItem, Log, All);

#if WITH_EDITOR
EDataValidationResult UCreateItemActivityConclusion::IsDataValid(FDataValidationContext& Context) const
{
	EDataValidationResult Result = Super::IsDataValid(Context);

	if (!IsValid(ItemClass))
	{
		Context.AddError(FText::FromString("ItemClass is null."));
		Result = EDataValidationResult::Invalid;
	}

	if (bCreateOnSuccess && !IsValid(OnActivitySuccess))
	{
		Context.AddError(FText::FromString("OnActivitySuccess is enabled but null."));
		Result = EDataValidationResult::Invalid;
	}

	if (bCreateOnFail && !IsValid(OnActivityFail))
	{
		Context.AddError(FText::FromString("OnActivityFail is enabled but null."));
		Result = EDataValidationResult::Invalid;
	}

	return Result;
}
#endif

void UCreateItemActivityConclusion::Conclude_Implementation(const FActivityExecutionState& ActivityState) const
{
	UItemAsset* ItemAsset = nullptr;
	switch (ActivityState.Status)
	{
	case EActivityExecutionStatus::Success:
		if (bCreateOnSuccess)
			ItemAsset = OnActivitySuccess;
		break;

	case EActivityExecutionStatus::Failed:
		if (bCreateOnFail)
			ItemAsset = OnActivityFail;
		break;

	default:
		checkNoEntry();
		return;
	}

	if (!ItemAsset)
		return;

	UWorld* World = ActivityState.Holder->GetWorld();
	check(World);

	if (auto* NewItem = World->SpawnActor<AItemActor>(ItemClass))
	{
		NewItem->SetItemAsset(ItemAsset);
		if (!ActivityState.Holder->TryPickup(NewItem))
		{
			UE_LOGFMT(MS_CreateItem, Error, "Holder cannot pickup the new item. Teleporting it.");
			NewItem->SetActorLocation(ActivityState.Holder->GetComponentLocation());
		}
	}
	else
	{
		UE_LOGFMT(MS_CreateItem, Error, "Failed to spawn item actor {0}.", ItemClass->GetName());
	}
}
