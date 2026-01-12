// Fill out your copyright notice in the Description page of Project Settings.


#include "SettingsManager.h"
#include "SettingsPreferences.h"
#include "PlayerPreferences.h"
#include "Kismet/GameplayStatics.h"

const FString USettingsManager::SettingsSaveSlotName = TEXT("GameSettings");
const FString USettingsManager::PlayerPreferencesSaveSlotName = TEXT("PlayerPreferences");

USettingsManager* USettingsManager::Get(const UObject* WorldContextObject)
{
	if (!WorldContextObject)
	{
		UE_LOG(LogTemp, Warning, TEXT("SettingsManager::Get - WorldContextObject is null"));
		return nullptr;
	}

	UGameInstance* GameInstance = WorldContextObject->GetWorld()
		? WorldContextObject->GetWorld()->GetGameInstance()
		: nullptr;

	if (!GameInstance)
	{
		UE_LOG(LogTemp, Warning, TEXT("SettingsManager::Get - Failed to get GameInstance"));
		return nullptr;
	}

	return GameInstance->GetSubsystem<USettingsManager>();
}

void USettingsManager::InitializeAndApplySettings()
{
	// Load or create settings
	LoadSettings();
	LoadPlayerPreferences();

	// Apply all settings to the game
	ApplyAll();

	UE_LOG(LogTemp, Log, TEXT("SettingsManager initialized and applied all settings"));
}

void USettingsManager::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	// Load settings on initialization
	LoadSettings();
	LoadPlayerPreferences();

	// Apply loaded settings
	if (Settings)
	{
		Settings->ApplyAllSettings();
	}

	if (PlayerPreferences)
	{
		PlayerPreferences->ApplyPreferences();
	}

	UE_LOG(LogTemp, Log, TEXT("SettingsManager initialized"));
}

void USettingsManager::Deinitialize()
{
	// Auto-save on shutdown
	SaveSettings();
	SavePlayerPreferences();

	Super::Deinitialize();
}

void USettingsManager::LoadSettings()
{
	// Try to load existing save
	if (USettingsPreferences* LoadedSettings = Cast<USettingsPreferences>(
		UGameplayStatics::LoadGameFromSlot(SettingsSaveSlotName, 0)))
	{
		Settings = LoadedSettings;
		UE_LOG(LogTemp, Log, TEXT("Loaded settings from save slot"));
	}
	else
	{
		// Create new settings with defaults
		Settings = NewObject<USettingsPreferences>(this);
		Settings->LoadFromGameUserSettings(); // Load from UE's GameUserSettings
		UE_LOG(LogTemp, Log, TEXT("Created new settings with defaults"));
	}
}

void USettingsManager::SaveSettings()
{
	if (!Settings)
	{
		UE_LOG(LogTemp, Warning, TEXT("Cannot save settings - Settings is null"));
		return;
	}

	// Save to GameUserSettings first
	Settings->SaveToGameUserSettings();

	// Then save our custom save game
	if (UGameplayStatics::SaveGameToSlot(Settings, SettingsSaveSlotName, 0))
	{
		UE_LOG(LogTemp, Log, TEXT("Settings saved successfully"));
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to save settings"));
	}
}

void USettingsManager::LoadPlayerPreferences()
{
	// Try to load existing save
	if (UPlayerPreferences* LoadedPreferences = Cast<UPlayerPreferences>(
		UGameplayStatics::LoadGameFromSlot(PlayerPreferencesSaveSlotName, 0)))
	{
		PlayerPreferences = LoadedPreferences;
		UE_LOG(LogTemp, Log, TEXT("Loaded player preferences from save slot"));
	}
	else
	{
		// Create new preferences with defaults
		PlayerPreferences = NewObject<UPlayerPreferences>(this);
		UE_LOG(LogTemp, Log, TEXT("Created new player preferences with defaults"));
	}
}

void USettingsManager::SavePlayerPreferences()
{
	if (!PlayerPreferences)
	{
		UE_LOG(LogTemp, Warning, TEXT("Cannot save player preferences - PlayerPreferences is null"));
		return;
	}

	if (UGameplayStatics::SaveGameToSlot(PlayerPreferences, PlayerPreferencesSaveSlotName, 0))
	{
		UE_LOG(LogTemp, Log, TEXT("Player preferences saved successfully"));
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to save player preferences"));
	}
}

void USettingsManager::ApplyAll()
{
	if (Settings)
	{
		Settings->ApplyAllSettings();
	}

	if (PlayerPreferences)
	{
		PlayerPreferences->ApplyPreferences();
	}

	UE_LOG(LogTemp, Log, TEXT("Applied all settings and preferences"));
}

void USettingsManager::ResetSettingsToDefaults()
{
	// Create new settings object with defaults
	Settings = NewObject<USettingsPreferences>(this);
	Settings->ApplyAllSettings();
	SaveSettings();

	UE_LOG(LogTemp, Log, TEXT("Reset settings to defaults"));
}

void USettingsManager::ResetPlayerPreferencesToDefaults()
{
	// Create new preferences object with defaults
	PlayerPreferences = NewObject<UPlayerPreferences>(this);
	PlayerPreferences->ResetToDefaults();
	PlayerPreferences->ApplyPreferences();
	SavePlayerPreferences();

	UE_LOG(LogTemp, Log, TEXT("Reset player preferences to defaults"));
}
