// Fill out your copyright notice in the Description page of Project Settings.

#include "DeliveryFurniture.h"
#include "AlchemyGameState.h"
#include "HolderComponent.h"
#include "ItemActor.h"

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
	
	if (auto* Item = Cast<AItemActor>(Holder->GetCarriable()))
	{
		check(Item->GetItemAsset() != nullptr)
		bool bDelivered = GameState->DeliverOrder(Item->GetItemAsset());
		if (bDelivered)
		{
			// Item is consumed by the delivery
			Holder->Release();
			Item->Destroy();
		}
	}
}

bool ADeliveryFurniture::CanInteract_Implementation(AActor* InInstigator) const
{
	if (!InInstigator)
		return false;
	
	// Can't interact if holding nothing
	auto* Holder = InInstigator->GetComponentByClass<UHolderComponent>();	
	return Holder && Holder->GetCarriable() != nullptr;
}

