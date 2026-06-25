// Fill out your copyright notice in the Description page of Project Settings.

#include "ActorFilters/InteractableActorFilter.h"

#include "Interactable.h"

bool UInteractableActorFilter::Matches_Implementation(AActor* Candidate) const
{
	return Candidate && Candidate->Implements<UInteractable>() && IInteractable::Execute_CanInteract(Candidate, Instigator);
}
