// Fill out your copyright notice in the Description page of Project Settings.

#include "ActorFilters/FreeHolderActorFilter.h"

#include "CoreGameplayLibrary.h"
#include "HolderComponent.h"

bool UFreeHolderActorFilter::Matches_Implementation(AActor* Candidate) const
{
	const UHolderComponent* Holder = UCoreGameplayLibrary::FindComponentInAttachChain<UHolderComponent>(Candidate);
	return Holder && Holder->GetOwner() != Ignored && !Holder->GetCarriable();
}
