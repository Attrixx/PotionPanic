// Fill out your copyright notice in the Description page of Project Settings.

#include "ActivityHolderTarget.h"
#include "ActivityExecutionState.h"
#include "HolderComponent.h"
#include "ItemActor.h"

FActivityTargetHolders::FActivityTargetHolders(const FActivityExecutionState& State, EActivityHolderTarget Target)
{
	UHolderComponent* StationHolder = State.Holder.Get();
	UHolderComponent* InstigatorHolder = State.InstigatorHolder.Get();

	if (Target == EActivityHolderTarget::Origin)
	{
		Target = State.bItemTakenFromInstigator
			? EActivityHolderTarget::InstigatorFirst
			: EActivityHolderTarget::StationFirst;
	}

	// The player who finished the activity is not the one it started with: handing the result to
	// "the" instigator would reward a passer-by, so anything but a station-only target serves the
	// station first and keeps the instigator as a fallback.
	if (State.bInstigatorChanged && Target != EActivityHolderTarget::Station)
	{
		Target = EActivityHolderTarget::StationFirst;
	}

	switch (Target)
	{
	case EActivityHolderTarget::Station:
		Holders.Add(StationHolder);
		break;

	case EActivityHolderTarget::Instigator:
		Holders.Add(InstigatorHolder);
		break;

	case EActivityHolderTarget::StationFirst:
		Holders.Add(StationHolder);
		Holders.Add(InstigatorHolder);
		break;

	case EActivityHolderTarget::InstigatorFirst:
		Holders.Add(InstigatorHolder);
		Holders.Add(StationHolder);
		break;

	default:
		checkNoEntry();
		break;
	}

	// A missing instigator is nominal (an activity can conclude on its own), so the absent holders
	// are dropped here rather than asserted on: the conclusion decides what an empty list means.
	Holders.RemoveAll([](const UHolderComponent* Holder) { return !IsValid(Holder); });
}

UHolderComponent* FActivityTargetHolders::FindFreeHolder() const
{
	for (UHolderComponent* Holder : Holders)
	{
		if (!Holder->GetCarriable())
			return Holder;
	}

	return nullptr;
}

AItemActor* FActivityTargetHolders::FindCarriedItem() const
{
	for (UHolderComponent* Holder : Holders)
	{
		if (AItemActor* Item = Cast<AItemActor>(Holder->GetCarriable()))
			return Item;
	}

	return nullptr;
}
