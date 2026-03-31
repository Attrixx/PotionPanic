#include "VideoSettingsWidget.h"
#include "GameFramework/GameUserSettings.h"

const FIntPoint UVideoSettingsWidget::Resolutions[NUM_RESOLUTIONS] = {
	{1280, 720}, {1920, 1080}, {2560, 1440}, {3840, 2160}
};

const int32 UVideoSettingsWidget::FPSValues[NUM_FPS_OPTIONS] = { 30, 60, 120, 0 };

void UVideoSettingsWidget::LoadCurrentSettings()
{
	UGameUserSettings* Settings = UGameUserSettings::GetGameUserSettings();
	if (!Settings) return;

	switch (Settings->GetFullscreenMode())
	{
	case EWindowMode::Fullscreen:        ScreenMode = 0; break;
	case EWindowMode::WindowedFullscreen: ScreenMode = 1; break;
	case EWindowMode::Windowed:          ScreenMode = 2; break;
	default:                             ScreenMode = 0; break;
	}

	Quality = FMath::Max(Settings->GetOverallScalabilityLevel(), 0);

	const FIntPoint CurrentRes = Settings->GetScreenResolution();
	Resolution = 0;
	for (int32 i = 0; i < NUM_RESOLUTIONS; ++i)
	{
		if (Resolutions[i] == CurrentRes)
		{
			Resolution = i;
			break;
		}
	}

	const float FPSLimit = Settings->GetFrameRateLimit();
	FPS = 3;
	for (int32 i = 0; i < NUM_FPS_OPTIONS; ++i)
	{
		if (FMath::IsNearlyEqual(FPSLimit, static_cast<float>(FPSValues[i]), 1.0f))
		{
			FPS = i;
			break;
		}
	}

	VSync = Settings->IsVSyncEnabled() ? 0 : 1;
	bDirty = false;
}

void UVideoSettingsWidget::ApplyIfDirty()
{
	if (!bDirty) return;
	Apply();
	bDirty = false;
}

void UVideoSettingsWidget::Apply()
{
	UGameUserSettings* Settings = UGameUserSettings::GetGameUserSettings();
	if (!Settings) return;

	static const EWindowMode::Type ModeMap[] = {
		EWindowMode::Fullscreen,
		EWindowMode::WindowedFullscreen,
		EWindowMode::Windowed
	};

	if (ScreenMode >= 0 && ScreenMode < 3)
	{
		Settings->SetFullscreenMode(ModeMap[ScreenMode]);
	}

	Settings->SetOverallScalabilityLevel(FMath::Clamp(Quality, 0, 2));

	if (Resolution >= 0 && Resolution < NUM_RESOLUTIONS)
	{
		Settings->SetScreenResolution(Resolutions[Resolution]);
	}

	if (FPS >= 0 && FPS < NUM_FPS_OPTIONS)
	{
		Settings->SetFrameRateLimit(static_cast<float>(FPSValues[FPS]));
	}

	Settings->SetVSyncEnabled(VSync == 0);
	Settings->ApplySettings(false);
	Settings->SaveSettings();
}