// Fill out your copyright notice in the Description page of Project Settings.

#include "ActorFilters/InterfaceActorFilter.h"

bool UInterfaceActorFilter::Matches_Implementation(AActor* Candidate) const
{
	return Candidate && Interface && Candidate->GetClass()->ImplementsInterface(Interface);
}
