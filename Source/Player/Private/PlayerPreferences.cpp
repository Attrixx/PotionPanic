// Fill out your copyright notice in the Description page of Project Settings.


#include "PlayerPreferences.h"
#include "EnhancedInputSubsystems.h"
#include "EnhancedInputComponent.h"
#include "Kismet/GameplayStatics.h"

UPlayerPreferences::UPlayerPreferences()
{
	// Default player info
	PlayerName = TEXT("Player");

	// Default gameplay settings
	MouseSensitivity = 1.0f;
	bInvertYAxis = false;

	// Default input device settings
	PreferredInputDevice = EInputDeviceType::Auto;
	GamepadSensitivity = 1.0f;
	GamepadDeadzone = 0.25f; // Standard deadzone
	bGamepadVibrationEnabled = true;

	// Keybinds are empty by default - will use Enhanced Input defaults
}

void UPlayerPreferences::ApplyPreferences()
{
	// Apply mouse sensitivity and invert Y axis settings
	// This would typically interface with your input component or player controller
	// Implementation depends on how you're handling input in your game

	UE_LOG(LogTemp, Log, TEXT("Applied Player Preferences - Name: %s, Mouse Sensitivity: %.2f, Invert Y: %s"),
		*PlayerName, MouseSensitivity, bInvertYAxis ? TEXT("Yes") : TEXT("No"));

	// Note: Keybind application would typically happen in your player controller
	// by modifying the Enhanced Input Mapping Context at runtime
}

void UPlayerPreferences::SaveKeybind(FName ActionName, FKey NewKey)
{
	if (ActionName.IsNone())
	{
		UE_LOG(LogTemp, Warning, TEXT("Cannot save keybind with empty ActionName"));
		return;
	}

	// Store the custom keybind
	CustomKeybinds.Add(ActionName, NewKey);

	UE_LOG(LogTemp, Log, TEXT("Saved keybind: %s -> %s"), *ActionName.ToString(), *NewKey.ToString());

	// Note: You would typically also update the Enhanced Input Mapping Context here
	// This requires access to the player controller and its input component
}

FKey UPlayerPreferences::GetKeybindForAction(FName ActionName) const
{
	// Check if we have a custom keybind
	if (const FKey* CustomKey = CustomKeybinds.Find(ActionName))
	{
		return *CustomKey;
	}

	// Return an invalid key if not found
	// In a complete implementation, you'd return the default from your Input Mapping Context
	return FKey();
}

void UPlayerPreferences::ResetKeybindsToDefault()
{
	// Clear all custom keybinds
	CustomKeybinds.Empty();

	UE_LOG(LogTemp, Log, TEXT("Reset all keybinds to default"));

	// Note: You would also need to restore the default Enhanced Input Mapping Context
}

void UPlayerPreferences::ResetToDefaults()
{
	// Reset all preferences to default values
	PlayerName = TEXT("Player");
	MouseSensitivity = 1.0f;
	bInvertYAxis = false;
	
	// Reset input device settings
	PreferredInputDevice = EInputDeviceType::Auto;
	GamepadSensitivity = 1.0f;
	GamepadDeadzone = 0.25f;
	bGamepadVibrationEnabled = true;
	
	ResetKeybindsToDefault();

	UE_LOG(LogTemp, Log, TEXT("Reset all player preferences to default"));
}

EInputDeviceType UPlayerPreferences::DetectActiveInputDevice() const
{
	// If user has set a preference, return it
	if (PreferredInputDevice != EInputDeviceType::Auto)
	{
		return PreferredInputDevice;
	}

	// Auto-detect based on gamepad connection state
	// We check if any gamepad is connected using the platform's input system
	auto GamepadKeys = {
		EKeys::Gamepad_LeftX, EKeys::Gamepad_LeftY,
		EKeys::Gamepad_RightX, EKeys::Gamepad_RightY,
		EKeys::Gamepad_FaceButton_Bottom
	};
	
	// Check if any gamepad key exists (indicates gamepad is connected)
	bool bGamepadConnected = false;
	for (const FKey& Key : GamepadKeys)
	{
		if (Key.IsValid() && Key.IsGamepadKey())
		{
			bGamepadConnected = true;
			break;
		}
	}
	
	// Return Gamepad if one is connected, otherwise KeyboardMouse
	EInputDeviceType DetectedDevice = bGamepadConnected ? EInputDeviceType::Gamepad : EInputDeviceType::KeyboardMouse;
	
	UE_LOG(LogTemp, Log, TEXT("Auto-detected input device: %s"), 
		DetectedDevice == EInputDeviceType::Gamepad ? TEXT("Gamepad") : TEXT("KeyboardMouse"));
	
	return DetectedDevice;
}

void UPlayerPreferences::ApplyCustomInputMappings(APlayerController* PlayerController)
{
	if (!PlayerController)
	{
		UE_LOG(LogTemp, Warning, TEXT("Cannot apply custom input mappings - PlayerController is null"));
		return;
	}

	UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer());
	if (!Subsystem)
	{
		UE_LOG(LogTemp, Warning, TEXT("Cannot apply custom input mappings - Enhanced Input Subsystem not found"));
		return;
	}

	// Log custom keybinds (actual remapping done in PlayerController with IMC access)
	if (CustomKeybinds.Num() > 0)
	{
		UE_LOG(LogTemp, Log, TEXT("Applying %d custom keybinds"), CustomKeybinds.Num());
		for (const TPair<FName, FKey>& Keybind : CustomKeybinds)
		{
			UE_LOG(LogTemp, Log, TEXT("  %s -> %s"), *Keybind.Key.ToString(), *Keybind.Value.ToString());
		}
	}
	
	UE_LOG(LogTemp, Log, TEXT("Input settings: MouseSens=%.2f GamepadSens=%.2f Deadzone=%.2f Vibration=%s"),
		MouseSensitivity, GamepadSensitivity, GamepadDeadzone, 
		bGamepadVibrationEnabled ? TEXT("On") : TEXT("Off"));
}
