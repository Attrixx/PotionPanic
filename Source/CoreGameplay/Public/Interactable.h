// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "Interactable.generated.h"

// This class does not need to be modified.
UINTERFACE()
class UInteractable : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class COREGAMEPLAY_API IInteractable
{
	GENERATED_BODY()

public:

	UFUNCTION(BlueprintNativeEvent)
	void Interact(AActor* InInstigator);

protected:

	virtual void Interact_Implementation(AActor* InInstigator) PURE_VIRTUAL(IInteractable::Interact,)
};
