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
 * 
 */
class COREGAMEPLAY_API ICarriable
{
	GENERATED_BODY()

public:
	
	// Pickup is manually called by some user input
	UFUNCTION(BlueprintNativeEvent, Category = "Carriable")
	bool TryPickup(USceneComponent* AttachComponent);
	
	// Catch is automatically called from a specific world state
	UFUNCTION(BlueprintNativeEvent, Category = "Carriable")
	bool TryCatch(USceneComponent* AttachComponent);
	
	UFUNCTION(BlueprintNativeEvent, Category = "Carriable")
	void Drop();

	UFUNCTION(BlueprintNativeEvent, Category = "Carriable")
	void Throw(FVector Velocity);
};
