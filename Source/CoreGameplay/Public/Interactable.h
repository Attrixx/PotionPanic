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

	UFUNCTION(BlueprintNativeEvent)
	bool CanInteract(AActor* InInstigator) const;

protected:

	virtual void Interact_Implementation(AActor* InInstigator) PURE_VIRTUAL(IInteractable::Interact,)
	virtual bool CanInteract_Implementation(AActor* InInstigator) const;
};
