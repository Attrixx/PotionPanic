// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Interactable.h"
#include "DeliveryFurniture.generated.h"

UCLASS()
class GAMEFLOW_API ADeliveryFurniture : public AActor, public IInteractable
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	ADeliveryFurniture();

protected:
	virtual void Interact_Implementation(AActor* InInstigator) override;
	virtual bool CanInteract_Implementation(AActor* InInstigator) const override;
};
