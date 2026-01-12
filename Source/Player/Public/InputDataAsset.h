// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "InputDataAsset.generated.h"

class UInputAction;
class UInputMappingContext;

/**
 * Data Asset to hold references to Input Actions and Mapping Context
 * This centralizes all input configuration for easy access
 */
UCLASS(BlueprintType)
class PLAYER_API UInputDataAsset : public UDataAsset
{
	GENERATED_BODY()

public:
	// ==================== Input Mapping Context ====================

	/** Default Input Mapping Context with AZERTY + Gamepad bindings */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputMappingContext> DefaultMappingContext;

	// ==================== Input Actions ====================

	/** Move action (Axis2D: ZQSD / Gamepad stick) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input|Actions")
	TObjectPtr<UInputAction> MoveAction;

	/** Interact action (Digital: E / L2+Square / LT+X) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input|Actions")
	TObjectPtr<UInputAction> InteractAction;

	/** Pickup/Drop action (Digital: A / R2+Cross / RT+A) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input|Actions")
	TObjectPtr<UInputAction> PickupDropAction;

	/** Throw action (Digital: Space / Triangle / Y) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input|Actions")
	TObjectPtr<UInputAction> ThrowAction;

	/** Dash action (Digital: LShift / Circle / B) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input|Actions")
	TObjectPtr<UInputAction> DashAction;

	/** Pause action (Digital: Escape / Options / Menu) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input|Actions")
	TObjectPtr<UInputAction> PauseAction;
};
