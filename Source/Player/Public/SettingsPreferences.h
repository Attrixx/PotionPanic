// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "SettingsPreferences.generated.h"


// Window mode options
UENUM(BlueprintType)
enum class EWindowModeType : uint8
{
	Fullscreen UMETA(DisplayName = "Fullscreen"),
	WindowedFullscreen UMETA(DisplayName = "Borderless Window"),
	Windowed UMETA(DisplayName = "Windowed")
};

// Graphics quality levels
UENUM(BlueprintType)
enum class EGraphicsQuality : uint8
{
	Low UMETA(DisplayName = "Low"),
	Medium UMETA(DisplayName = "Medium"),
	High UMETA(DisplayName = "High"),
	Epic UMETA(DisplayName = "Epic")
};

/**
 * Settings for audio and video configuration
 * Saved per-platform using USaveGame serialization
 */
UCLASS()
class PLAYER_API USettingsPreferences : public USaveGame
{
	GENERATED_BODY()

public:
	USettingsPreferences();

	// ==================== Audio Settings ====================
	
	UPROPERTY(BlueprintReadWrite, Category = "Audio")
	float MasterVolume;

	UPROPERTY(BlueprintReadWrite, Category = "Audio")
	float MusicVolume;

	UPROPERTY(BlueprintReadWrite, Category = "Audio")
	float SFXVolume;

	UPROPERTY(BlueprintReadWrite, Category = "Audio")
	float UIVolume;

	// ==================== Video Settings ====================
	
	UPROPERTY(BlueprintReadWrite, Category = "Video")
	EWindowModeType WindowMode;

	UPROPERTY(BlueprintReadWrite, Category = "Video")
	FIntPoint Resolution;

	UPROPERTY(BlueprintReadWrite, Category = "Video")
	bool bVSyncEnabled;

	UPROPERTY(BlueprintReadWrite, Category = "Video")
	int32 FrameRateLimit;

	UPROPERTY(BlueprintReadWrite, Category = "Video")
	EGraphicsQuality ShadowQuality;

	UPROPERTY(BlueprintReadWrite, Category = "Video")
	EGraphicsQuality TextureQuality;

	UPROPERTY(BlueprintReadWrite, Category = "Video")
	EGraphicsQuality AntiAliasingQuality;

	UPROPERTY(BlueprintReadWrite, Category = "Video")
	EGraphicsQuality PostProcessingQuality;

	// ==================== Methods ====================

	/** Apply audio settings to the game */
	UFUNCTION(BlueprintCallable, Category = "Settings")
	void ApplyAudioSettings();

	/** Apply video settings to the game */
	UFUNCTION(BlueprintCallable, Category = "Settings")
	void ApplyVideoSettings();

	/** Load settings from GameUserSettings */
	UFUNCTION(BlueprintCallable, Category = "Settings")
	void LoadFromGameUserSettings();

	/** Save settings to GameUserSettings */
	UFUNCTION(BlueprintCallable, Category = "Settings")
	void SaveToGameUserSettings();

	/** Apply all settings */
	UFUNCTION(BlueprintCallable, Category = "Settings")
	void ApplyAllSettings();
};
