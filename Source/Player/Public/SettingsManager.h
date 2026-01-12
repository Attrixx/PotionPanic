// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "SettingsManager.generated.h"

class USettingsPreferences;
class UPlayerPreferences;

/**
 * Subsystem to manage game settings and player preferences
 * Handles loading, saving, and applying settings
 */
UCLASS()
class PLAYER_API USettingsManager : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	// GameInstanceSubsystem interface
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	// ==================== Static Helper ====================

	/** Get the SettingsManager instance from a world context object */
	UFUNCTION(BlueprintPure, Category = "Settings", meta = (WorldContext = "WorldContextObject"))
	static USettingsManager* Get(const UObject* WorldContextObject);

	/** Initialize and apply all settings - call this at game startup */
	UFUNCTION(BlueprintCallable, Category = "Settings")
	void InitializeAndApplySettings();

	// ==================== Settings Management ====================

	/** Get the current settings preferences */
	UFUNCTION(BlueprintCallable, Category = "Settings")
	USettingsPreferences* GetSettings() const { return Settings; }

	/** Get the current player preferences */
	UFUNCTION(BlueprintCallable, Category = "Settings")
	UPlayerPreferences* GetPlayerPreferences() const { return PlayerPreferences; }

	/** Load settings from save file, or create new if none exists */
	UFUNCTION(BlueprintCallable, Category = "Settings")
	void LoadSettings();

	/** Save current settings to file */
	UFUNCTION(BlueprintCallable, Category = "Settings")
	void SaveSettings();

	/** Load player preferences from save file, or create new if none exists */
	UFUNCTION(BlueprintCallable, Category = "Settings")
	void LoadPlayerPreferences();

	/** Save current player preferences to file */
	UFUNCTION(BlueprintCallable, Category = "Settings")
	void SavePlayerPreferences();

	/** Apply all current settings and preferences */
	UFUNCTION(BlueprintCallable, Category = "Settings")
	void ApplyAll();

	/** Reset settings to defaults */
	UFUNCTION(BlueprintCallable, Category = "Settings")
	void ResetSettingsToDefaults();

	/** Reset player preferences to defaults */
	UFUNCTION(BlueprintCallable, Category = "Settings")
	void ResetPlayerPreferencesToDefaults();

private:
	UPROPERTY()
	TObjectPtr<USettingsPreferences> Settings;

	UPROPERTY()
	TObjectPtr<UPlayerPreferences> PlayerPreferences;

	static const FString SettingsSaveSlotName;
	static const FString PlayerPreferencesSaveSlotName;
};
