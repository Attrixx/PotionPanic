// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "InputBindable.generated.h"

class UEnhancedInputComponent;

// This class does not need to be modified.
UINTERFACE()
class UInputBindable : public UInterface
{
	GENERATED_BODY()
};

/**
 * Implement this interface on components for a callback when an input component
 * is set up by an Owner (PlayerController or Pawn).
 */
class COREGAMEPLAY_API IInputBindable
{
	GENERATED_BODY()
	
public:
	
	/** Called locally by the Owner of this component. */
	UFUNCTION(BlueprintNativeEvent)
	void SetupInputComponent(UEnhancedInputComponent* EIC);
};
