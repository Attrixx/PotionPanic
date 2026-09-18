// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PotionPanicActivatableWidget.h"
#include "AlchemistHUDWidget.generated.h"

/**
 * Root of the in-game UI, active for the whole level and asking for game input. Every screen
 * that takes the input over (round choice, end screen) lives inside it and activates as one of
 * its children: when such a screen deactivates, CommonUI falls back to this root and hands the
 * input back to the game. Without a root, nothing would, and the cursor would stay up.
 */
UCLASS(Abstract)
class USERINTERFACES_API UAlchemistHUDWidget : public UPotionPanicActivatableWidget
{
	GENERATED_BODY()

public:

	UAlchemistHUDWidget(const FObjectInitializer& ObjectInitializer);
};
