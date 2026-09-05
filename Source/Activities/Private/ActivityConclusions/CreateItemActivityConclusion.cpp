// Fill out your copyright notice in the Description page of Project Settings.

#include "ActivityConclusions/CreateItemActivityConclusion.h"
#include "ActivityExecutionState.h"
#include "ActivityHolderTarget.h"
#include "HolderComponent.h"
#include "ItemActor.h"
#include "ItemAsset.h"
#include <Misc/DataValidation.h>

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

#if WITH_EDITOR
void UCreateItemActivityConclusion::GatherItemsProducedOnSuccess(TSet<const UItemAsset*>& OutItems) const
{
	if (bCreateOnSuccess && OnActivitySuccess)
		OutItems.Add(OnActivitySuccess);
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

	// Nothing to create: do not demand a receiving holder for a no-op.
	if (!ItemAsset)
		return;

	const FActivityTargetHolders Targets(ActivityState, Target);
	if (Targets.IsEmpty())
	{
		// Target names only holders that are not there: an Instigator target on an activity that
		// concluded without one, typically.
		UE_LOGFMT(MS_CreateItem, Error, "No holder to receive the new item. Nothing created.");
		return;
	}

	UWorld* World = Targets.GetPreferred()->GetWorld();
	check(World);

	auto* NewItem = World->SpawnActor<AItemActor>(ItemClass);
	if (!NewItem)
	{
		UE_LOGFMT(MS_CreateItem, Error, "Failed to spawn item actor {0}.", ItemClass->GetName());
		return;
	}

	NewItem->SetItemAsset(ItemAsset);

	if (UHolderComponent* Receiver = Targets.FindFreeHolder())
	{
		Receiver->TryPickup(NewItem);
		return;
	}

	// Every candidate holder is already carrying something: the instigator kept its item and the
	// station stayed occupied too. The item exists and must not vanish, so it goes to the floor at
	// the preferred holder instead of being handed over.
	UE_LOGFMT(MS_CreateItem, Warning, "Every target holder is full. Dropping the new item.");
	NewItem->SetActorLocation(Targets.GetPreferred()->GetComponentLocation());
}
