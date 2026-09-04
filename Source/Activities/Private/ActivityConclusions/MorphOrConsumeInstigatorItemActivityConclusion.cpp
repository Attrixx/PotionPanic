// Fill out your copyright notice in the Description page of Project Settings.

#include "ActivityConclusions/MorphOrConsumeInstigatorItemActivityConclusion.h"
#include "ActivityExecutionState.h"
#include "HolderComponent.h"
#include "ItemActor.h"
#include "ItemAsset.h"

#if WITH_EDITOR
void UMorphOrConsumeInstigatorItemActivityConclusion::GatherItemsProducedOnSuccess(TSet<const UItemAsset*>& OutItems) const
{
	// Without a morph the item is consumed, so nothing is left behind.
	if (bMorphOnSuccess && OnActivitySuccess)
		OutItems.Add(OnActivitySuccess);
}
#endif

void UMorphOrConsumeInstigatorItemActivityConclusion::Conclude_Implementation(const FActivityExecutionState& ActivityState) const
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

	AActor* Instigator = ActivityState.LastInstigator.Get();
	check(Instigator);

	UHolderComponent* InstigatorHolder = Instigator->FindComponentByClass<UHolderComponent>();
	check(InstigatorHolder);

	// Guaranteed by the activity's steps: reaching this conclusion means the instigator still
	// carries the item it started with.
	AItemActor* CarriedItem = Cast<AItemActor>(InstigatorHolder->GetCarriable());
	check(CarriedItem);

	if (ItemAsset)
	{
		CarriedItem->SetItemAsset(ItemAsset);
	}
	else
	{
		CarriedItem->Destroy();
	}
}
