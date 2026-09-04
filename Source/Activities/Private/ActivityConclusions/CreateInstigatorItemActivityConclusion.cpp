// Fill out your copyright notice in the Description page of Project Settings.

#include "ActivityConclusions/CreateInstigatorItemActivityConclusion.h"
#include "ActivityExecutionState.h"
#include "HolderComponent.h"
#include "ItemActor.h"
#include "ItemAsset.h"
#include <Misc/DataValidation.h>

#if WITH_EDITOR
EDataValidationResult UCreateInstigatorItemActivityConclusion::IsDataValid(FDataValidationContext& Context) const
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
void UCreateInstigatorItemActivityConclusion::GatherItemsProducedOnSuccess(TSet<const UItemAsset*>& OutItems) const
{
	if (bCreateOnSuccess && OnActivitySuccess)
		OutItems.Add(OnActivitySuccess);
}
#endif

void UCreateInstigatorItemActivityConclusion::Conclude_Implementation(const FActivityExecutionState& ActivityState) const
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

	// Nothing to create: do not demand an instigator with a free holder for a no-op.
	if (!ItemAsset)
		return;

	AActor* Instigator = ActivityState.LastInstigator.Get();
	check(Instigator);

	UHolderComponent* InstigatorHolder = Instigator->FindComponentByClass<UHolderComponent>();
	check(InstigatorHolder);

	UWorld* World = InstigatorHolder->GetWorld();
	check(World);

	AItemActor* NewItem = World->SpawnActor<AItemActor>(ItemClass);
	check(NewItem);

	NewItem->SetItemAsset(ItemAsset);

	// The holder must be free by now: whatever the instigator carried into the activity was
	// consumed by it. verify, not check: the pickup must still happen in shipping builds.
	verify(InstigatorHolder->TryPickup(NewItem));
}
