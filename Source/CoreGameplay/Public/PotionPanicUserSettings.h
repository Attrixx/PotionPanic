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

	UFUNCTION(BlueprintCallable, Category = "Settings")
	static UPotionPanicUserSettings* GetPotionPanicUserSettings();

	UPROPERTY(Config)
	float MasterVolume = 1.0f;

	UPROPERTY(Config)
	float MusicVolume = 0.8f;

	UPROPERTY(Config)
	float SFXVolume = 1.0f;

	UPROPERTY(Config)
	TArray<FSavedKeybind> SavedKeybinds;
};
