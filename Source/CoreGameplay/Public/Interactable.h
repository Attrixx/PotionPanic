// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "Interactable.generated.h"

// This class does not need to be modified.
UINTERFACE(MinimalAPI, NotBlueprintable)
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

	UFUNCTION(BlueprintCallable)
	virtual void Interact(APlayerController* Instigator) PURE_VIRTUAL(,);
};
