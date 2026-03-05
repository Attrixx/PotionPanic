// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "Carriable.generated.h"

class UHolderComponent;

// This class does not need to be modified.
UINTERFACE()
class UCarriable : public UInterface
{
	GENERATED_BODY()
};

/**
 * Represent an object that can be carried by attaching/following to another component.
 */
class COREGAMEPLAY_API ICarriable
{
	GENERATED_BODY()

public:
	
	UFUNCTION(BlueprintNativeEvent, Category = "Carriable")
	USceneComponent* GetAttachComponent();
	
	// Pickup is manually called by some user input
	UFUNCTION(BlueprintNativeEvent, Category = "Carriable")
	bool TryPickup(USceneComponent* AttachComponent);
	
	// Catch is automatically called from a specific world state
	UFUNCTION(BlueprintNativeEvent, Category = "Carriable")
	bool TryCatch(USceneComponent* AttachComponent);
	
	// Reattach to another component, without going through an intermediate detached state
	UFUNCTION(BlueprintNativeEvent, Category = "Carriable")
	bool TryTransfer(USceneComponent* AttachComponent);
	
	// Detach and drops the object. Cannot fail.
	UFUNCTION(BlueprintNativeEvent, Category = "Carriable")
	void Drop();

	// Detach and throw the object. Cannot fail.
	UFUNCTION(BlueprintNativeEvent, Category = "Carriable")
	void Throw(FVector Velocity);
};
