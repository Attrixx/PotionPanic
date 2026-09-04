// Fill out your copyright notice in the Description page of Project Settings.

#include "ActivityConclusions/ChangeInstigatorItemTagsActivityConclusion.h"
#include "ActivityExecutionState.h"
#include "HolderComponent.h"
#include "ItemActor.h"

void UChangeInstigatorItemTagsActivityConclusion::Conclude_Implementation(const FActivityExecutionState& ActivityState) const
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

	// Nothing to touch: do not demand an instigator holding an item for a no-op.
	if (Method == EChangeTagsMethod::DoNothing)
		return;

	AActor* Instigator = ActivityState.LastInstigator.Get();
	check(Instigator);

	UHolderComponent* InstigatorHolder = Instigator->FindComponentByClass<UHolderComponent>();
	check(InstigatorHolder);

	// Guaranteed by the activity's steps: reaching this conclusion means the instigator still
	// carries the item it started with.
	AItemActor* CarriedItem = Cast<AItemActor>(InstigatorHolder->GetCarriable());
	check(CarriedItem);

	switch (Method)
	{
	case EChangeTagsMethod::SetTags:
		CarriedItem->SetItemTags(*Container);
		break;

	case EChangeTagsMethod::AddTags:
		CarriedItem->AppendItemTags(*Container);
		break;

	case EChangeTagsMethod::RemoveTags:
		CarriedItem->RemoveItemTag(*Container);
		break;

	default:
		checkNoEntry();
		break;
	}
}
