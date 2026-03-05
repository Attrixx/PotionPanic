// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "Carriable.generated.h"

// This class does not need to be modified.
UINTERFACE()
class UCarriable : public UInterface
{
	GENERATED_BODY()
};

/**
 * Represent an object that can be carried by attaching to another component.
 */
class COREGAMEPLAY_API ICarriable
{
	GENERATED_BODY()

public:
	
	UFUNCTION(BlueprintNativeEvent, Category = "Carriable")
	UPrimitiveComponent* GetPrimitive() const;
	virtual UPrimitiveComponent* GetPrimitive_Implementation() const;
	
	UFUNCTION(BlueprintNativeEvent, Category = "Carriable")
	FName GetStandaloneCollisionProfileName() const;
	virtual FName GetStandaloneCollisionProfileName_Implementation() const;
	
	UFUNCTION(BlueprintNativeEvent, Category = "Carriable")
	FName GetCarriedCollisionProfileName() const;
	virtual FName GetCarriedCollisionProfileName_Implementation() const;
	
	UFUNCTION(BlueprintNativeEvent, Category = "Carriable")
	void OnPickup(USceneComponent* AttachComponent);
	virtual void OnPickup_Implementation(USceneComponent* AttachComponent);
	
	UFUNCTION(BlueprintNativeEvent, Category = "Carriable")
	void OnDrop();
	virtual void OnDrop_Implementation();

	UFUNCTION(BlueprintNativeEvent, Category = "Carriable")
	void OnThrow(FVector Velocity);
	virtual void OnThrow_Implementation(FVector Velocity);
};
