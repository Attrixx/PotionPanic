// Fill out your copyright notice in the Description page of Project Settings.

#include "StationVisualActor.h"

USceneComponent* AStationVisualActor::GetItemAnchor_Implementation(FName& OutSocketName) const
{
	OutSocketName = NAME_None;
	return GetRootComponent();
}

void AStationVisualActor::Interact_Implementation(AActor* InInstigator)
{
	// This class is supposed to be instanced as a child actor.
	// Check if we can relay the interaction event.
	AActor* Parent = GetAttachParentActor();
	if (Parent && Parent->Implements<UInteractable>())
	{
		Execute_Interact(Parent, InInstigator);
	}
}
