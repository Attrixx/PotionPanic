#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameUserSettings.h"
#include "PotionPanicUserSettings.generated.h"

USTRUCT(BlueprintType)
struct FSavedKeybind
{
	GENERATED_BODY()

	UPROPERTY()
	int32 PlayerIndex = 0;

	UPROPERTY()
	FName InputActionName;

	UPROPERTY()
	int32 MappingIndex = 0;

	UPROPERTY()
	FKey KeyboardKey;

	UPROPERTY()
	FKey GamepadKey;
};

UCLASS()
class COREGAMEPLAY_API UPotionPanicUserSettings : public UGameUserSettings
{
	GENERATED_BODY()

public:

	static constexpr float DefaultMasterVolume = 1.0f;
	static constexpr float DefaultMusicVolume  = 0.8f;
	static constexpr float DefaultSFXVolume    = 1.0f;

	UFUNCTION(BlueprintCallable, Category = "Settings")
	static UPotionPanicUserSettings* GetPotionPanicUserSettings();

	UFUNCTION(BlueprintCallable, Category = "Settings")
	void EnsureVideoDefaults();

	virtual void SetToDefaults() override;

	UPROPERTY(Config)
	float MasterVolume = DefaultMasterVolume;

	UPROPERTY(Config)
	float MusicVolume = DefaultMusicVolume;

	UPROPERTY(Config)
	float SFXVolume = DefaultSFXVolume;

	UPROPERTY(Config)
	TArray<FSavedKeybind> SavedKeybinds;

	UPROPERTY(Config)
	bool bVideoDefaultsInitialized = false;

	UPROPERTY(Config)
	bool bAudioDefaultsInitialized = false;
};
