#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "VideoSettingsWidget.generated.h"

UCLASS()
class USERINTERFACES_API UVideoSettingsWidget : public UUserWidget
{
	GENERATED_BODY()

public:

	UFUNCTION(BlueprintCallable, Category = "Settings|Video")
	void SetScreenMode(int32 Index) { ScreenMode = Index; bDirty = true; }

	UFUNCTION(BlueprintCallable, Category = "Settings|Video")
	void SetQuality(int32 Index) { Quality = Index; bDirty = true; }

	UFUNCTION(BlueprintCallable, Category = "Settings|Video")
	void SetResolution(int32 Index) { Resolution = Index; bDirty = true; }

	UFUNCTION(BlueprintCallable, Category = "Settings|Video")
	void SetFPS(int32 Index) { FPS = Index; bDirty = true; }

	UFUNCTION(BlueprintCallable, Category = "Settings|Video")
	void SetVSync(int32 Index) { VSync = Index; bDirty = true; }

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Settings|Video")
	int32 GetScreenModeIndex() const { return ScreenMode; }

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Settings|Video")
	int32 GetQualityIndex() const { return Quality; }

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Settings|Video")
	int32 GetResolutionIndex() const { return Resolution; }

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Settings|Video")
	int32 GetFPSIndex() const { return FPS; }

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Settings|Video")
	int32 GetVSyncIndex() const { return VSync; }

	UFUNCTION(BlueprintCallable, Category = "Settings|Video")
	void LoadCurrentSettings();

	UFUNCTION(BlueprintCallable, Category = "Settings|Video")
	TArray<FIntPoint> GetAvailableResolutions();

	void ApplyIfDirty();

private:

	static constexpr int32 NUM_FPS_OPTIONS = 4;
	static const int32 FPSValues[NUM_FPS_OPTIONS];

	void Apply();
	void CacheResolutions();

	TArray<FIntPoint> CachedResolutions;

	int32 ScreenMode = 0;
	int32 Quality    = 0;
	int32 Resolution = 0;
	int32 FPS        = 1;
	int32 VSync      = 0;

	bool bDirty = false;
};
