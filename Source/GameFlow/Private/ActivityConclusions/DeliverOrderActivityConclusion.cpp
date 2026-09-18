// Fill out your copyright notice in the Description page of Project Settings.

#include "ActivityConclusions/DeliverOrderActivityConclusion.h"
#include "AlchemyGameState.h"
#include "ActivityExecutionState.h"
#include "ItemActor.h"
#include "ItemAsset.h"
#include "HolderComponent.h"

DEFINE_LOG_CATEGORY_STATIC(MS_DeliverOrder, Log, All);

void UDeliverOrderActivityConclusion::Conclude_Implementation(const FActivityExecutionState& State) const
{
	if (State.Status != EActivityExecutionStatus::Success)
	{
		return;
	}

	AItemActor* Item = State.Item.Get();
	if (!Item)
	{
		UE_LOGFMT(MS_DeliverOrder, Error, "The activity concluded without an item to deliver.");
		return;
	}

	AAlchemyGameState* GameState = Item->GetWorld()->GetGameState<AAlchemyGameState>();
	if (!GameState)
	{
		UE_LOGFMT(MS_DeliverOrder, Error, "No AAlchemyGameState to deliver to.");
		return;
	}

	// The holder belongs to the station the item was handed to: where the points show up.
	AActor* DeliveredAt = State.Holder.IsValid() ? State.Holder->GetOwner() : nullptr;

	if (GameState->DeliverOrder(Item->GetItemAsset(), DeliveredAt))
	{
		Item->Destroy();
	}
	else
	{
		UE_LOGFMT(MS_DeliverOrder, Log, "{0} failed to deliver, it was not ordered.", 
			Item->GetItemAsset() ? Item->GetItemAsset()->ItemName.ToString() : "NULL");
	}
}
