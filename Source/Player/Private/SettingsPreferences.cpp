// Fill out your copyright notice in the Description page of Project Settings.


#include "SettingsPreferences.h"
#include "GameFramework/GameUserSettings.h"
#include "AudioDevice.h"
#include "Kismet/GameplayStatics.h"

USettingsPreferences::USettingsPreferences()
{
	// Audio defaults (0.0 to 1.0)
	MasterVolume = 1.0f;
	MusicVolume = 0.8f;
	SFXVolume = 1.0f;
	UIVolume = 0.9f;

	// Video defaults
	WindowMode = EWindowModeType::WindowedFullscreen;
	Resolution = FIntPoint(1920, 1080);
	bVSyncEnabled = true;
	FrameRateLimit = 60;
	ShadowQuality = EGraphicsQuality::High;
	TextureQuality = EGraphicsQuality::High;
	AntiAliasingQuality = EGraphicsQuality::High;
	PostProcessingQuality = EGraphicsQuality::High;
}

void USettingsPreferences::ApplyAudioSettings()
{
	if (UWorld* World = GetWorld())
	{
		if (FAudioDevice* AudioDevice = World->GetAudioDeviceRaw())
		{
			// Audio volumes are stored and ready to use
			// Connect to Sound Classes in editor for full control
			UE_LOG(LogTemp, Log, TEXT("Audio Settings: Master=%.2f Music=%.2f SFX=%.2f UI=%.2f"),
				MasterVolume, MusicVolume, SFXVolume, UIVolume);
		}
	}
}

void USettingsPreferences::ApplyVideoSettings()
{
	UGameUserSettings* GameSettings = UGameUserSettings::GetGameUserSettings();
	if (!GameSettings)
	{
		UE_LOG(LogTemp, Warning, TEXT("Failed to get GameUserSettings"));
		return;
	}

	// Window Mode
	EWindowMode::Type UnrealWindowMode;
	switch (WindowMode)
	{
		case EWindowModeType::Fullscreen:
			UnrealWindowMode = EWindowMode::Fullscreen;
			break;
		case EWindowModeType::WindowedFullscreen:
			UnrealWindowMode = EWindowMode::WindowedFullscreen;
			break;
		case EWindowModeType::Windowed:
			UnrealWindowMode = EWindowMode::Windowed;
			break;
		default:
			UnrealWindowMode = EWindowMode::WindowedFullscreen;
	}
	GameSettings->SetFullscreenMode(UnrealWindowMode);

	// Resolution
	GameSettings->SetScreenResolution(Resolution);

	// VSync
	GameSettings->SetVSyncEnabled(bVSyncEnabled);

	// Frame Rate Limit
	GameSettings->SetFrameRateLimit(static_cast<float>(FrameRateLimit));

	// Graphics Quality (0 = Low, 1 = Medium, 2 = High, 3 = Epic)
	auto QualityToInt = [](EGraphicsQuality Quality) -> int32
	{
		return static_cast<int32>(Quality);
	};

	GameSettings->SetShadowQuality(QualityToInt(ShadowQuality));
	GameSettings->SetTextureQuality(QualityToInt(TextureQuality));
	GameSettings->SetAntiAliasingQuality(QualityToInt(AntiAliasingQuality));
	GameSettings->SetPostProcessingQuality(QualityToInt(PostProcessingQuality));

	// Apply and save
	GameSettings->ApplySettings(false);

	UE_LOG(LogTemp, Log, TEXT("Applied Video Settings - Resolution: %dx%d, VSync: %s"),
		Resolution.X, Resolution.Y, bVSyncEnabled ? TEXT("On") : TEXT("Off"));
}

void USettingsPreferences::LoadFromGameUserSettings()
{
	UGameUserSettings* GameSettings = UGameUserSettings::GetGameUserSettings();
	if (!GameSettings)
	{
		return;
	}

	// Load video settings from GameUserSettings
	Resolution = GameSettings->GetScreenResolution();
	bVSyncEnabled = GameSettings->IsVSyncEnabled();
	FrameRateLimit = static_cast<int32>(GameSettings->GetFrameRateLimit());

	// Window mode
	EWindowMode::Type CurrentWindowMode = GameSettings->GetFullscreenMode();
	switch (CurrentWindowMode)
	{
		case EWindowMode::Fullscreen:
			WindowMode = EWindowModeType::Fullscreen;
			break;
		case EWindowMode::WindowedFullscreen:
			WindowMode = EWindowModeType::WindowedFullscreen;
			break;
		case EWindowMode::Windowed:
			WindowMode = EWindowModeType::Windowed;
			break;
	}

	// Quality settings
	auto IntToQuality = [](int32 Value) -> EGraphicsQuality
	{
		return static_cast<EGraphicsQuality>(FMath::Clamp(Value, 0, 3));
	};

	ShadowQuality = IntToQuality(GameSettings->GetShadowQuality());
	TextureQuality = IntToQuality(GameSettings->GetTextureQuality());
	AntiAliasingQuality = IntToQuality(GameSettings->GetAntiAliasingQuality());
	PostProcessingQuality = IntToQuality(GameSettings->GetPostProcessingQuality());
}

void USettingsPreferences::SaveToGameUserSettings()
{
	// Video settings are saved via ApplyVideoSettings
	// Audio settings are saved with this SaveGame object
	ApplyVideoSettings();
}

void USettingsPreferences::ApplyAllSettings()
{
	ApplyAudioSettings();
	ApplyVideoSettings();
}
