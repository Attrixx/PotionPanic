// Fill out your copyright notice in the Description page of Project Settings.

#include "ActorFilters/CarriableActorFilter.h"

#include "Carriable.h"
#include "CoreGameplayLibrary.h"
#include "HolderComponent.h"

bool UCarriableActorFilter::Matches_Implementation(AActor* Candidate) const
{
	if (!Candidate || Candidate == Ignored)
	{
		return false;
	}

	if (Candidate->Implements<UCarriable>())
	{
		return true;
	}

	const UHolderComponent* Holder = UCoreGameplayLibrary::FindComponentInAttachChain<UHolderComponent>(Candidate);
	return Holder && Holder->GetCarriable() != nullptr;
}
