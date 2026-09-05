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
	
	UFUNCTION(BlueprintNativeEvent, Category = "Carriable")
	FName GetStandaloneCollisionProfileName() const;
	
	UFUNCTION(BlueprintNativeEvent, Category = "Carriable")
	FName GetCarriedCollisionProfileName() const;
	
	UFUNCTION(BlueprintNativeEvent, Category = "Carriable")
	void OnPickup(USceneComponent* AttachComponent);
	
	UFUNCTION(BlueprintNativeEvent, Category = "Carriable")
	void OnDrop();

	UFUNCTION(BlueprintNativeEvent, Category = "Carriable")
	void OnThrow(FVector Velocity);

	/** @return Whether a holder is allowed to throw this Carriable away. Defaults to true. */
	UFUNCTION(BlueprintNativeEvent, Category = "Carriable")
	bool CanBeThrown() const;
	
protected: // Default implementations
	
	virtual UPrimitiveComponent* GetPrimitive_Implementation() const;
	virtual FName GetStandaloneCollisionProfileName_Implementation() const;
	virtual FName GetCarriedCollisionProfileName_Implementation() const;
	virtual void OnPickup_Implementation(USceneComponent* AttachComponent);
	virtual void OnDrop_Implementation();
	virtual void OnThrow_Implementation(FVector Velocity);
	virtual bool CanBeThrown_Implementation() const;
};
