// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "ActivityStepPresentation.h"
#include "ActivityInputSettings.generated.h"

class UInputAction;
class UInputMappingContext;

/**
 * Maps the four activity input directions onto real input actions.
 *
 * The Activities module deliberately knows nothing about Enhanced Input: a step asks for "Left" and
 * this is where that becomes a key. Kept out of the pawn so the mapping is one project-wide
 * decision rather than a per-Blueprint one.
 */
UCLASS(config = Game, defaultconfig, meta = (DisplayName = "Activity Input"))
class PLAYER_API UActivityInputSettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:

	FName GetCategoryName() const override;

	/**
	 * @param Slot The slot to resolve.
	 * @return The action configured for Slot, null when there is none. Loads it if needed.
	 */
	UInputAction* GetSlotAction(EActivityInputSlot Slot) const;

	/**
	 * Pushed over the usual mappings while a step owns the player's inputs, and popped when it
	 * gives them back. Mapping the movement keys to the slot actions in here is what makes the
	 * player stop walking: the higher priority context wins them.
	 */
	UPROPERTY(EditAnywhere, config, Category = "Input")
	TSoftObjectPtr<UInputMappingContext> ComboMappingContext;

	/**
	 * The action standing behind each direction. A slot with no entry simply cannot be pressed, and
	 * a step that draws it will always time out on it.
	 */
	UPROPERTY(EditAnywhere, config, Category = "Input")
	TMap<EActivityInputSlot, TSoftObjectPtr<UInputAction>> SlotActions;

	/** Priority the combo context is pushed at. Must beat the movement context, which sits at 0. */
	UPROPERTY(EditAnywhere, config, Category = "Input", meta = (ClampMin = 1))
	int32 ComboContextPriority = 10;

	/**
	 * Reserved: the action that would let a player give up on a step they cannot finish.
	 * Nothing binds it yet -- the pawn's plumbing is there, the key is not.
	 */
	UPROPERTY(EditAnywhere, config, Category = "Input")
	TSoftObjectPtr<UInputAction> CancelAction;
};
