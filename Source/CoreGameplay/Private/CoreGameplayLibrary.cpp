// Fill out your copyright notice in the Description page of Project Settings.

#include "CoreGameplayLibrary.h"

UActorComponent* UCoreGameplayLibrary::FindComponentInAttachChain(AActor* Actor, TSubclassOf<UActorComponent> ComponentClass)
{
	if (!ComponentClass)
	{
		return nullptr;
	}

	for (AActor* Current = Actor; Current; Current = Current->GetAttachParentActor())
	{
		if (UActorComponent* Component = Current->FindComponentByClass(ComponentClass))
		{
			return Component;
		}
	}

	return nullptr;
}
