// Fill out your copyright notice in the Description page of Project Settings.

#include "DeliveryFurniture.h"
#include "AlchemyGameState.h"
#include "HolderComponent.h"

ADeliveryFurniture::ADeliveryFurniture()
{
	PrimaryActorTick.bCanEverTick = false;
}

void ADeliveryFurniture::Interact_Implementation(AActor* InInstigator)
{	
	if (!InInstigator)
		return;
	
	auto* GameState = GetWorld()->GetGameState<AAlchemyGameState>();
	check(GameState);
	
	auto* Holder = InInstigator->GetComponentByClass<UHolderComponent>();	
	if (!Holder)
		return;
	
	if (UObject* Carriable = Holder->GetCarriable())
	{
		GameState->SubmitOrderObject(Carriable);
	}
}

bool ADeliveryFurniture::CanInteract_Implementation(AActor* InInstigator) const
{
	if (!InInstigator)
		return false;
	
	auto* Holder = InInstigator->GetComponentByClass<UHolderComponent>();	
	return Holder && Holder->GetCarriable() != nullptr;
}

