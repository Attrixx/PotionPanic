// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "PlayerPreferences.generated.h"

// Input device types for multi-platform support
UENUM(BlueprintType)
enum class EInputDeviceType : uint8
{
	KeyboardMouse UMETA(DisplayName = "Keyboard & Mouse"),
	Gamepad UMETA(DisplayName = "Gamepad"),
	Auto UMETA(DisplayName = "Auto-Detect")
};

/**
 * Player-specific preferences and keybinds
 * Uses ULocalPlayerSaveGame for automatic per-player persistence
 */
UCLASS()
class PLAYER_API UPlayerPreferences : public ULocalPlayerSaveGame
{
	GENERATED_BODY()

public:
	UPlayerPreferences();

	// ==================== Player Info ====================

	UPROPERTY(BlueprintReadWrite, Category = "Player")
	FString PlayerName;

	// ==================== Gameplay Settings ====================

	/** Mouse sensitivity multiplier (0.1 to 5.0) */
	UPROPERTY(BlueprintReadWrite, Category = "Gameplay")
	float MouseSensitivity;

	/** Whether to invert Y axis for mouse/gamepad */
	UPROPERTY(BlueprintReadWrite, Category = "Gameplay")
	bool bInvertYAxis;

	// ==================== Input Device Settings ====================

	/** Preferred input device (can be set to Auto for automatic detection) */
	UPROPERTY(BlueprintReadWrite, Category = "Input")
	EInputDeviceType PreferredInputDevice;

	/** Sensitivity for gamepad look (separate from mouse) */
	UPROPERTY(BlueprintReadWrite, Category = "Input|Gamepad")
	float GamepadSensitivity;

	/** Deadzone for analog sticks (0.0 to 1.0) */
	UPROPERTY(BlueprintReadWrite, Category = "Input|Gamepad")
	float GamepadDeadzone;

	/** Enable/disable gamepad vibrations */
	UPROPERTY(BlueprintReadWrite, Category = "Input|Gamepad")
	bool bGamepadVibrationEnabled;

	// ==================== Keybinds ====================
	
	/** 
	 * Custom keybinds stored as ActionName -> KeyName
	 * This will store player customizations for Enhanced Input actions
	 */
	UPROPERTY(BlueprintReadWrite, Category = "Keybinds")
	TMap<FName, FKey> CustomKeybinds;

	// ==================== Methods ====================

	/** Apply all player preferences to the game */
	UFUNCTION(BlueprintCallable, Category = "Player Preferences")
	void ApplyPreferences();

	/** Save a custom keybind */
	UFUNCTION(BlueprintCallable, Category = "Player Preferences")
	void SaveKeybind(FName ActionName, FKey NewKey);

	/** Get the current key for an action (custom or default) */
	UFUNCTION(BlueprintCallable, Category = "Player Preferences")
	FKey GetKeybindForAction(FName ActionName) const;

	/** Reset all keybinds to default */
	UFUNCTION(BlueprintCallable, Category = "Player Preferences")
	void ResetKeybindsToDefault();

	/** Reset all preferences to default values */
	UFUNCTION(BlueprintCallable, Category = "Player Preferences")
	void ResetToDefaults();

	/** Detect which input device is currently being used */
	UFUNCTION(BlueprintCallable, Category = "Player Preferences")
	EInputDeviceType DetectActiveInputDevice() const;

	/** Apply custom input mappings to a player controller */
	UFUNCTION(BlueprintCallable, Category = "Player Preferences")
	void ApplyCustomInputMappings(APlayerController* PlayerController);
};
